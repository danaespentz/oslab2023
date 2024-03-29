/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/
#define perror_pthread(ret,msg)\
	do{errno=ret;perror(msg);}while(0)
/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void sigint_handler(int signum){
	reset_xterm_color(1);
	exit(1);
}

sem_t *sem;
int nrthreads;

void* compute_and_output_mandel_line(void *thread_index)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */
	int i, color_val[x_chars];

	for(i=(int)thread_index;i<y_chars;i+=nrthreads){
		compute_mandel_line(i,color_val);
		if(sem_wait(&sem[(int)thread_index])<0){
			perror("sem_wait");
			exit(1);
		}
		output_mandel_line(1,color_val);
		if(sem_post(&sem[((int)thread_index+1)%nrthreads])<0){
			perror("sem_post");
			exit(1);
		}
	}
	return NULL;
}

int safe_atoi(char *s, int *val){
	long l;
	char *endp;

	l = strtol(s, &endp, 10);
	if (s != endp && *endp == '\0') {
		*val = l;
		return 0;
	} else
		return -1;
}		

void* safe_malloc(size_t size){
	void * ptr = malloc(size);
	if(ptr==NULL){
		
		exit(EXIT_FAILURE);
	}
	return ptr;
}


int main(int argc,char **argv){
	int i,ret;
	int line;

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	/*
	 * draw the Mandelbrot Set, one line at a time.
	 * Output is sent to file descriptor '1', i.e., standard output.
	 */

	if(argc!=2)
		exit(1);

	nrthreads=atoi(argv[1]);

	/* sets up the signal handler for the SIGINT signal (Ctrl+C)*/
	struct sigaction sa;
	sa.sa_handler=sigint_handler;
	sa.sa_flags= 0;
	sigemptyset(&sa.sa_mask);
	
	if(sigaction(SIGINT, &sa,NULL)<0){
		perror("sigaction");
		exit(1);
	}

	/* the sem variable is a pointer to a semaphore array with the size of nrthreads
	 * Semaphores are synchronization primitives used to control the synchronization between threads 
	 * when outputting lines of the Mandelbrot set to the terminal !
	 * sem_wait(): locks a seaphore
	 * sem_post(): releases the lock */
	
	sem=(sem_t*)malloc(nrthreads*sizeof(sem_t));

	/* the first semaphore allows the first thread to start immediately, 
	 * and the rest of the semaphores ensure that subsequent threads wait 
	 * until signaled by another thread before proceeding */

	if(sem_init(&sem[0],0,1)<0){
		perror("sem_init");
		exit(1);
	}

	for(i=1; i<nrthreads;i++){
		if(sem_init(&sem[i],0,0)<0){
			perror("sem_init");
			exit(1);
		}
	}

	pthread_t thread[nrthreads];
	for(i=0;i<nrthreads;i++){
		ret=pthread_create(&thread[i],NULL,compute_and_output_mandel_line,(void*)i);
		if(ret){
			perror_pthread(ret, "pthread_create");
			exit(1);
		}
	}
	
	/*Synchronization: By calling pthread_join, the main thread waits for each child thread to complete before proceeding. 
	 * This ensures that all the threads finish their work before the program exits. */
	for(i=0;i<nrthreads;i++){
		ret=pthread_join(thread[i],NULL);
		if(ret)
			perror_pthread(ret, "pthread_join");
	}

	/*The purpose of this loop is to clean up and release the resources associated with the semaphores. */
	for(i=0;i<nrthreads;i++){
		if(sem_destroy(&sem[i])<0){
				perror("sem_destroy");
				exit(1);
		}
	}

	free(sem);
			

//	for (line = 0; line < y_chars; line++) {
//		compute_and_output_mandel_line(1, line);
//	}

	reset_xterm_color(1);
	return 0;
}
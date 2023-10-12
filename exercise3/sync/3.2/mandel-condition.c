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
 *  * Compile-time parameters *
 *   ***************************/
#define perror_pthread(ret,msg) \
	    do { errno = ret; perror(msg); } while (0)

/*
 *  * Output at the terminal is x_chars wide by y_chars long
 *   */
int y_chars = 50;
int x_chars = 90;

/*
 *  * The part of the complex plane to be drawn:
 *   * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
 *    */
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 *  * Every character in the final output is
 *   * xstep x ystep units wide on the complex plane.
 *    */
double xstep;
double ystep;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
/*
 *  * This function computes a line of output
 *   * as an array of x_char color values.
 *    */
void *safe_malloc(size_t size){
	void *p;
	if ((p = malloc(size)) == NULL) {
		fprintf(stderr, "Out of memory, failed to allocate %zd bytes\n", size);
		exit(1);
	}
	return p;
}

void compute_mandel_line(int line, int color_val[]){
	    /*
	     *      * x and y traverse the complex plane.
	     *           */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x += xstep, n++) {

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
 *  * This function outputs an array of x_char color values
 *   * to a 256-color xterm.
 *    */
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

void output_mandel_line(int fd, int color_val[]){
	int i;

	char point = '@';
	char newline = '\n';
	
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

void sigint_handler(int signum) {
	    reset_xterm_color(1);
	        exit(1);
}

pthread_mutex_t mutex;
pthread_cond_t cond;
int value = 0;
int nrthreads;
int current_line = 0;
int *current_thread;
struct custom_struct{
	  int fd;
	  int thrid;
       	  int thrcnt;
          pthread_t tid;
	  int len;
	  double *arr;
};

void *compute_and_output_mandel_line(void *arg){
	int color_val[x_chars];
	struct custom_struct *struct_1=arg;
	int thrid=struct_1->thrid;
	int fd=struct_1->fd;
	int thrcnt = struct_1->thrcnt;
	
	int line_x;
	for(line_x=thrid;line_x<y_chars;line_x+=thrcnt){
		pthread_mutex_lock(&mutex);
		while(*current_thread!=thrid)
			pthread_cond_wait(&cond,&mutex);
		compute_mandel_line(line_x,color_val);
		output_mandel_line(fd,color_val);


		*current_thread=(*current_thread+1)%thrcnt;

		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

int main(int argc, char **argv){
	int i, ret;
	double *arr;

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	if (argc != 2)
		exit(1);

	nrthreads = atoi(argv[1]);
	current_thread = &value;

	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) < 0) {
		perror("sigaction");
		exit(1);
	}

	pthread_mutex_init(&mutex, NULL);
     	pthread_cond_init(&cond, NULL);
	
	arr = safe_malloc(x_chars * sizeof(*arr));
	for (i = 0; i < x_chars; i++)
		arr[i] = 0;
	  
	struct custom_struct *thread_index;
	pthread_t thread[nrthreads];
	thread_index = safe_malloc(nrthreads * sizeof(*thread_index));

	for (i = 0; i < nrthreads; i++) {
		thread_index[i].arr = arr;
		thread_index[i].len = 90;
		thread_index[i].thrid = i;
		thread_index[i].thrcnt = nrthreads;
		
		ret = pthread_create(&thread[i], NULL, compute_and_output_mandel_line, &thread_index[i]);
		if (ret) {
			perror_pthread(ret, "pthread_create");
			exit(1);
		}
	}
	for (i = 0; i < nrthreads; i++) {
		ret = pthread_join(thread[i], NULL);
		if (ret)
			perror_pthread(ret, "pthread_join");
	}

	reset_xterm_color(1);
	return 0;
}

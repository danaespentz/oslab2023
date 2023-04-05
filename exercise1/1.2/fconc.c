#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {

        int fd1, fd2, fd3, oflags, mode;
        char *buff[20];
        memset(buff, 0, sizeof(buff));

        oflags = O_CREAT | O_TRUNC | O_RDWR;
        mode = S_IRUSR | S_IWUSR;

        fd1 = open("A", O_RDONLY);
        fd2 = open("B", O_RDONLY);

        if(fd1==-1){
                printf("A: No such file or directory\n");
                exit(1);
        }
        if(fd2==-1){
                printf("B: No such file or directory\n");
                exit(1);
        }

        if(argv[3])
                fd3 = open("C",oflags,mode);
        else
                fd3 = open("fconc.out",oflags,mode);

        int rdcnt=0, cnt=0;
        while (1){
                rdcnt=read(fd1,buff,sizeof(buff)-1);
                if(rdcnt==-1 || rdcnt==0)
                        break;
                while(1){
                        cnt=write(fd3, buff, rdcnt);
                        if (cnt==-1 || cnt==0 || cnt==rdcnt)
                                break;
                }
        }
        rdcnt=0; cnt=0;
        while (1){
                rdcnt=read(fd2,buff,sizeof(buff)-1);
                if(rdcnt==-1 || rdcnt==0)
                        break;
                while(1){
                        cnt=write(fd3, buff, rdcnt);
                        if(cnt==-1 || cnt==0 || cnt==rdcnt)
                                break;
                }
        }

        close(fd1); close(fd2); close(fd3);
        return 0;
}
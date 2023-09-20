{\rtf1\ansi\ansicpg1252\cocoartf1561\cocoasubrtf610
{\fonttbl\f0\fnil\fcharset0 Menlo-Regular;}
{\colortbl;\red255\green255\blue255;\red0\green0\blue0;\red255\green255\blue255;}
{\*\expandedcolortbl;;\csgray\c0;\csgray\c100000;}
\paperw11900\paperh16840\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\pard\tx560\tx1120\tx1680\tx2240\tx2800\tx3360\tx3920\tx4480\tx5040\tx5600\tx6160\tx6720\pardirnatural\partightenfactor0

\f0\fs22 \cf2 \cb3 \CocoaLigature0 #include <sys/stat.h>                                                                                                                               \
#include <sys/types.h>\
#include <fcntl.h> \
#include <stdio.h> \
#include <stdlib.h>\
#include <string.h>\
#include <unistd.h>\
\
int main(int argc, char **argv) \{\
	\
	int fd1, fd2, fd3, oflags, mode;\
	char *buff[20];\
	memset(buff, 0, sizeof(buff));\
\
	oflags = O_CREAT | O_TRUNC | O_RDWR;\
	mode = S_IRUSR | S_IWUSR;\
	\
	fd1 = open("A", O_RDONLY);\
	fd2 = open("B", O_RDONLY);\
	\
	if(fd1==-1)\{\
		printf("A: No such file or directory\\n");\
		exit(1);\
	\}\
	if(fd2==-1)\{\
		printf("B: No such file or directory\\n");                    \
		exit(1);\
	\}\
	\
	if(argv[3])\
		fd3 = open("C",oflags,mode);\
	else\
		fd3 = open("fconc.out",oflags,mode);\
	\
	int rdcnt=0, cnt=0;                                                       \
	while (1)\{\
		rdcnt=read(fd1,buff,sizeof(buff)-1);\
		if(rdcnt==-1 || rdcnt==0)\
			break;\
		while(1)\{\
			cnt=write(fd3, buff, rdcnt);\
			if (cnt==-1 || cnt==0 || cnt==rdcnt)\
				break;\
		\}	\
	\}\
	rdcnt=0; cnt=0;	\
	while (1)\{\
		rdcnt=read(fd2,buff,sizeof(buff)-1);\
		if(rdcnt==-1 || rdcnt==0)\
	         	break;\
		while(1)\{\
			cnt=write(fd3, buff, rdcnt);\
			if(cnt==-1 || cnt==0 || cnt==rdcnt)\
				break;\
		\}\
	\}\
		\
	close(fd1); close(fd2); close(fd3);\
	return 0;\
\} \
}
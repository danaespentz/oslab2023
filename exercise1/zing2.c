#include <unistd.h>
#include <stdio.h>

void zing(void){
        printf("Hello, %s \n", getlogin());
}
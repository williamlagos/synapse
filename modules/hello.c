#include<stdio.h>
#include<stdlib.h>

void start(int max, char** buffer)
{
    int count;
    for (count = 0; count < max; count++) {
        printf("%s", buffer[count]);
    }
    fprintf(stdout,"Hello World!\n");
}

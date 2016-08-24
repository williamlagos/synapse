#include<stdio.h>
#include<stdlib.h>

void start(int max, char** buffer)
{
    int count;
    for (count = 0; count < max; count++) {
        printf("%s", buffer[count]);
    }
    printf("Hello World!\n");
    // system("event");
    // system("kodi");
}

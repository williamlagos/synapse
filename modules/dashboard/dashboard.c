#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void start(int max, char** buffer)
{
    int count;
    for (count = 0; count < max; count++) {
        printf("%s", buffer[count]);
    }
    printf("Hello World!\n");
    execv("/usr/bin/kodi",NULL);
    execv("modules/dashboard/event",NULL);
}

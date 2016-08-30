#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void start(int max, char** buffer)
{
    /*int count;
    for (count = 0; count < max; count++) {
        printf("%s", buffer[count]);
    }
    printf("Hello World!\n");*/
    pid_t event_id = 0;
    event_id = fork();
    if (event_id == 0) {
        execv("/usr/lib/kodi/kodi.bin",NULL);
    } else {
        printf("Started dashboard %d",event_id);
        execv("modules/dashboard/event",NULL);
    }
}

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define DEFAULT_COMMAND_PATH "cat worked.file"

int start(int max, char** buffer)
{
    char cmd[128];
    char path[1024];
    if (max < 2) {
        strcpy(cmd, DEFAULT_COMMAND_PATH);
    } else {
        fprintf(stdout, buffer[1]);
        for (int i = 1; i < max; i++) {
            strcat(cmd, buffer[i]);
        }
    }

    int status = EXIT_SUCCESS;

    /* Open the command for reading. */
    FILE *f = popen(DEFAULT_COMMAND_PATH, "r");
    if(f == NULL){
        printf("Failed to run command\n" );
        exit(EXIT_FAILURE);
    }

    /* Read the output a line at a time - output it. */
    fgets(path, sizeof(path)-1, f);
    if(strstr(path,"True") || strstr(path,"False")){
        status = EXIT_SUCCESS;
    } else {
        status = EXIT_FAILURE;
    }

    pclose(f);
    return status;
}

int main(int argc, char** argv)
{
    return start(argc, argv);
}
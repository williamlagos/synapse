#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define DEFAULT_COMMAND_PATH "cat worked.file"

int start(int max, char** buffer)
{
    char cmd[256];
    if (max < 2) {
        strcpy(cmd, DEFAULT_COMMAND_PATH);
    } else {
        for (int i = 1; i < max; i++) {
            strcat(cmd, buffer[i]);
            strcat(cmd, " ");
        }
    }

    /* Open the command for reading. */
    // fprintf(stdout, "command: %s\n", cmd);
    FILE *f = popen(cmd, "r");
    if(f == NULL){
        fprintf(stderr, "error: failed to run command\n");
        return EXIT_FAILURE;
    }

    /* Read the output a line at a time - output it. */
    // char path[1024];
    // for (int i = 0; fgets(path, sizeof(path) - 1, f); i++) {
    //     fprintf(stdout, "%d %s", i, path);
    //     if (strstr(path,"foo")) return EXIT_SUCCESS;
    //     else return EXIT_FAILURE;
    // }
    // fputs("\n", stdout);

    int status = WEXITSTATUS(pclose(f));
    if (status == -1) return EXIT_FAILURE;
    else return status;
}

int main(int argc, char** argv)
{
    return start(argc, argv);
}
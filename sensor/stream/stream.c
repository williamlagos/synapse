#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HDMISTATUS_COMMAND_PATH "/opt/vc/bin/tvservice -s"

int start(int max, char** buffer)
{
    int status = EXIT_SUCCESS;
    char path[1035];

    /* Open the command for reading. */
    FILE *f = popen(HDMISTATUS_COMMAND_PATH, "r");
    if(f == NULL){
        printf("Failed to run command\n" );
        exit(EXIT_FAILURE);
    }

    /* Read the output a line at a time - output it. */
    fgets(path, sizeof(path)-1, f);
    if(strstr(path,"NTSC") || strstr(path,"PAL")){
        /*system("systemctl start minidlna");
        system("systemctl start gmediarender");
        system("systemctl start shairport-sync");*/
        status = EXIT_SUCCESS;
    } else {
        status = EXIT_FAILURE;
    }

    // fprintf(stdout,"%s", path);
    pclose(f);
    return status;
}

int main(int argc, char** argv)
{
    return start(argc,argv);
}

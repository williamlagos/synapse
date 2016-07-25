#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>

#define FALSE 0
#define TRUE 1

int main (int argc, char** argv)
{
  FILE* f = NULL;
  pid_t sid = 0;
  pid_t process_id = 0;
  char info[30] = "Logging some information...\n";

  process_id = fork();
  if (process_id < 0) {
    printf("Process forking failed!\n");
    exit(EXIT_FAILURE);
  }

  // Parent process
  if (process_id > 0) {
    printf("Process ID of child process: %d\n", process_id);
    exit(EXIT_SUCCESS);
  }

  umask(0);
  sid = setsid();
  if (sid < 0) exit(EXIT_FAILURE);

  chdir("/");
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  f = fopen("log.txt","w+");

  while(TRUE) {
    sleep(1);
    fprintf(f,info);
    fflush(f);
  }

  fclose(f);
  return 0;
}

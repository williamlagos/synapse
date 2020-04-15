#include<stdio.h>
#include<stdlib.h>

/* Transitory code, to be transformed into bash or python */

void start(int max, char** buffer)
{
	//fprintf(stdout,"Activating PulseAudio...\n");
	system("pactl info &> /dev/null");
}

int main(int argc, char** argv)
{
	start(argc,argv);
	return EXIT_SUCCESS;
}

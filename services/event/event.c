#include <stdio.h>
#include <SDL2/SDL.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9930
#define LOOP 1

void action(const char* message){
    int s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    char buffer[BUFLEN];

    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);
	bzero((char*) &address,address_length);
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = INADDR_ANY;

    bind(s,(struct sockaddr*)&address,sizeof(struct sockaddr_in));

    strcpy(buffer,message);
    sendto(s,buffer,BUFLEN,0,(struct sockaddr*)&address,address_length);
}

void start(int max, char** buffer)
{
    int joysticks,i;
    SDL_Event event;
    SDL_Init(SDL_INIT_JOYSTICK);
    joysticks = SDL_NumJoysticks();
    if(joysticks == 0){
        fprintf(stdout,"No joysticks were found.\n");
        exit(EXIT_FAILURE);
    }

    // Get the first joystick of the list
    SDL_Joystick* js = SDL_JoystickOpen(0);
    char guid_str[1024];
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);
    SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
    const char* name = SDL_JoystickName(js);

    int axes = SDL_JoystickNumAxes(js);
    int hats = SDL_JoystickNumHats(js);
    int balls = SDL_JoystickNumBalls(js);
    int buttons = SDL_JoystickNumButtons(js);

    if(buttons < 2){
        fprintf(stdout,"There aren't enough buttons to control.\n");
        exit(EXIT_FAILURE);
    }

    if(hats == 0){
        fprintf(stdout,"There aren't any Hat to use to move.\n");
        exit(EXIT_FAILURE);
    }

    /* fprintf(stdout,"%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n",
               guid_str, name, axes, buttons, hats, balls); */
    while(LOOP){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_JOYBUTTONDOWN:
                    if(event.jbutton.button == 0) action("Select");
                    else if(event.jbutton.button = 1) action("Back");
                    break;
                case SDL_JOYHATMOTION:
                    if(event.jhat.value & SDL_HAT_UP) action("Up");
                    else if(event.jhat.value & SDL_HAT_DOWN) action("Down");
                    else if(event.jhat.value & SDL_HAT_LEFT) action("Left");
                    else if(event.jhat.value & SDL_HAT_RIGHT) action("Right");
                    break;
            }
        }
    }
    SDL_JoystickClose(js);
    SDL_Quit();
}

int main(int argc, char** argv)
{
    start(argc,argv);
    return EXIT_SUCCESS;
}

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

void start (int max, char** buffer)
{
    int joysticks,i;
    SDL_Init(SDL_INIT_JOYSTICK);
    joysticks = SDL_NumJoysticks();
    char message[512];
    strcpy(message,"Right");
    action(message);
    //system("kodi/client");
    for (i = 0; i < joysticks; i++) {
        SDL_Joystick* js = SDL_JoystickOpen(i);
        if (js) {
            char guid_str[1024];
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            const char* name = SDL_JoystickName(js);

            int axes = SDL_JoystickNumAxes(js);
            int hats = SDL_JoystickNumHats(js);
            int balls = SDL_JoystickNumBalls(js);
            int buttons = SDL_JoystickNumButtons(js);

            printf("%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n",
                   guid_str, name, axes, buttons, hats, balls);

            SDL_JoystickClose(js);
        }
    }

    SDL_Quit();
}

#include "event.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 9930
#define forever while(1)

int s;
CXBMCClient* client;

CAddress::CAddress(int Port)
{
    m_Addr.sin_family = AF_INET;
    m_Addr.sin_port = htons(Port);
    m_Addr.sin_addr.s_addr = INADDR_ANY;
    memset(m_Addr.sin_zero, '\0', sizeof m_Addr.sin_zero);
}

CAddress::CAddress(const char *Address, int Port)
{
    m_Addr.sin_port = htons(Port);
    struct hostent *h;
    if (Address == NULL || (h=gethostbyname(Address)) == NULL) {
        if (Address != NULL) printf("Error: Get host by name\n");
        m_Addr.sin_addr.s_addr  = INADDR_ANY;
        m_Addr.sin_family       = AF_INET;
    } else {
        m_Addr.sin_family = h->h_addrtype;
        m_Addr.sin_addr = *((struct in_addr *)h->h_addr);
    }
    memset(m_Addr.sin_zero, '\0', sizeof m_Addr.sin_zero);
}

void CAddress::SetPort(int port) { m_Addr.sin_port = htons(port); }
bool CAddress::Bind(int Sockfd){ return (bind(Sockfd, (struct sockaddr *)&m_Addr, sizeof m_Addr) == 0); }
const sockaddr* CAddress::GetAddress() { return ((struct sockaddr *)&m_Addr); }

XBMCClientUtils::XBMCClientUtils() {}
XBMCClientUtils::~XBMCClientUtils() {}

unsigned int XBMCClientUtils::GetUniqueIdentifier()
{
    static time_t id = time(NULL);
    return id;
}

void XBMCClientUtils::Clean()
{
    #ifdef _WIN32
    WSACleanup();
    #endif
}

bool XBMCClientUtils::Initialize()
{
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData)) return false;
    #endif
    return true;
}

CXBMCClient::CXBMCClient(const char *IP, int Port, int Socket, unsigned int UID)
{
    m_Addr = CAddress(IP, Port);
    if (Socket == -1) m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
    else m_Socket = Socket;
    if (UID) m_UID = UID;
    else m_UID = XBMCClientUtils::GetUniqueIdentifier();
}

void CXBMCClient::SendNOTIFICATION(const char *Title, const char *Message, unsigned short IconType, const char *IconFile = NULL)
{
    if (m_Socket < 0) return;
    CPacketNOTIFICATION notification(Title, Message, IconType, IconFile);
    notification.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendHELO(const char *DevName, unsigned short IconType, const char *IconFile = NULL)
{
    if (m_Socket < 0) return;
    CPacketHELO helo(DevName, IconType, IconFile);
    helo.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendButton(const char *Button, const char *DeviceMap, unsigned short Flags, unsigned short Amount = 0)
{
    if (m_Socket < 0) return;
    CPacketBUTTON button(Button, DeviceMap, Flags, Amount);
    button.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendButton(unsigned short ButtonCode, const char *DeviceMap, unsigned short Flags, unsigned short Amount = 0)
{
    if (m_Socket < 0) return;
    CPacketBUTTON button(ButtonCode, DeviceMap, Flags, Amount);
    button.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendButton(unsigned short ButtonCode, unsigned Flags, unsigned short Amount = 0)
{
    if (m_Socket < 0) return;
    CPacketBUTTON button(ButtonCode, Flags, Amount);
    button.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendMOUSE(int X, int Y, unsigned char Flag = MS_ABSOLUTE)
{
    if (m_Socket < 0) return;
    CPacketMOUSE mouse(X, Y, Flag);
    mouse.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendLOG(int LogLevel, const char *Message, bool AutoPrintf = true)
{
    if (m_Socket < 0) return;
    CPacketLOG log(LogLevel, Message, AutoPrintf);
    log.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendACTION(const char *ActionMessage, int ActionType = ACTION_EXECBUILTIN)
{
    if (m_Socket < 0) return;
    CPacketACTION action(ActionMessage, ActionType);
    action.Send(m_Socket, m_Addr, m_UID);
}

void CXBMCClient::SendBYE()
{
    if (m_Socket < 0) return;
    CPacketBYE* bye = new CPacketBYE();
    bye->Send(m_Socket, m_Addr, m_UID);
    delete bye;
}

void CXBMCClient::SendPING()
{
    if (m_Socket < 0) return;
    CPacketPING* ping = new CPacketPING();
    ping->Send(m_Socket, m_Addr, m_UID);
    delete ping;
}

void shutdown(int s)
{
    std::cout << "Shutdown" << std::endl;
    close(s);
    client->SendBYE();
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    char buf[BUFLEN];
    struct sockaddr_in address, client_address;
    socklen_t client_address_length = sizeof(client_address);
    signal(SIGTERM,shutdown);
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset((char *) &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(s, (sockaddr*) &address, sizeof(address));
    client = new CXBMCClient();
    client->SendHELO("HUB",ICON_NONE);
    forever {
        recvfrom(s, buf, BUFLEN, 0, (sockaddr*) &client_address, &client_address_length);
        if (strstr(buf,"Exit")) break;
        client->SendACTION(buf,ACTION_BUTTON);
    }
    return 0;
}

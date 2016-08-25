#ifndef __KODI_CLIENT_H__
#define __KODI_CLIENT_H__

/*
 *      Copyright (C) 2008-2015 Team Kodi
 *      Copyright (C) 2016 Efforia
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>

#define STD_PORT       9777

#define MS_ABSOLUTE    0x01
//#define MS_RELATIVE    0x02

#define BTN_USE_NAME   0x01
#define BTN_DOWN       0x02
#define BTN_UP         0x04
#define BTN_USE_AMOUNT 0x08
#define BTN_QUEUE      0x10
#define BTN_NO_REPEAT  0x20
#define BTN_VKEY       0x40
#define BTN_AXIS       0x80

#define PT_HELO         0x01
#define PT_BYE          0x02
#define PT_BUTTON       0x03
#define PT_MOUSE        0x04
#define PT_PING         0x05
#define PT_BROADCAST    0x06
#define PT_NOTIFICATION 0x07
#define PT_BLOB         0x08
#define PT_LOG          0x09
#define PT_ACTION       0x0A
#define PT_DEBUG        0xFF

#define ICON_NONE       0x00
#define ICON_JPEG       0x01
#define ICON_PNG        0x02
#define ICON_GIF        0x03

#define MAX_PACKET_SIZE  1024
#define HEADER_SIZE      32
#define MAX_PAYLOAD_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)

#define MAJOR_VERSION 2
#define MINOR_VERSION 0

#define LOGDEBUG   0
#define LOGINFO    1
#define LOGNOTICE  2
#define LOGWARNING 3
#define LOGERROR   4
#define LOGSEVERE  5
#define LOGFATAL   6
#define LOGNONE    7

#define ACTION_EXECBUILTIN 0x01
#define ACTION_BUTTON      0x02

class CAddress {
    private:
        struct sockaddr_in m_Addr;
    public:
        CAddress(int Port = STD_PORT);
        CAddress(const char *Address, int Port = STD_PORT);
        void SetPort(int port);
        const sockaddr *GetAddress();
        bool Bind(int Sockfd);
};

class XBMCClientUtils {
    public:
        XBMCClientUtils();
        ~XBMCClientUtils();
        static unsigned int GetUniqueIdentifier();
        static void Clean();
        static bool Initialize();
};

class CPacket {
/*   Base class that implements a single event packet.

     - Generic packet structure (maximum 1024 bytes per packet)
     - Header is 32 bytes long, so 992 bytes available for payload
     - large payloads can be split into multiple packets using H4 and H5
       H5 should contain total no. of packets in such a case
     - H6 contains length of P1, which is limited to 992 bytes
     - if H5 is 0 or 1, then H4 will be ignored (single packet msg)
     - H7 must be set to zeros for now

         -----------------------------
         | -H1 Signature ("XBMC")    | - 4  x CHAR                4B
         | -H2 Version (eg. 2.0)     | - 2  x UNSIGNED CHAR       2B
         | -H3 PacketType            | - 1  x UNSIGNED SHORT      2B
         | -H4 Sequence number       | - 1  x UNSIGNED LONG       4B
         | -H5 No. of packets in msg | - 1  x UNSIGNED LONG       4B
         | -H6 Payload size          | - 1  x UNSIGNED SHORT      2B
         | -H7 Client's unique token | - 1  x UNSIGNED LONG       4B
         | -H8 Reserved              | - 10 x UNSIGNED CHAR      10B
         |---------------------------|
         | -P1 payload               | -
         -----------------------------
*/
    public:
        CPacket();
        virtual ~CPacket();
        bool Send(int Socket, CAddress &Addr, unsigned int UID);
    protected:
        char            m_Header[HEADER_SIZE];
        unsigned short  m_PacketType;
        std::vector<char> m_Payload;
        static void ConstructHeader(int PacketType, int NumberOfPackets, int CurrentPacket, unsigned short PayloadSize, unsigned int UniqueToken, char *Header);
        virtual void ConstructPayload();
};

class CPacketHELO : public CPacket {
    /************************************************************************/
    /* Payload format                                                       */
    /* %s -  device name (max 128 chars)                                    */
    /* %c -  icontype ( 0=>NOICON, 1=>JPEG , 2=>PNG , 3=>GIF )              */
    /* %s -  my port ( 0=>not listening )                                   */
    /* %d -  reserved1 ( 0 )                                                */
    /* %d -  reserved2 ( 0 )                                                */
    /* XX -  imagedata ( can span multiple packets )                        */
    /************************************************************************/
    private:
        std::vector<char> m_DeviceName;
        unsigned short m_IconType;
        char *m_IconData;
        unsigned short m_IconSize;
    public:
        virtual void ConstructPayload();
        CPacketHELO(const char *DevName, unsigned short IconType, const char *IconFile);
        virtual ~CPacketHELO();
};

class CPacketNOTIFICATION : public CPacket {
    /************************************************************************/
    /* Payload format:                                                      */
    /* %s - caption                                                         */
    /* %s - message                                                         */
    /* %c - icontype ( 0=>NOICON, 1=>JPEG , 2=>PNG , 3=>GIF )               */
    /* %d - reserved ( 0 )                                                  */
    /* XX - imagedata ( can span multiple packets )                         */
    /************************************************************************/
    private:
        std::vector<char> m_Title;
        std::vector<char> m_Message;
        unsigned short m_IconType;
        char *m_IconData;
        unsigned short m_IconSize;
    public:
        virtual void ConstructPayload();
        CPacketNOTIFICATION(const char *Title, const char *Message, unsigned short IconType, const char *IconFile);
        virtual ~CPacketNOTIFICATION();
};

class CPacketBUTTON : public CPacket {
    /************************************************************************/
    /* Payload format                                                       */
    /* %i - button code                                                     */
    /* %i - flags 0x01 => use button map/name instead of code               */
    /*            0x02 => btn down                                          */
    /*            0x04 => btn up                                            */
    /*            0x08 => use amount                                        */
    /*            0x10 => queue event                                       */
    /*            0x20 => do not repeat                                     */
    /*            0x40 => virtual key                                       */
    /*            0x40 => axis key                                          */
    /* %i - amount ( 0 => 65k maps to -1 => 1 )                             */
    /* %s - device map (case sensitive and required if flags & 0x01)        */
    /*      "KB" - Standard keyboard map                                    */
    /*      "XG" - Xbox Gamepad                                             */
    /*      "R1" - Xbox Remote                                              */
    /*      "R2" - Xbox Universal Remote                                    */
    /*      "LI:devicename" -  valid LIRC device map where 'devicename'     */
    /*                         is the actual name of the LIRC device        */
    /*      "JS<num>:joyname" -  valid Joystick device map where            */
    /*                           'joyname'  is the name specified in        */
    /*                           the keymap. JS only supports button code   */
    /*                           and not button name currently (!0x01).     */
    /* %s - button name (required if flags & 0x01)                          */
    /************************************************************************/
    private:
        std::vector<char> m_DeviceMap;
        std::vector<char> m_Button;
        unsigned short m_ButtonCode;
        unsigned short m_Amount;
        unsigned short m_Flags;
    public:
        virtual void ConstructPayload();
        CPacketBUTTON(const char *Button, const char *DeviceMap, unsigned short Flags, unsigned short Amount);
        CPacketBUTTON(unsigned short ButtonCode, const char *DeviceMap, unsigned short Flags, unsigned short Amount);
        CPacketBUTTON(unsigned short ButtonCode, unsigned short Flags, unsigned short Amount);
        // Used to send a release event
        CPacketBUTTON();
        virtual ~CPacketBUTTON();
        inline unsigned short GetFlags();
        inline unsigned short GetButtonCode();
};

class CPacketPING : public CPacket {
    /************************************************************************/
    /* no payload                                                           */
    /************************************************************************/
    public:
        CPacketPING();
        virtual ~CPacketPING();
};

class CPacketBYE : public CPacket {
    /************************************************************************/
    /* no payload                                                           */
    /************************************************************************/
    public:
        CPacketBYE();
        virtual ~CPacketBYE();
};

class CPacketMOUSE : public CPacket {
    /************************************************************************/
    /* Payload format                                                       */
    /* %c - flags                                                           */
    /*    - 0x01 absolute position                                          */
    /* %i - mousex (0-65535 => maps to screen width)                        */
    /* %i - mousey (0-65535 => maps to screen height)                       */
    /************************************************************************/
    private:
        unsigned short m_X;
        unsigned short m_Y;
        unsigned char  m_Flag;
    public:
        CPacketMOUSE(int X, int Y, unsigned char Flag);
        virtual void ConstructPayload();
        virtual ~CPacketMOUSE();
};

class CPacketLOG : public CPacket {
    /************************************************************************/
    /* Payload format                                                       */
    /* %c - log type                                                        */
    /* %s - message                                                         */
    /************************************************************************/
    private:
        std::vector<char> m_Message;
        unsigned char  m_LogLevel;
        bool m_AutoPrintf;
    public:
        CPacketLOG(int LogLevel, const char *Message, bool AutoPrintf);
        virtual void ConstructPayload();
        virtual ~CPacketLOG();
};

class CPacketACTION : public CPacket {
    /************************************************************************/
    /* Payload format                                                       */
    /* %c - action type                                                     */
    /* %s - action message                                                  */
    /************************************************************************/
    private:
        unsigned char     m_ActionType;
        std::vector<char> m_Action;
    public:
        CPacketACTION(const char *Action, unsigned char ActionType);
        virtual void ConstructPayload();
        virtual ~CPacketACTION();
};

class CXBMCClient {
    private:
        CAddress      m_Addr;
        int           m_Socket;
        unsigned int  m_UID;
    public:
        CXBMCClient(const char *IP = "127.0.0.1", int Port = 9777, int Socket = -1, unsigned int UID = 0);
        void SendNOTIFICATION(const char *Title, const char *Message, unsigned short IconType, const char *IconFile);
        void SendHELO(const char *DevName, unsigned short IconType, const char *IconFile);
        void SendButton(const char *Button, const char *DeviceMap, unsigned short Flags, unsigned short Amount);
        void SendButton(unsigned short ButtonCode, const char *DeviceMap, unsigned short Flags, unsigned short Amount);
        void SendButton(unsigned short ButtonCode, unsigned Flags, unsigned short Amount);
        void SendMOUSE(int X, int Y, unsigned char Flag);
        void SendLOG(int LogLevel, const char *Message, bool AutoPrintf);
        void SendACTION(const char *ActionMessage, int ActionType);
        void SendBYE();
        void SendPING();
};

#endif

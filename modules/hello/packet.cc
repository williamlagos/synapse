#include "event.h"

CPacket::CPacket()
{
    m_PacketType = 0;
}

CPacket::~CPacket() {}

bool CPacket::Send(int Socket, CAddress &Addr, unsigned int UID = XBMCClientUtils::GetUniqueIdentifier())
{
    if (m_Payload.empty()) ConstructPayload();
    bool SendSuccessfull = true;
    int NbrOfPackages = (m_Payload.size() / MAX_PAYLOAD_SIZE) + 1;
    int Send = 0;
    int Sent = 0;
    int Left = m_Payload.size();
    for (int Package = 1; Package <= NbrOfPackages; Package++) {
        if (Left > MAX_PAYLOAD_SIZE) {
          Send = MAX_PAYLOAD_SIZE;
          Left -= Send;
        } else {
          Send = Left;
          Left = 0;
        }
        ConstructHeader(m_PacketType, NbrOfPackages, Package, Send, UID, m_Header);
        int i, j;
        char t[MAX_PACKET_SIZE];
        for (i = 0; i < 32; i++) t[i] = m_Header[i];
        for (j = 0; j < Send; j++) t[(32 + j)] = m_Payload[j + Sent];
        int rtn = sendto(Socket, t, (32 + Send), 0, Addr.GetAddress(), sizeof(struct sockaddr));
        if (rtn != (32 + Send)) SendSuccessfull = false;
        Sent += Send;
    }
    return SendSuccessfull;
}

void CPacket::ConstructHeader(int PacketType, int NumberOfPackets, int CurrentPacket, unsigned short PayloadSize, unsigned int UniqueToken, char *Header)
{
    sprintf(Header, "XBMC");
    for (int i = 4; i < HEADER_SIZE; i++) Header[i] = 0;
    Header[4]  = MAJOR_VERSION;
    Header[5]  = MINOR_VERSION;
    if (CurrentPacket == 1) {
        Header[6]  = ((PacketType & 0xff00) >> 8);
        Header[7]  =  (PacketType & 0x00ff);
    } else {
        Header[6]  = ((PT_BLOB & 0xff00) >> 8);
        Header[7]  =  (PT_BLOB & 0x00ff);
    }
    Header[8]  = ((CurrentPacket & 0xff000000) >> 24);
    Header[9]  = ((CurrentPacket & 0x00ff0000) >> 16);
    Header[10] = ((CurrentPacket & 0x0000ff00) >> 8);
    Header[11] =  (CurrentPacket & 0x000000ff);
    Header[12] = ((NumberOfPackets & 0xff000000) >> 24);
    Header[13] = ((NumberOfPackets & 0x00ff0000) >> 16);
    Header[14] = ((NumberOfPackets & 0x0000ff00) >> 8);
    Header[15] =  (NumberOfPackets & 0x000000ff);
    Header[16] = ((PayloadSize & 0xff00) >> 8);
    Header[17] =  (PayloadSize & 0x00ff);
    Header[18] = ((UniqueToken & 0xff000000) >> 24);
    Header[19] = ((UniqueToken & 0x00ff0000) >> 16);
    Header[20] = ((UniqueToken & 0x0000ff00) >> 8);
    Header[21] =  (UniqueToken & 0x000000ff);
}

void CPacket::ConstructPayload() {}

void CPacketHELO::ConstructPayload()
{
    m_Payload.clear();
    for (unsigned int i = 0; i < m_DeviceName.size(); i++)
        m_Payload.push_back(m_DeviceName[i]);
    m_Payload.push_back('\0');
    m_Payload.push_back(m_IconType);
    m_Payload.push_back(0);
    m_Payload.push_back('\0');
    for (int j = 0; j < 8; j++) m_Payload.push_back(0);
    for (int ico = 0; ico < m_IconSize; ico++)
        m_Payload.push_back(m_IconData[ico]);
}

CPacketHELO::CPacketHELO(const char *DevName, unsigned short IconType, const char *IconFile = NULL) : CPacket()
{
    m_PacketType = PT_HELO;
    unsigned int len = strlen(DevName);
    for (unsigned int i = 0; i < len; i++)
        m_DeviceName.push_back(DevName[i]);
    m_IconType = IconType;
    if (IconType == ICON_NONE || IconFile == NULL) {
        m_IconData = NULL;
        m_IconSize = 0;
        return;
    }
    std::ifstream::pos_type size;
    std::ifstream file (IconFile, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        m_IconData = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (m_IconData, size);
        file.close();
        m_IconSize = size;
    } else {
        m_IconType = ICON_NONE;
        m_IconSize = 0;
    }
}

CPacketHELO::~CPacketHELO()
{
    m_DeviceName.clear();
    delete[] m_IconData;
}

void CPacketNOTIFICATION::ConstructPayload()
{
    m_Payload.clear();
    for (unsigned int i = 0; i < m_Title.size(); i++)
        m_Payload.push_back(m_Title[i]);
    m_Payload.push_back('\0');
    for (unsigned int i = 0; i < m_Message.size(); i++)
        m_Payload.push_back(m_Message[i]);
    m_Payload.push_back('\0');
    m_Payload.push_back(m_IconType);
    for (int i = 0; i < 4; i++) m_Payload.push_back(0);
    for (int ico = 0; ico < m_IconSize; ico++)
        m_Payload.push_back(m_IconData[ico]);
}

CPacketNOTIFICATION::CPacketNOTIFICATION(const char *Title, const char *Message, unsigned short IconType, const char *IconFile = NULL) : CPacket()
{
    m_PacketType = PT_NOTIFICATION;
    m_IconData = NULL;
    m_IconSize = 0;
    unsigned int len = 0;
    if (Title != NULL) {
        len = strlen(Title);
        for (unsigned int i = 0; i < len; i++)
            m_Title.push_back(Title[i]);
    }
    if (Message != NULL) {
        len = strlen(Message);
        for (unsigned int i = 0; i < len; i++)
            m_Message.push_back(Message[i]);
    }
    m_IconType = IconType;
    if (IconType == ICON_NONE || IconFile == NULL) return;
    std::ifstream::pos_type size;
    std::ifstream file (IconFile, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        m_IconData = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (m_IconData, size);
        file.close();
        m_IconSize = size;
    } else m_IconType = ICON_NONE;
}

CPacketNOTIFICATION::~CPacketNOTIFICATION()
{
    m_Title.clear();
    m_Message.clear();
    delete[] m_IconData;
}

void CPacketBUTTON::ConstructPayload()
{
    m_Payload.clear();
    if (m_Button.size() != 0) {
        if (!(m_Flags & BTN_USE_NAME)) m_Flags |= BTN_USE_NAME;
        m_ButtonCode = 0;
    } else m_Button.clear();
    if (m_Amount > 0) {
        if (!(m_Flags & BTN_USE_AMOUNT))
        m_Flags |= BTN_USE_AMOUNT;
    }
    if (!((m_Flags & BTN_DOWN) || (m_Flags & BTN_UP))) m_Flags |= BTN_DOWN;
    m_Payload.push_back(((m_ButtonCode & 0xff00) >> 8));
    m_Payload.push_back( (m_ButtonCode & 0x00ff));
    m_Payload.push_back(((m_Flags & 0xff00) >> 8) );
    m_Payload.push_back( (m_Flags & 0x00ff));
    m_Payload.push_back(((m_Amount & 0xff00) >> 8) );
    m_Payload.push_back( (m_Amount & 0x00ff));
    for (unsigned int i = 0; i < m_DeviceMap.size(); i++)
        m_Payload.push_back(m_DeviceMap[i]);
    m_Payload.push_back('\0');
    for (unsigned int i = 0; i < m_Button.size(); i++)
        m_Payload.push_back(m_Button[i]);
    m_Payload.push_back('\0');
}

CPacketBUTTON::CPacketBUTTON(const char *Button, const char *DeviceMap, unsigned short Flags, unsigned short Amount = 0) : CPacket()
{
    m_PacketType = PT_BUTTON;
    m_Flags      = Flags;
    m_ButtonCode = 0;
    m_Amount     = Amount;
    unsigned int len = strlen(DeviceMap);
    for (unsigned int i = 0; i < len; i++)
        m_DeviceMap.push_back(DeviceMap[i]);
    len = strlen(Button);
    for (unsigned int i = 0; i < len; i++)
        m_Button.push_back(Button[i]);
}

CPacketBUTTON::CPacketBUTTON(unsigned short ButtonCode, const char *DeviceMap, unsigned short Flags, unsigned short Amount = 0) : CPacket()
{
    m_PacketType = PT_BUTTON;
    m_Flags      = Flags;
    m_ButtonCode = ButtonCode;
    m_Amount     = Amount;
    unsigned int len = strlen(DeviceMap);
    for (unsigned int i = 0; i < len; i++)
        m_DeviceMap.push_back(DeviceMap[i]);
}

CPacketBUTTON::CPacketBUTTON(unsigned short ButtonCode, unsigned short Flags, unsigned short Amount = 0) : CPacket()
{
    m_PacketType = PT_BUTTON;
    m_Flags      = Flags;
    m_ButtonCode = ButtonCode;
    m_Amount     = Amount;
}

CPacketBUTTON::CPacketBUTTON() : CPacket()
{
    m_PacketType = PT_BUTTON;
    m_Flags = BTN_UP;
    m_Amount = 0;
    m_ButtonCode = 0;
}

CPacketBUTTON::~CPacketBUTTON()
{
    m_DeviceMap.clear();
    m_Button.clear();
}

inline unsigned short CPacketBUTTON::GetFlags() { return m_Flags; }
inline unsigned short CPacketBUTTON::GetButtonCode() { return m_ButtonCode; }

CPacketPING::CPacketPING() : CPacket() { m_PacketType = PT_PING; }
CPacketPING::~CPacketPING() { }

CPacketBYE::CPacketBYE() : CPacket() { m_PacketType = PT_BYE; }
CPacketBYE::~CPacketBYE() { }

CPacketMOUSE::CPacketMOUSE(int X, int Y, unsigned char Flag = MS_ABSOLUTE)
{
    m_PacketType = PT_MOUSE;
    m_Flag = Flag;
    m_X = X;
    m_Y = Y;
}

void CPacketMOUSE::ConstructPayload()
{
    m_Payload.clear();
    m_Payload.push_back(m_Flag);
    m_Payload.push_back(((m_X & 0xff00) >> 8));
    m_Payload.push_back( (m_X & 0x00ff));
    m_Payload.push_back(((m_Y & 0xff00) >> 8));
    m_Payload.push_back( (m_Y & 0x00ff));
}

CPacketMOUSE::~CPacketMOUSE() { }

CPacketLOG::CPacketLOG(int LogLevel, const char *Message, bool AutoPrintf = true)
{
    m_PacketType = PT_LOG;
    unsigned int len = strlen(Message);
    for (unsigned int i = 0; i < len; i++)
        m_Message.push_back(Message[i]);
    m_LogLevel = LogLevel;
    m_AutoPrintf = AutoPrintf;
}

void CPacketLOG::ConstructPayload()
{
    m_Payload.clear();
    m_Payload.push_back( (m_LogLevel & 0x00ff) );
    if (m_AutoPrintf) {
        char* str=&m_Message[0];
        printf("%s\n", str);
    }
    for (unsigned int i = 0; i < m_Message.size(); i++)
        m_Payload.push_back(m_Message[i]);
    m_Payload.push_back('\0');
}

CPacketLOG::~CPacketLOG() { }

CPacketACTION::CPacketACTION(const char *Action, unsigned char ActionType = ACTION_EXECBUILTIN)
{
    m_PacketType = PT_ACTION;
    m_ActionType = ActionType;
    unsigned int len = strlen(Action);
    for (unsigned int i = 0; i < len; i++)
        m_Action.push_back(Action[i]);
}

void CPacketACTION::ConstructPayload()
{
    m_Payload.clear();
    m_Payload.push_back(m_ActionType);
    for (unsigned int i = 0; i < m_Action.size(); i++)
        m_Payload.push_back(m_Action[i]);
    m_Payload.push_back('\0');
}

CPacketACTION::~CPacketACTION() { }

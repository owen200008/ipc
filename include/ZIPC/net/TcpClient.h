/***********************************************************************************************
// filename : 	tcpclient.h
// describ:		tcp net
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "TcpSessionNotify.h"
#ifdef _MSC_VER
#include <winsock2.h>    
#include <windows.h>
#else

#endif

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetClient : public NetSessionNotify {
public:
    NetClient();
    virtual ~NetClient();

    //! [IPv6Address]:port || IPv4Address:port
    virtual int32_t Connect(const char* lpszAddress);

    //! reconnect
    int32_t DoConnect();

    const sockaddr_storage& GetAddr() {
        return m_addr;
    }
    const int GetAddrLen() {
        return m_addrlen;
    }
protected:
    bool                    m_bAddrSuccess = false;
    sockaddr_storage        m_addr;
    int                     m_addrlen = sizeof(sockaddr_storage);
};

__NS_ZILLIZ_IPC_END
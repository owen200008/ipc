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
#include <sys/socket.h>
#endif

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TcpThreadSocketClient;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TcpClient : public NetSessionNotify {
public:
    virtual ~TcpClient();

    //! 
    static std::shared_ptr<TcpClient> CreateTcpClient() {
        auto pClient = std::shared_ptr<TcpClient>(new TcpClient());
        pClient->InitTcpClient();
        return pClient;
    }

    //! must be call first
    void InitTcpClient();

    //! [IPv6Address]:port || IPv4Address:port
    virtual int32_t Connect(const char* lpszAddress);

    //! reconnect
    int32_t DoConnect();

    //! shutdown
    void Shutdown();

    //!
    bool IsShutdown();
protected:
    TcpClient();

    //!
    virtual TcpThreadSocket * GetThreadSocket() override;
protected:
    TcpThreadSocketClient*  m_pSocket;
    bool                    m_bAddrSuccess = false;
    sockaddr_storage        m_addr;
    int                     m_addrlen = sizeof(sockaddr_storage);
};

__NS_ZILLIZ_IPC_END

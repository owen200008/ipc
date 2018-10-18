/***********************************************************************************************
// filename : 	TcpAccept.h
// describ:		tcp listen link
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "event.h"
#include "evdns.h"
#include "ZIPC/net/TcpDefine.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
class NetThread;
class TcpServer;

#define READBUFFERSIZE_MSG			16384
class TcpThreadAccept {
public:
    TcpThreadAccept(NetThread* pThread);
    virtual ~TcpThreadAccept();
public:
    //get socketid
    evutil_socket_t & GetSocketID() { return m_socketfd; }

    bool IsListen() {
        return m_socketfd != INVALID_SOCKET;
    }
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! need init socket
    void InitTcpSocket(std::shared_ptr<TcpServer>& pSession);

    //!
    int32_t Listen(const sockaddr_storage& addr, int addrlen);

    //!
    void Close();
protected:
    void OnAccept();

    friend void OnLinkListenRead(evutil_socket_t fd, short event, void *arg);
protected:
    NetThread*                              m_pThread;
    std::shared_ptr<TcpServer>	            m_pNotify;
    evutil_socket_t					        m_socketfd = INVALID_SOCKET;
    event							        m_revent;
};

void OnLinkListenRead(evutil_socket_t fd, short event, void *arg);

__NS_ZILLIZ_IPC_END
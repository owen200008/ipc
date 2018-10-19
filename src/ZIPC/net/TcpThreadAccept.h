/***********************************************************************************************
// filename : 	TcpAccept.h
// describ:		tcp listen link
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "event.h"
#include "evdns.h"
#include "TcpAfx.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetThread;
class TcpServer;
//////////////////////////////////////////////////////////////////////////
class TcpThreadAccept {
public:
    TcpThreadAccept(NetThread* pThread);
    virtual ~TcpThreadAccept();
public:
    bool IsListen() {
        return m_socketfd != INVALID_SOCKET;
    }

    //!
    void ReuseSessionID(uint32_t nSessionID);
protected:
    volatile evutil_socket_t	            m_socketfd = INVALID_SOCKET;
    IPCVector<uint32_t>                     m_vtReuseSessionID;
    IPCVector<uint32_t>                     m_vtReuseSessionIDRun;
    std::mutex                              m_lockReuse;
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! need init socket
    void InitTcpSocket(std::shared_ptr<TcpServer>& pSession);

    //!
    void Listen(evutil_socket_t socketfd);

    //!
    void Close();
protected:
    void OnAccept();

    //!
    uint32_t CreateClientSessionID();

    friend void OnLinkListenRead(evutil_socket_t fd, short event, void *arg);
protected:
    NetThread*                              m_pThread;
    std::shared_ptr<TcpServer>	            m_pNotify;
    
    event							        m_revent;

    uint32_t                                m_nClientSessionMgr = 0;
    IPCVector<uint32_t>                     m_vtThreadReuseSessionID;
};

void OnLinkListenRead(evutil_socket_t fd, short event, void *arg);

__NS_ZILLIZ_IPC_END
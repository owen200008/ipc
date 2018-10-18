#include "TcpThreadAccept.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"
#include "TcpThreadSocketSession.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadAccept::TcpThreadAccept(NetThread* pThread) {
    m_pThread = pThread;
}

TcpThreadAccept::~TcpThreadAccept() {
    if (m_socketfd != INVALID_SOCKET) {
        LogFuncLocation(IPCLog_Error, "TcpAccept::~TcpAccept m_socketfd != INVALID_SOCKET");
    }
}

///////////////////////////////////////////////////////////////////////////////
void TcpThreadAccept::InitTcpSocket(std::shared_ptr<TcpServer>& pSession) {
    m_pNotify = pSession;
}

//!
int32_t TcpThreadAccept::Listen(const sockaddr_storage& addr, int addrlen) {
    if (IsListen()) {
        return BASIC_NET_ALREADY_LISTEN;
    }
    int32_t lReturn = BASIC_NET_OK;
    evutil_socket_t socketfd = INVALID_SOCKET;
    do {
        socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET) {
            lReturn = BASIC_NET_SOCKET_ERROR;
            break;
        }
        evutil_make_socket_nonblocking(socketfd);
        evutil_make_listen_socket_reuseable(socketfd);
        evutil_make_listen_socket_reuseable_port(socketfd);
        // bind our name to the socket
        int nRet = ::bind(socketfd, (::sockaddr*)&addr, addrlen);   //I know this
        if (nRet != 0) {
            lReturn = BASIC_NET_BIND_ERROR;
            break;
        }
        // Set the socket to listen
        nRet = listen(socketfd, 0x7fffffff);
        if (nRet != 0) {
            lReturn = BASIC_NET_LISTEN_ERROR;
            break;
        }
    } while (0);
    if (lReturn == BASIC_NET_OK) {
        m_socketfd = socketfd;
        event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkListenRead, this);
        event_base_set(m_pThread->m_base, &m_revent);
        event_add(&m_revent, NULL);
    }
    else if (socketfd != INVALID_SOCKET) {
        evutil_closesocket(socketfd);
    }
    return lReturn;
}

//!
void TcpThreadAccept::Close() {
    if (m_socketfd != INVALID_SOCKET) {
        event_del(&m_revent);

        evutil_closesocket(m_socketfd);
        m_socketfd = INVALID_SOCKET;
    }
}

///////////////////////////////////////////////////////////////////////////////
void TcpThreadAccept::OnAccept() {
    sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    evutil_socket_t s = accept(m_socketfd, (::sockaddr*)&addr, &addrlen);   //I know this
    if (s == INVALID_SOCKET) {
        return;
    }
    std::shared_ptr<TcpServerSession> pClientSession;
    m_pNotify->ConstructSession(pClientSession);
    //create and add map
    m_pNotify->AddSessionMap(pClientSession);

    auto pSessionSocket = (TcpThreadSocketSession*)pClientSession->GetThreadSocket();
    //change to session thread
    auto pSessionThread = pClientSession->GetNetThread();
    if (pSessionThread->IsSameThread()) {
        pSessionSocket->Accept(s, addr, addrlen);
    }
    else {
        pSessionThread->SetEvent(pClientSession, [s, pSessionSocket, addr, addrlen]() {
            pSessionSocket->Accept(s, addr, addrlen);
        });
    }
}
///////////////////////////////////////////////////////////////////////////////
void OnLinkListenRead(evutil_socket_t fd, short event, void *arg) {
    ((TcpThreadAccept*)arg)->OnAccept();
}

//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



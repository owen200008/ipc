#include "TcpThreadAccept.h"
#include "ZIPC/net/TcpDefine.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"
#include "TcpThreadSocketSession.h"


__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadAccept::TcpThreadAccept(NetThread* pThread) {
    m_pThread = pThread;
    m_vtThreadReuseSessionID.reserve(128);
    m_vtReuseSessionID.reserve(128);
    m_vtReuseSessionIDRun.reserve(128);
}

TcpThreadAccept::~TcpThreadAccept() {
    if (m_socketfd != INVALID_SOCKET) {
        LogFuncLocation(IPCLog_Error, "TcpAccept::~TcpAccept m_socketfd != INVALID_SOCKET");
    }
}

//!
void TcpThreadAccept::ReuseSessionID(uint32_t nSessionID) {
    std::lock_guard<std::mutex> lock(m_lockReuse);
    m_vtReuseSessionID.push_back(nSessionID);
}
///////////////////////////////////////////////////////////////////////////////
void TcpThreadAccept::InitTcpSocket(std::shared_ptr<TcpServer>& pSession) {
    m_pNotify = pSession;
}

//!
void TcpThreadAccept::Listen(evutil_socket_t socketfd) {
    m_socketfd = socketfd;
    event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkListenRead, this);
    event_base_set(m_pThread->m_base, &m_revent);
    event_add(&m_revent, NULL);
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
    pClientSession->InitTcpServerSession(CreateClientSessionID(), m_pNotify);
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

//!
uint32_t TcpThreadAccept::CreateClientSessionID() {
    uint32_t nRet = 0;
    if (m_vtThreadReuseSessionID.size() > 0) {
        nRet = m_vtThreadReuseSessionID.back();
        m_vtThreadReuseSessionID.pop_back();
    }
    else {
        nRet = m_nClientSessionMgr++;
        if (nRet % 100 == 99) {
            {
                std::lock_guard<std::mutex> lock(m_lockReuse);
                swap(m_vtReuseSessionIDRun, m_vtReuseSessionID);
            }
            if (m_vtReuseSessionID.size() > 0) {
                m_vtThreadReuseSessionID.insert(m_vtThreadReuseSessionID.end(), m_vtReuseSessionIDRun.begin(), m_vtReuseSessionIDRun.end());
                m_vtReuseSessionIDRun.clear();
            }
        }
    }
    return nRet;
}
///////////////////////////////////////////////////////////////////////////////
void OnLinkListenRead(evutil_socket_t fd, short event, void *arg) {
    ((TcpThreadAccept*)arg)->OnAccept();
}

//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



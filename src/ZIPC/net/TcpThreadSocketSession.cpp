#include "TcpThreadSocketSession.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadSocketSession::TcpThreadSocketSession(NetThread* pThread) : TcpThreadSocket(pThread){
    memset(m_szPeerAddr, 0, sizeof(m_szPeerAddr));
    m_nPeerPort = 0;
}

TcpThreadSocketSession::~TcpThreadSocketSession() {
}

//!
const char* TcpThreadSocketSession::GetPeerAddressPort(uint32_t& nPort) {
    nPort = m_nPeerPort.load();
    return m_szPeerAddr;
}

//! 
int32_t TcpThreadSocketSession::Accept(evutil_socket_t s, const sockaddr_storage& addr, int addrlen) {
    sockaddr* pAddr = (sockaddr*)&addr;
    int nFamily = pAddr->sa_family;
    if (nFamily == AF_INET) {
        char szBuf[128] = { 0 };
        sockaddr_in* pSockAddr = (struct sockaddr_in*)pAddr;
        strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET, &pSockAddr->sin_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
        m_nPeerPort.store(ntohs(pSockAddr->sin_port));		//I know this
    }
    else {
        char szBuf[128] = { 0 };
        sockaddr_in6* pSockAddr = (struct sockaddr_in6*)pAddr;
        strncpy(m_szPeerAddr, evutil_inet_ntop(AF_INET6, &pSockAddr->sin6_addr, szBuf, sizeof(szBuf)), ADDRESS_MAX_LENGTH);		//I know this
        m_nPeerPort.store(ntohs(pSockAddr->sin6_port));		//I know this
    }

    evutil_make_socket_nonblocking(s);
    evutil_make_listen_socket_reuseable(s);
    evutil_make_listen_socket_reuseable_port(s);

    m_socketfd = s;

    event_set(&m_revent, m_socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
    event_base_set(m_pThread->m_base, &m_revent);
    event_set(&m_wevent, m_socketfd, EV_WRITE, OnLinkWrite, this);
    event_base_set(m_pThread->m_base, &m_wevent);
    //read data
    event_add(&m_revent, NULL);
    OnConnect();
    return BASIC_NET_OK;
}
void TcpThreadSocketSession::OnDisconnect(uint32_t dwNetCode){
    TcpThreadSocket::OnDisconnect(dwNetCode);
    //try to delete object
    m_pNotify = nullptr;
}
//////////////////////////////////////////////////////////////////////////
void TcpThreadSocketSession::ClearMemberData() {
    TcpThreadSocket::ClearMemberData();
}
//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



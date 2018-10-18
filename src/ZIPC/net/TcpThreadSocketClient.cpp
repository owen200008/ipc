#include "TcpThreadSocketClient.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"
#include "TcpAfx.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadSocketClient::TcpThreadSocketClient(NetThread* pThread) : TcpThreadSocket(pThread){
}

TcpThreadSocketClient::~TcpThreadSocketClient() {
}

int32_t TcpThreadSocketClient::Connect(const sockaddr_storage& addr, int addrlen) {
    uint8_t statLink = m_statLink;
    if (statLink != TIL_SS_IDLE) {
        int32_t lRet = BASIC_NET_GENERIC_ERROR;
        switch (statLink) {
        case TIL_SS_CONNECTING: {
            lRet = BASIC_NET_CONNECTING_ERROR;
            break;
        }
        case TIL_SS_CONNECTED: {
            lRet = BASIC_NET_ALREADY_CONNECT;
            break;
        }
        }
        return lRet;
    }
    int32_t lReturn = BASIC_NET_OK;
    evutil_socket_t socketfd = INVALID_SOCKET;
    do {
        evutil_socket_t socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET) {
            lReturn = BASIC_NET_SOCKET_ERROR;
            break;
        }
        evutil_make_socket_nonblocking(socketfd);
        evutil_make_listen_socket_reuseable(socketfd);
        evutil_make_listen_socket_reuseable_port(socketfd);
        int nRet = connect(socketfd, (::sockaddr*)&addr, addrlen);
        if (nRet == 0) {
            m_socketfd = socketfd;
            //event
            event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
            event_base_set(m_pThread->m_base, &m_revent);
            event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
            event_base_set(m_pThread->m_base, &m_wevent);

            event_add(&m_revent, NULL);

            OnConnect();
        }
#ifdef _MSC_VER
        else if (errno == EINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (errno == EINPROGRESS) {
#endif
            m_socketfd = socketfd;
            //wait for write onconnect
            m_statLink = TIL_SS_CONNECTING;
            event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
            event_base_set(m_pThread->m_base, &m_revent);
            event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
            event_base_set(m_pThread->m_base, &m_wevent);

            event_add(&m_wevent, NULL);
        }
        else {
            //int nRetErrorNo = errno;
            lReturn = BASIC_NET_GENERIC_ERROR;
        }
    } while (0);

    if (lReturn != BASIC_NET_OK && socketfd != INVALID_SOCKET) {
        evutil_closesocket(socketfd);
    }
    
    return BASIC_NET_OK;
}

//////////////////////////////////////////////////////////////////////////
void TcpThreadSocketClient::ClearMemberData() {
    TcpThreadSocket::ClearMemberData();
}
//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



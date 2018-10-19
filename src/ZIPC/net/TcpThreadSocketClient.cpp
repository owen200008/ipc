#include "TcpThreadSocketClient.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadSocketClient::TcpThreadSocketClient(NetThread* pThread) : TcpThreadSocket(pThread){
}

TcpThreadSocketClient::~TcpThreadSocketClient() {
}

//!
void TcpThreadSocketClient::ConnectSuccess(evutil_socket_t socketfd) {
    m_socketfd = socketfd;
    //event
    event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
    event_base_set(m_pThread->m_base, &m_revent);
    event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
    event_base_set(m_pThread->m_base, &m_wevent);

    event_add(&m_revent, NULL);

    OnConnect();
}

void TcpThreadSocketClient::ConnectWaitEvent(evutil_socket_t socketfd) {
    m_socketfd = socketfd;
    //wait for write onconnect
    m_statLink = TIL_SS_CONNECTING;
    event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
    event_base_set(m_pThread->m_base, &m_revent);
    event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
    event_base_set(m_pThread->m_base, &m_wevent);

    event_add(&m_wevent, NULL);
}
//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



#include "NetThread.h"
#include "NetMgr.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/conf/IPCConfRead.h"

__NS_ZILLIZ_IPC_START
////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetThread::NetThread(){
    m_vtEvent.reserve(256);
    m_vtEventRun.reserve(256);
    m_vtDeathSocket.reserve(256);
    m_vtCloseSocket.reserve(256);
    m_vtRevertSocket.reserve(256);
    m_vtAllocateSocket.reserve(256);
}

NetThread::~NetThread(){
    for (auto p : m_vtAllocateSocket) {
        delete p;
    }
    m_vtAllocateSocket.clear();
    if(m_base){
        if (m_dnsbase) {
            evdns_base_free(m_dnsbase, 0);
        }

        event_base_loopbreak(m_base);
        m_thread_worker_ptr->join();

        event_del(&notify_event);
        evutil_closesocket(m_pair[0]);
        evutil_closesocket(m_pair[1]);
        event_base_free(m_base);
    }
}

//! init
void NetThread::Init(uint16_t nIndex, struct event_config *cfg){
	if(evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1){
		IPCLog::GetInstance().Log(IPCLog_Throw, "create evutil_socketpair error");
		return;
	}
    m_nIndex = nIndex;

	evutil_make_socket_nonblocking(m_pair[0]);
	evutil_make_socket_nonblocking(m_pair[1]);

    //create self eventbase
	m_base = event_base_new_with_config(cfg);
    event_set(&notify_event, m_pair[0], EV_READ | EV_PERSIST, [](int fd, short event, void *arg) {
        NetThread *thr = (NetThread*)arg;
        thr->RunMessageQueue();
    }, this);
	event_base_set(m_base, &notify_event);
	if(event_add(&notify_event, NULL) == -1){
		IPCLog::GetInstance().Log(IPCLog_Throw, "libevent eventadd error");
		return;
	}
	m_dnsbase = evdns_base_new(m_base, 1);
	if(!m_dnsbase){
		IPCLog::GetInstance().Log(IPCLog_Throw, "libevent eventdns add error");
		return;
	}
	m_thread_worker_ptr = std::make_shared<std::thread>(&NetThread::EventLoop, this);
}

void NetThread::EventLoop(){
	NetMgr::GetInstance().IncrementNetThreadCount();
    event_base_loop(m_base, 0);
    NetMgr::GetInstance().DecrementNetThreadCount();
}

//!
void NetThread::SetEvent(NetThreadEvent& setEvent){
    //may be loop if call on same thread
    /*if(std::this_thread::get_id() == m_thread_worker_ptr->get_id()){
        setEvent.m_callback(setEvent.m_pSession, setEvent.m_lRevert);
        return;
    }*/
    int nSize = 0;
    {
        CSpinLockFunc lock(&m_lockEvent, true);
        nSize = m_vtEvent.size();
        m_vtEvent.push_back(setEvent);
    }
    if(nSize == 0){
        send(m_pair[1], "", 1, 0);
    }
}

void NetThread::RunMessageQueue(){
    char szBuf[NotifyEventBufferSize];
    int nReceived = 0;
    while ((nReceived = recv(m_pair[0], szBuf, NotifyEventBufferSize, 0)) == NotifyEventBufferSize) {
    }
    {
        CSpinLockFunc lock(&m_lockEvent, true);
        swap(m_vtEvent, m_vtEventRun);
    }
    for (auto& setEvent : m_vtEventRun) {
        auto pSocket = setEvent.m_pSession->GetTcpSocket();
        if (pSocket == nullptr) {
            pSocket = AssignTcpSocket();
            pSocket->InitTcpSocket(setEvent.m_pSession);
        }
        setEvent.m_callback(setEvent.m_pSession, pSocket, setEvent.m_lRevert);
    }
}

void NetThread::ReleaseTcpSocket(TcpSocket* p) {
    //first set to death
    m_vtDeathSocket.push_back(p);
}
TcpSocket* NetThread::AssignTcpSocket() {
    if (m_vtRevertSocket.size() > 0) {
        TcpSocket* p = *m_vtRevertSocket.rbegin();
        m_vtRevertSocket.pop_back();
        return p;
    }
    auto p = new TcpSocket(this);
    m_vtAllocateSocket.push_back(p);
    return p;
}
////////////////////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END

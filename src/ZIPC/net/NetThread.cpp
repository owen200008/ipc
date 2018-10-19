#include "NetThread.h"
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/conf/IPCConfRead.h"

__NS_ZILLIZ_IPC_START
////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetThread::NetThread(){
    m_vtEvent.reserve(256);
    m_vtEventRun.reserve(256);
}

NetThread::~NetThread(){
    if(m_base){
        event_base_loopbreak(m_base);
        //send notify
        send(m_pair[1], "", 1, 0);
        m_thread_worker_ptr->join();

        event_del(&notify_event);
        evutil_closesocket(m_pair[0]);
        evutil_closesocket(m_pair[1]);
        
        if (m_dnsbase) {
            evdns_base_free(m_dnsbase, 0);
        }
        event_base_free(m_base);
    }
}

//! init
void NetThread::Init(uint16_t nIndex, struct event_config *cfg){
	if(evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pair) == -1){
        LogFuncLocation(IPCLog_Throw, "create evutil_socketpair error");
		return;
	}

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
        LogFuncLocation(IPCLog_Throw, "libevent eventadd error");
		return;
	}
	m_dnsbase = evdns_base_new(m_base, 1);
	if(!m_dnsbase){
        LogFuncLocation(IPCLog_Throw, "libevent eventdns add error");
		return;
	}
	m_thread_worker_ptr = std::make_shared<std::thread>(&NetThread::EventLoop, this);
}

void NetThread::EventLoop(){
    event_base_loop(m_base, 0);
}

//!
void NetThread::SetEvent(const std::shared_ptr<NetBaseObject>& pSession, const std::function<void()>& func){
    //may be loop if call on same thread so change thread
    /*if(std::this_thread::get_id() == m_thread_worker_ptr->get_id()){
        setEvent.m_callback(setEvent.m_pSession, setEvent.m_lRevert);
        return;
    }*/
    int nSize = 0;
    {
        std::lock_guard<std::mutex> lock(m_lockEvent);
        nSize = m_vtEvent.size();
        m_vtEvent.push_back(std::move(NetThreadEvent(pSession, func)));
    }
    if(nSize == 0){
        send(m_pair[1], "", 1, 0);
    }
}

//! is same thread
bool NetThread::IsSameThread() {
    if (std::this_thread::get_id() == m_thread_worker_ptr->get_id()) {
        return true;
    }
    return false;
}

void NetThread::RunMessageQueue(){
    char szBuf[NotifyEventBufferSize];
    int nReceived = 0;
    while ((nReceived = recv(m_pair[0], szBuf, NotifyEventBufferSize, 0)) == NotifyEventBufferSize) {
    }
    {
        std::lock_guard<std::mutex> lock(m_lockEvent);
        swap(m_vtEvent, m_vtEventRun);
    }
    for (auto& setEvent : m_vtEventRun) {
        setEvent.m_func();
    }
    m_vtEventRun.clear();
}

////////////////////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END

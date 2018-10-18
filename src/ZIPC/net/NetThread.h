/***********************************************************************************************
// filename : 	NetThread.h
// describ:		libevent threads mgr
// version:   	1.0V
************************************************************************************************/
#pragma once
#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/base/mt/SpinLock.h"
#include "TcpThreadSocket.h"
#include "ZIPC/net/NetBaseObject.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (unsigned int)(~0)
#endif

#define NotifyQueueSize         32          //must <= 256
#define NotifyEventBufferSize   4096

struct NetThreadEvent {
    std::shared_ptr<NetBaseObject>                                  m_pSession;
    std::function<void()>   m_func;
    NetThreadEvent(const std::shared_ptr<NetBaseObject>& pSocket, const std::function<void()>& func) {
        m_pSession = pSocket;
        m_func = func;
    }
};

class NetThread {
public:
	NetThread();
	virtual ~NetThread();

    //! init
    void Init(uint16_t nIndex, struct event_config *cfg);

	//! set event
    void SetEvent(const std::shared_ptr<NetBaseObject>& pSession, const std::function<void()>& func);

    //! is same thread
    bool IsSameThread();

    //!get weight
    uint32_t GetWeight() { return m_nWeight; }
protected:
    //! get queue msg
    void RunMessageQueue();

    void EventLoop();
public:
    uint16_t                                    m_nIndex;
    std::shared_ptr<std::thread>                m_thread_worker_ptr;
    SpinLock                                    m_lockEvent;
    IPCVector<NetThreadEvent>                   m_vtEvent;
    IPCVector<NetThreadEvent>                   m_vtEventRun;

    evutil_socket_t					            m_pair[2];
    struct event_base*				            m_base = nullptr;
    struct evdns_base*				            m_dnsbase = nullptr;
    struct event					            notify_event;

    //weight
    volatile uint32_t                           m_nWeight = 0;
};


__NS_ZILLIZ_IPC_END
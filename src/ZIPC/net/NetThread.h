/***********************************************************************************************
// filename : 	NetThread.h
// describ:		libevent threads mgr
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once
#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/net/NetBaseObject.h"
#include "event.h"
#include "evdns.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////
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
protected:
    //! get queue msg
    void RunMessageQueue();

    void EventLoop();
public:
    std::shared_ptr<std::thread>                m_thread_worker_ptr;
    std::mutex                                  m_lockEvent;
    IPCVector<NetThreadEvent>                   m_vtEvent;
    IPCVector<NetThreadEvent>                   m_vtEventRun;

    evutil_socket_t					            m_pair[2];
    struct event_base*				            m_base = nullptr;
    struct evdns_base*				            m_dnsbase = nullptr;
    struct event					            notify_event;
};


__NS_ZILLIZ_IPC_END
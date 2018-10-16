/***********************************************************************************************
// filename : 	NetThread.h
// describ:		libevent threads mgr
// version:   	1.0V
************************************************************************************************/
#pragma once
#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/base/mt/SpinLock.h"
#include "TcpSocket.h"
#include "ZIPC/net/TcpSessionNotify.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////
#define NotifyQueueSize         32          //must <= 256
#define NotifyEventBufferSize   4096

struct NetThreadEvent {
    std::shared_ptr<NetSessionNotify>       m_pSession;
    NetSessionNotify::pNetThreadCallback    m_callback = nullptr;
    intptr_t							    m_lRevert = 0;
    NetThreadEvent() {
    }
    NetThreadEvent(std::shared_ptr<NetSessionNotify> pSocket, NetSessionNotify::pNetThreadCallback pCallback) {
        m_pSession = pSocket;
        m_callback = pCallback;
    }
    NetThreadEvent(std::shared_ptr<NetSessionNotify> pSocket, NetSessionNotify::pNetThreadCallback pCallback, intptr_t lRevert) {
        m_pSession = pSocket;
        m_callback = pCallback;
        m_lRevert = lRevert;
    }
};

class NetThread {
public:
	NetThread();
	virtual ~NetThread();

    //! init
    void Init(uint16_t nIndex, struct event_config *cfg);

	//! set event
    void SetEvent(NetThreadEvent&);

    //!get weight
    uint32_t GetWeight() { return m_nWeight; }
protected:
    TcpSocket* AssignTcpSocket();
    void ReleaseTcpSocket(TcpSocket*);

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
    IPCVector<TcpSocket*>                       m_vtRevertSocket;
    IPCVector<TcpSocket*>                       m_vtCloseSocket;
    IPCVector<TcpSocket*>                       m_vtAllocateSocket;

    friend class TcpSocket;
};


__NS_ZILLIZ_IPC_END
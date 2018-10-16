/***********************************************************************************************
// filename : 	NetMgr.h
// describ:		net mgr
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/base/mt/SpinLock.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////
//net mgr
class NetThread;
class NetMgr{
public:
	static NetMgr& GetInstance();

public:
    NetMgr();
    virtual ~NetMgr();

    //! 
    void Initialize();

    //! 
    void CloseNetSocket();
public:
    NetThread* AssignNetThread();

    void IncrementNetThreadCount();
    void DecrementNetThreadCount();
    uint16_t GetInitNetThreadCount();

    //! calc net threadindex
    uint16_t ReCalcNetThreadIndex();
protected:
    volatile bool                               m_bTimeToKill = false;

    std::shared_ptr<std::thread>                m_thread_ontimer_ptr;

    uint16_t 			                        m_nEventThreadCount = 1;
    std::atomic<uint16_t>                       m_nInitThreadCount = 0;
    NetThread*                                  m_pEventThreads = nullptr;

    //! 获取当前的线程
    volatile uint16_t                           m_nCurrentNetThreadIndex = 0;
    std::atomic<uint16_t>                       m_nNetThreadIndexGetCount = { 0 };
};


__NS_ZILLIZ_IPC_END


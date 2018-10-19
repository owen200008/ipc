/***********************************************************************************************
// filename : 	NetMgr.h
// describ:		net mgr
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"

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
protected:
    std::shared_ptr<std::thread>                m_thread_ontimer_ptr;

    uint16_t 			                        m_nEventThreadCount = 1;
    NetThread*                                  m_pEventThreads = nullptr;

    //! 获取当前的线程
    std::atomic<uint16_t>                       m_nNetThreadIndexGetCount = { 0 };
};


__NS_ZILLIZ_IPC_END


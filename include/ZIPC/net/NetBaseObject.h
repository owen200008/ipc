/***********************************************************************************************
// filename : 	NetBaseObject.h
// describ:		tcp net
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetThread;
///////////////////////////////////////////////////////////////////////////////////////////////
class NetBaseObject : public std::enable_shared_from_this<NetBaseObject> {
protected:
    NetBaseObject();
    virtual ~NetBaseObject();

    std::shared_ptr<NetBaseObject> GetSharedPtr() {
        return shared_from_this();
    }

    //!
    NetThread* GetNetThread() { return m_pNetThread; }
protected:
    NetThread* 			m_pNetThread = nullptr;
};

__NS_ZILLIZ_IPC_END
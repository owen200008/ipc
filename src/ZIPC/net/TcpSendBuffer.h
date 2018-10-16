/***********************************************************************************************
// filename : 	TcpSendBuffer.h
// describ:		
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/base/mem/SmartBuffer.h"

__NS_ZILLIZ_IPC_START
////////////////////////////////////////////////////////////////////////////////////////
class CMsgSendBuffer : public SmartBuffer{
public:
    CMsgSendBuffer();
    virtual ~CMsgSendBuffer();

    //返回剩余的长度
    int32_t SendBuffer(int32_t lSend);
};

__NS_ZILLIZ_IPC_END
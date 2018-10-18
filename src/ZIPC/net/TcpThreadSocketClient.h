/***********************************************************************************************
// filename : 	TcpThreadSocketClient.h
// describ:		tcp socket link
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "TcpThreadSocket.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ADDRESS_MAX_LENGTH		64
///////////////////////////////////////////////////////////////////////////////////////////////
class TcpThreadSocketClient : public TcpThreadSocket {
public:
    TcpThreadSocketClient(NetThread* pThread);
    virtual ~TcpThreadSocketClient();

public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! 
    int32_t Connect(const sockaddr_storage& addr, int addrlen);

protected:
    //!
    virtual void ClearMemberData();
};

__NS_ZILLIZ_IPC_END
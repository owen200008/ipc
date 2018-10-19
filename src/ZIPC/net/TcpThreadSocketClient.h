/***********************************************************************************************
// filename : 	TcpThreadSocketClient.h
// describ:		tcp socket link
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
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
    void ConnectWaitEvent(evutil_socket_t socketfd);

    //!
    void ConnectSuccess(evutil_socket_t socketfd);
};

__NS_ZILLIZ_IPC_END
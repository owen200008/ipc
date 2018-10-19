/***********************************************************************************************
// filename : 	TcpThreadSocketSession.h
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
class TcpThreadSocketSession : public TcpThreadSocket {
public:
    TcpThreadSocketSession(NetThread* pThread);
    virtual ~TcpThreadSocketSession();

    //!
    const char* GetPeerAddressPort(uint32_t& nPort);
protected:
    //port to acquire & release memory order
    char					                    m_szPeerAddr[ADDRESS_MAX_LENGTH];
    std::atomic<uint32_t>				        m_nPeerPort;
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! 
    int32_t Accept(evutil_socket_t s, const sockaddr_storage& addr, int addrlen);

    //! 
    virtual void OnDisconnect(uint32_t dwNetCode) override;
protected:
    //!
    virtual void ClearMemberData();
};

__NS_ZILLIZ_IPC_END
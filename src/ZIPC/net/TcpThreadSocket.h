/***********************************************************************************************
// filename : 	TcpThreadSocket.h
// describ:		tcp socket link
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "event.h"
#include "evdns.h"
#include "TcpAfx.h"
#include <cstring>

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
class NetThread;
class NetSessionNotify;

//////////////////////////////////////////////////////////////////////////
#define TIL_SS_IDLE						0x00		//
#define TIL_SS_CONNECTING				0x01		//
#define TIL_SS_CONNECTED				0x02		//
#define TIL_SS_LISTENING				0x03		//

#define TIL_SS_SHAKEHANDLE_IDLE		    0x00		//
#define TIL_SS_SHAKEHANDLE_TRANSMIT		0x01		//
//////////////////////////////////////////////////////////////////////////
struct SocketSendBuf {
    int32_t                             m_nLength = 0;
    int32_t                             m_nReadLength = 0;
    std::shared_ptr<char>               m_pBuffer;
    SocketSendBuf(){
    }
    SocketSendBuf(const char* pBuf, int32_t nLength) {
        m_nLength = nLength;
        char* pAlloc = new char[nLength];
        memcpy(pAlloc, pBuf, nLength);
        m_pBuffer = std::shared_ptr<char>(pAlloc);
    }
    SocketSendBuf(const std::shared_ptr<char>& pBuf, int32_t nLength) {
        m_nLength = nLength;
        m_pBuffer = pBuf;
    }
};
//////////////////////////////////////////////////////////////////////////

#define READBUFFERSIZE_MSG			16384
class TcpThreadSocket {
public:
    TcpThreadSocket(NetThread* pThread);
    virtual ~TcpThreadSocket();

    //! thread safe
    void InitTcpSocket(std::shared_ptr<NetSessionNotify>& pSession){ m_pNotify = pSession; }

    //!
    bool IsConnected();
    //!
    bool IsTransmit();
    //!
    uint8_t GetStatLink() { return m_statLink; }
protected:
    //! I know this(it is just state oper in the netthread so it is safe memory order)
    volatile uint8_t                                m_statTransmit = TIL_SS_SHAKEHANDLE_IDLE;
    volatile uint8_t                                m_statLink = TIL_SS_IDLE; 
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //!
    void Send(const SocketSendBuf& buf);
    void SendNoSend(const SocketSendBuf& buf);//for test

    //!
    void Close();

    //!
    void Shutdown();
protected:
    void OnReadEvent();
    void OnWriteEvent();

    //! 断开消息
    void OnConnect();
    virtual void OnDisconnect(uint32_t dwNetCode);
    void OnSendData(uint32_t dwIoSize);
    uint32_t OnReceive(const char *pszData, int32_t cbData);

    bool ReadBuffer(int16_t lSend);
    void SendDataFromQueue();

    //! 
    bool CanClose();

    void OnClose(bool bRemote = false, bool bCloseNoCheck = false);

    friend void OnLinkRead(evutil_socket_t fd, short event, void *arg);
    friend void OnLinkWrite(evutil_socket_t fd, short event, void *arg);
protected:
    //!
    virtual void ClearMemberData();
protected:
    NetThread*                              m_pThread;
    std::shared_ptr<NetSessionNotify>	    m_pNotify;
    evutil_socket_t					        m_socketfd = INVALID_SOCKET;
    event							        m_revent;
    event							        m_wevent;
    IPCQueue<SocketSendBuf>                 m_qSocketBuf;
    char                                    m_szReadySendBuffer[READBUFFERSIZE_MSG];
    int16_t                                 m_nReadySendBufferLength = 0;
    bool                                    m_bToClose = false;
};

void OnLinkRead(evutil_socket_t fd, short event, void *arg);
void OnLinkWrite(evutil_socket_t fd, short event, void *arg);

__NS_ZILLIZ_IPC_END
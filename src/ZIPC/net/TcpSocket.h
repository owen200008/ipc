/***********************************************************************************************
// filename : 	TcpSocket.h
// describ:		tcp socket link
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "event.h"
#include "evdns.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
class NetThread;
class NetSessionNotify;
//////////////////////////////////////////////////////////////////////////
struct SocketSendBuf {
    std::shared_ptr<char>               m_pBuffer;
    uint32_t                            m_nLength = 0;
    uint32_t                            m_nReadLength = 0;
};
//////////////////////////////////////////////////////////////////////////
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (unsigned int)(~0)
#endif

#define READBUFFERSIZE_MSG			16384
class TcpSocket {
public:
    TcpSocket(NetThread* pThread);
    virtual ~TcpSocket();
public:
    //get socketid
    evutil_socket_t & GetSocketID() { return m_socketfd; }

    //!
    bool IsConnected();
    //!
    bool IsTransmit();
protected:
    //!
    uint32_t GetSessionStatus(uint32_t dwMask) { return m_unSessionStatus & dwMask; }
    //!
    void SetSessionStatus(uint32_t dwValue, uint32_t dwMask) { m_unSessionStatus &= ~dwMask; m_unSessionStatus |= (dwValue & dwMask); }

    uint32_t					    m_unSessionStatus = 0;
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! need init socket
    static int32_t AssignConnect(std::shared_ptr<NetSessionNotify>& pSession, const sockaddr_storage& addr, int addrlen);

    //int32_t Listen();
    //!
    int32_t Close();
protected:
    void InitTcpSocket(evutil_socket_t fd, std::shared_ptr<NetSessionNotify>& pSession);
    //! 
    int32_t Connect(std::shared_ptr<NetSessionNotify>& pSession, const sockaddr_storage& addr, int addrlen);

protected:
    void OnReadEvent();
    void OnWriteEvent();

    //! 断开消息
    void OnConnect();
    void OnDisconnect(uint32_t dwNetCode);
    void OnSendData(uint32_t dwIoSize);
    uint32_t OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData);
    void OnError(uint32_t dwNetCode, int32_t lRetCode);

    bool ReadBuffer(int16_t lSend);
    void SendDataFromQueue();

    //! 
    bool CanClose();

    bool OnClose(bool bRemote = false);

    friend void OnLinkRead(evutil_socket_t fd, short event, void *arg);
    friend void OnLinkWrite(evutil_socket_t fd, short event, void *arg);
protected:
    NetThread*                              m_pThread;
    std::shared_ptr<NetSessionNotify>	    m_pNotify;
    evutil_socket_t					        m_socketfd = INVALID_SOCKET;
    event							        m_revent;
    event							        m_wevent;
    IPCQueue<SocketSendBuf>                 m_vtSocketBuf;
    char                                    m_szReadySendBuffer[READBUFFERSIZE_MSG];
    int16_t                                 m_nReadySendBufferLength = 0;
};
/*
//inner netthread data & multithreaddata
class ITCPSocket {
public:
	

protected:

    
protected:
    ///////////////////////////////////////////////////////////////////
    //netthread func
    //!
    virtual void InitMember();

    //!
    void CloseCallback(bool bRemote, uint32_t dwNetCode = 0);





public:





	//!
    virtual void Shutdown();

    //!
    virtual CBasicSessionNet* GetRealSessionNet() = 0;
public:

    


    //!
    void SetToSafeDelete();

protected:


	

    //!
    virtual bool CanClose();


};
*/
void OnLinkRead(evutil_socket_t fd, short event, void *arg);
void OnLinkWrite(evutil_socket_t fd, short event, void *arg);

__NS_ZILLIZ_IPC_END
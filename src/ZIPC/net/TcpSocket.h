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
#define TIL_SS_IDLE						0x00		//
#define TIL_SS_CONNECTING				0x01		//
#define TIL_SS_CONNECTED				0x02		//
#define TIL_SS_LISTENING				0x03		//

#define TIL_SS_SHAKEHANDLE_IDLE		    0x00		//
#define TIL_SS_SHAKEHANDLE_TRANSMIT		0x01		//
//////////////////////////////////////////////////////////////////////////
struct SocketSendBuf {
    std::shared_ptr<char>               m_pBuffer;
    int32_t                             m_nLength = 0;
    int32_t                             m_nReadLength = 0;
    SocketSendBuf(const char* pBuf, int32_t nLength) {
        m_pBuffer = std::make_shared<char>(new char[nLength]);
        memcpy(m_pBuffer.get(), pBuf, nLength);
        m_nLength = nLength;
    }
    SocketSendBuf(const std::shared_ptr<char> pBuf, int32_t nLength) {
        m_pBuffer = pBuf;
        m_nLength = nLength;
    }
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
    //better way to set the volatile
    uint8_t                                 m_statTransmit = TIL_SS_SHAKEHANDLE_IDLE;
    uint8_t                                 m_statLink = TIL_SS_IDLE;
    bool                                    m_bDeath = false;
public:
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //inner netthread call
    //! need init socket
    void InitTcpSocket(std::shared_ptr<NetSessionNotify>& pSession);

    //! 
    int32_t Connect(const sockaddr_storage& addr, int addrlen);

    //int32_t Listen();
    //!
    int32_t Close();
protected:
    
    

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
/***********************************************************************************************
// filename : 	tcpserver.h
// describ:		tcp net
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "TcpSessionNotify.h"
#ifdef _MSC_VER
#include <winsock2.h>    
#include <windows.h>
#else
#include <sys/socket.h>
#endif

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TcpThreadAccept;
class TcpThreadSocketSession;
class TcpServer;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BASIC_NET_ALREADY_LISTEN				(NET_ERROR | 0x0001)			//
#define BASIC_NET_NO_CREATESESSIONFUNC  		(NET_ERROR | 0x0002)			//


///////////////////////////////////////////////////////////////////////////////////////////////
class TcpServerSession : public NetSessionNotify {
public:
    virtual ~TcpServerSession();

    //!
    static std::shared_ptr<TcpServerSession> CreateTcpServerSession() {
        return std::shared_ptr<TcpServerSession>(new TcpServerSession());
    }

    //! unique in one server
    uint32_t GetSessionID() {
        return m_nSessionID;
    }
protected:
    TcpServerSession();
    
    virtual uint32_t OnDisconnect(uint32_t dwNetCode);

    //! 
    void InitTcpServerSession(uint32_t nSessionID, const std::shared_ptr<TcpServer>& pServer);

    //!
    virtual TcpThreadSocket* GetThreadSocket() override;
protected:
    std::shared_ptr<TcpServer>      m_pServer;
    uint32_t                        m_nSessionID;
    TcpThreadSocketSession*         m_pSocket;

    friend class TcpServer;
    friend class TcpThreadAccept;
};

class TcpServer : public NetBaseObject {
public:
    typedef void(*FuncCreateSession)(std::shared_ptr<TcpServerSession>&);
public:
    virtual ~TcpServer();

    static std::shared_ptr<TcpServer> CreateTcpServer() {
        return std::shared_ptr<TcpServer>(new TcpServer());
    }
    //! must be call first [IPv6Address]:port || IPv4Address:port
    int32_t InitTcpServer(const char* lpszAddress, const FuncCreateSession func);

    //! 
    bool IsListen();

    //! 
    int32_t DoListen();

    //!
    void Close();

    //! shutdown
    void Shutdown();

    //!
    std::shared_ptr<TcpServerSession> GetSessionBySessionID(uint32_t nSessionID);
protected:
    TcpServer();

    void ConstructSession(std::shared_ptr<TcpServerSession>& pSession);

    void AddSessionMap(const std::shared_ptr<TcpServerSession>& pSession);
    void DelSessionMap(uint32_t nSessionID);
protected:
    TcpThreadAccept*	                                            m_pSocket = nullptr;
    FuncCreateSession                                               m_createSessionFunc;

    uint32_t                                                        m_nClientSessionMgr = 0;
    bool                                                            m_bAddrSuccess = false;
    sockaddr_storage                                                m_addr;
    int                                                             m_addrlen = sizeof(sockaddr_storage);

    std::mutex                                                      m_mtxSession;
    IPCMap<uint32_t, std::shared_ptr<TcpServerSession>>             m_mapClientSession;

    friend class TcpThreadAccept;
    friend class TcpServerSession;
};

__NS_ZILLIZ_IPC_END

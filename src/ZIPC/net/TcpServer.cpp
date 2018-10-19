#include "ZIPC/net/TcpServer.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "TcpThreadAccept.h"
#include "TcpThreadSocketSession.h"
#include "ZIPC/base/log/IPCLog.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TcpServerSession::TcpServerSession() {
}

TcpServerSession::~TcpServerSession() {
    if (m_pSocket) {
        delete m_pSocket;
    }
}

void TcpServerSession::InitTcpServerSession(uint32_t nSessionID, const std::shared_ptr<TcpServer>& pServer) {
    m_nSessionID = nSessionID;
    m_pServer = pServer;
    m_pSocket = new TcpThreadSocketSession(m_pNetThread);
    auto pServerSession = std::static_pointer_cast<NetSessionNotify>(shared_from_this());
    m_pSocket->InitTcpSocket(pServerSession);
}

uint32_t TcpServerSession::OnDisconnect(uint32_t dwNetCode) {
    auto retValue = NetSessionNotify::OnDisconnect(dwNetCode);
    m_pServer->DelSessionMap(GetSessionID());

    //self thread shutdown
    m_pServer.reset();

    return retValue;
}
TcpThreadSocket * TcpServerSession::GetThreadSocket(){
    return m_pSocket;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TcpServer::TcpServer() {
}
TcpServer::~TcpServer() {
    if (m_pSocket) {
        LogFuncLocation(IPCLog_Error, "TcpServer::~TcpServer m_pSocket exist£¬ must be call shutdown");
    }
}

bool TcpServer::IsListen() {
    return m_pSocket->IsListen();
}

int32_t TcpServer::InitTcpServer(const char* lpszAddress, const std::function<void(std::shared_ptr<TcpServerSession>&)>& func) {
    if (func == nullptr)
        return BASIC_NET_NO_CREATESESSIONFUNC;
    if (lpszAddress == nullptr || lpszAddress[0] == '\0') {
        return BASIC_NET_INVALID_ADDRESS;
    }
    if (evutil_parse_sockaddr_port(lpszAddress, (sockaddr*)&m_addr, &m_addrlen) != 0) {
        return BASIC_NET_INVALID_ADDRESS;
    }
    m_bAddrSuccess = true;
    m_pSocket = new TcpThreadAccept(m_pNetThread);
    auto pServer = std::static_pointer_cast<TcpServer>(shared_from_this());
    m_pSocket->InitTcpSocket(pServer);
    m_createSessionFunc = func;
    return BASIC_NET_OK;
}

//! 
int32_t TcpServer::DoListen() {
    if (m_createSessionFunc == nullptr)
        return BASIC_NET_NO_CREATESESSIONFUNC;
    if (IsListen())
        return BASIC_NET_ALREADY_LISTEN;
    int32_t lReturn = BASIC_NET_OK;
    evutil_socket_t socketfd = INVALID_SOCKET;
    do {
        socketfd = socket(m_addr.ss_family, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET) {
            lReturn = BASIC_NET_SOCKET_ERROR;
            break;
        }
        evutil_make_socket_nonblocking(socketfd);
        evutil_make_listen_socket_reuseable(socketfd);
        evutil_make_listen_socket_reuseable_port(socketfd);
        // bind our name to the socket
        int nRet = ::bind(socketfd, (::sockaddr*)&m_addr, m_addrlen);   //I know this
        if (nRet != 0) {
            lReturn = BASIC_NET_BIND_ERROR;
            break;
        }
        // Set the socket to listen
        nRet = listen(socketfd, 0x7fffffff);
        if (nRet != 0) {
            lReturn = BASIC_NET_LISTEN_ERROR;
            break;
        }
    } while (0);
    if (lReturn == BASIC_NET_OK) {
        m_pNetThread->SetEvent(shared_from_this(), [this, socketfd]() {
            m_pSocket->Listen(socketfd);
        });
    }
    else if (socketfd != INVALID_SOCKET) {
        evutil_closesocket(socketfd);
    }
    return lReturn;
}

//! 
void TcpServer::Close() {
    m_pNetThread->SetEvent(shared_from_this(), [this]() {
        m_pSocket->Close();
    });
}

//! shutdown
void TcpServer::Shutdown() {
    m_pNetThread->SetEvent(shared_from_this(), [this]() {
        if (m_pSocket) {
            m_pSocket->Close();
            delete m_pSocket;
            m_pSocket = nullptr;
        }
    });
}

//!
bool TcpServer::IsShutdown() {
    return m_pSocket == nullptr;
}

void TcpServer::AddSessionMap(const std::shared_ptr<TcpServerSession>& pSession) {
    std::lock_guard<std::mutex> lock(m_mtxSession);
    m_mapClientSession[pSession->GetSessionID()] = pSession;
}

void TcpServer::DelSessionMap(uint32_t nSessionID) {
    {
        std::lock_guard<std::mutex> lock(m_mtxSession);
        m_mapClientSession.erase(nSessionID);
    }
    //reuse nSessionID

}

//!
std::shared_ptr<TcpServerSession> TcpServer::GetSessionBySessionID(uint32_t nSessionID) {
    std::lock_guard<std::mutex> lock(m_mtxSession);
    auto iter = m_mapClientSession.find(nSessionID);
    if (iter != m_mapClientSession.end()) {
        return iter->second;
    }
    return nullptr;
}

void TcpServer::ConstructSession(std::shared_ptr<TcpServerSession>& pSession) {
    m_createSessionFunc(pSession);
}

__NS_ZILLIZ_IPC_END

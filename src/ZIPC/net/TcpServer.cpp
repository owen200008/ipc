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
    return m_pSocket ? m_pSocket->GetSocketID() != INVALID_SOCKET : false;
}

int32_t TcpServer::InitTcpServer(const char* lpszAddress, const FuncCreateSession func) {
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
    std::promise<int32_t> promiseObj;
    auto future = promiseObj.get_future();

    m_pNetThread->SetEvent(shared_from_this(), [&promiseObj, this]() {
        promiseObj.set_value(m_pSocket->Listen(m_addr, m_addrlen));
    });
    return future.get();
}

//! 
void TcpServer::Close() {
    m_pNetThread->SetEvent(shared_from_this(), [this]() {
        m_pSocket->Close();
    });
}

//! shutdown
void TcpServer::Shutdown() {
    std::promise<int32_t> promiseObj;
    auto future = promiseObj.get_future();
    m_pNetThread->SetEvent(shared_from_this(), [this, &promiseObj]() {
        if (m_pSocket) {
            m_pSocket->Close();
            delete m_pSocket;
            m_pSocket = nullptr;
        }
        promiseObj.set_value(BASIC_NET_OK);
    });
    future.get();
}

void TcpServer::AddSessionMap(const std::shared_ptr<TcpServerSession>& pSession) {
    std::lock_guard<std::mutex> lock(m_mtxSession);
    m_mapClientSession[pSession->GetSessionID()] = pSession;
}

void TcpServer::DelSessionMap(uint32_t nSessionID) {
    std::lock_guard<std::mutex> lock(m_mtxSession);
    m_mapClientSession.erase(nSessionID);
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
    pSession->InitTcpServerSession(m_nClientSessionMgr++, std::static_pointer_cast<TcpServer>(shared_from_this()));
}

__NS_ZILLIZ_IPC_END

#include "TcpSocket.h"
#include "NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include <assert.h>

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpSocket::TcpSocket(NetThread* pThread) {
    m_pThread = pThread;
}

TcpSocket::~TcpSocket() {
#ifdef _DEBUG
    if (m_socketfd != INVALID_SOCKET) {
        IPCLog::GetInstance().Log(IPCLog_Error, "ITCPSocket::~ITCPSocket m_socketfd != INVALID_SOCKET");
    }
#endif
}

bool TcpSocket::IsConnected() {
    return m_statLink == TIL_SS_CONNECTED;
}

bool TcpSocket::IsTransmit() {
    return m_statTransmit == TIL_SS_SHAKEHANDLE_TRANSMIT;
}

//! 
bool TcpSocket::CanClose() {
    if (m_statLink != TIL_SS_CONNECTING) {
        if (m_vtSocketBuf.size() == 0 && m_nReadySendBufferLength == 0)
            return true;
        return !IsConnected();
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////
void TcpSocket::InitTcpSocket(std::shared_ptr<NetSessionNotify>& pSession) {
    m_pNotify = pSession;
    pSession->InitSocket(this);
}

int32_t TcpSocket::Connect(const sockaddr_storage& addr, int addrlen) {
    uint8_t statLink = m_statLink;
    if (statLink != TIL_SS_IDLE) {
        int32_t lRet = BASIC_NET_GENERIC_ERROR;
        switch (statLink) {
        case TIL_SS_CONNECTING: {
            lRet = BASIC_NET_CONNECTING_ERROR;
            break;
        }
        case TIL_SS_CONNECTED: {
            lRet = BASIC_NET_ALREADY_CONNECT;
            break;
        }
        }
        return lRet;
    }
    int32_t lReturn = BASIC_NET_OK;
    evutil_socket_t socketfd = INVALID_SOCKET;
    do {
        evutil_socket_t socketfd = socket(addr.ss_family, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET) {
            lReturn = BASIC_NET_SOCKET_ERROR;
            break;
        }
        evutil_make_socket_nonblocking(socketfd);
        evutil_make_listen_socket_reuseable(socketfd);
        evutil_make_listen_socket_reuseable_port(socketfd);
        int nRet = connect(socketfd, (::sockaddr*)&addr, addrlen);
        if (nRet == 0) {
            m_socketfd = socketfd;
            //event
            event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
            event_base_set(m_pThread->m_base, &m_revent);
            event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
            event_base_set(m_pThread->m_base, &m_wevent);

            event_add(&m_revent, NULL);

            OnConnect();
        }
#ifdef _MSC_VER
        else if (errno == EINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (errno == EINPROGRESS) {
#endif
            m_socketfd = socketfd;
            //wait for write onconnect
            m_statLink = TIL_SS_CONNECTING;
            event_set(&m_revent, socketfd, EV_READ | EV_PERSIST, OnLinkRead, this);
            event_base_set(m_pThread->m_base, &m_revent);
            event_set(&m_wevent, socketfd, EV_WRITE, OnLinkWrite, this);
            event_base_set(m_pThread->m_base, &m_wevent);

            event_add(&m_wevent, NULL);
        }
        else {
            //int nRetErrorNo = errno;
            lReturn = BASIC_NET_GENERIC_ERROR;
        }
    } while (0);

    if (lReturn != BASIC_NET_OK && socketfd != INVALID_SOCKET) {
        evutil_closesocket(socketfd);
        m_socketfd = INVALID_SOCKET;
    }
    
    return BASIC_NET_OK;
}

//!
int32_t TcpSocket::Close() {
    if (m_socketfd == INVALID_SOCKET)
        return BASIC_NET_OK;
    if (!OnClose()) {
        return BASIC_NET_CLOSE_WAITCLOSE;
    }
    return BASIC_NET_OK;
}

///////////////////////////////////////////////////////////////////////////////
void TcpSocket::OnReadEvent() {
    char szBuf[READBUFFERSIZE_MSG];
    int nReceived = 0;
    do {
        while ((nReceived = recv(m_socketfd, szBuf, READBUFFERSIZE_MSG, 0)) > 0) {
            OnReceive(BASIC_NETCODE_SUCC, szBuf, nReceived);
        }
        if (nReceived == 0) {
            OnClose(true);
            break;
        }
        else if (nReceived < 0) {
            int nNumber = errno;
            if (nNumber == EAGAIN || nNumber == EINTR || errno == EWOULDBLOCK) {
                break;
            }
            bool bError = false;
#ifdef _MSC_VER
            if (nReceived == SOCKET_ERROR) {
                DWORD dwLastError = WSAGetLastError();   //I know this
                if (dwLastError == WSAEWOULDBLOCK) {
                    break;
                }
                else if (dwLastError != WSA_IO_PENDING) {
                    bError = true;
                }
            }
#else
            bError = true;
#endif
            if (bError) {
                OnClose(true);
                break;
            }
        }
    } while (0);
}


void TcpSocket::OnWriteEvent() {
    if (m_statLink == TIL_SS_CONNECTING) {
        int nErr = 0;
        socklen_t nLen = sizeof(nErr);
        getsockopt(m_socketfd, SOL_SOCKET, SO_ERROR, (char *)&nErr, &nLen);
        if (nErr == 0) {
            event_add(&m_revent, NULL);
            OnConnect();
        }
        else {
            OnClose(true);
        }
    }
    else {
        SendDataFromQueue();
    }
}

void TcpSocket::OnConnect() {
    uint32_t dwNetCode = BASIC_NETCODE_SUCC;
    m_statLink = TIL_SS_CONNECTED;
    int32_t lRet = m_pNotify->OnConnect(dwNetCode);

    //
    if (lRet == BASIC_NET_OK) {
        m_statTransmit = TIL_SS_SHAKEHANDLE_TRANSMIT;
    }
    else if (lRet == BASIC_NET_GENERIC_ERROR) {
        OnClose();
    }
}

uint32_t TcpSocket::OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData) {
    int32_t lRet = m_pNotify->OnReceive(dwNetCode, pszData, cbData);
    //
    if ((lRet & BASIC_NET_HR_RET_HANDSHAKE) && !(lRet & NET_ERROR)) {
        m_statTransmit = TIL_SS_SHAKEHANDLE_TRANSMIT;
    }
    return lRet;
}

bool TcpSocket::ReadBuffer(int16_t lSend) {
    int16_t nLeft = m_nReadySendBufferLength - lSend;
    if (nLeft > 0){
        if (lSend == 0)
            return true;
        memmove(m_szReadySendBuffer, &m_szReadySendBuffer[lSend], nLeft);
        m_nReadySendBufferLength = nLeft;
    }
    else{
        int16_t nRevert = READBUFFERSIZE_MSG;
        while (m_vtSocketBuf.size() > 0 && nRevert > 0) {
            auto& data = m_vtSocketBuf.front();
            uint32_t nDataLength = data.m_nLength - data.m_nReadLength;
            if (nDataLength > nRevert) {
                data.m_nReadLength += nRevert;
                nRevert = 0;
                memcpy(m_szReadySendBuffer, &data.m_pBuffer.get()[data.m_nReadLength], nRevert);
                break;
            }
            else if (nDataLength == nRevert) {
                nRevert = 0;
                memcpy(m_szReadySendBuffer, &data.m_pBuffer.get()[data.m_nReadLength], nRevert);
                m_vtSocketBuf.pop();
                break;
            }
            else {
                nRevert -= nDataLength;
                m_vtSocketBuf.pop();
            }
        }
        m_nReadySendBufferLength = READBUFFERSIZE_MSG - nRevert;
    }
    return m_nReadySendBufferLength > 0;
}

void TcpSocket::SendDataFromQueue() {
    if (!IsConnected()) {
        return;
    }

    bool bError = false;
    int nTotalSend = 0;
    int16_t lSend = 0;
    while (ReadBuffer(lSend)) {
        lSend = send(m_socketfd, m_szReadySendBuffer, m_nReadySendBufferLength, 0);
        if (lSend >= 0) {
            nTotalSend += lSend;
        }
        else {
            int nNumber = errno;
            if (nNumber == EAGAIN) {
                event_add(&m_wevent, NULL);
                break;
            }
            else if (nNumber == EINTR) {
                lSend = 0;
                continue;
            }
            else {
#ifdef _MSC_VER
                if (lSend == SOCKET_ERROR) {
                    DWORD dwLastError = WSAGetLastError();   //I know this
                    if (dwLastError == WSAEWOULDBLOCK) {
                        event_add(&m_wevent, NULL);
                    }
                    else if (dwLastError != WSA_IO_PENDING) {
                        bError = true;
                    }
                }
#else
                bError = true;
#endif
                break;
            }
        }
    }
    if (bError) {
        OnClose(true);
    }
    else {
        OnSendData(nTotalSend);
    }
}

bool TcpSocket::OnClose(bool bRemote) {
    if (m_socketfd == INVALID_SOCKET) {
        return true;
    }
    if (CanClose() || bRemote) {
        event_del(&m_revent);
        event_del(&m_wevent);

        evutil_closesocket(m_socketfd);
        m_socketfd = INVALID_SOCKET;
        m_statTransmit = TIL_SS_SHAKEHANDLE_IDLE;
        m_statLink = TIL_SS_IDLE;
        IPCQueue<SocketSendBuf> queuetmp;
        swap(queuetmp, m_vtSocketBuf);
        m_nReadySendBufferLength = 0;

        OnDisconnect(bRemote ? BASIC_NETCODE_CLOSE_REMOTE : 0);
        return true;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////
/*
void ITCPSocket::InitMember(){
m_socketfd = INVALID_SOCKET;
m_unSessionStatus &= TIL_RESET_MASK;
}

void ITCPSocket::CloseCallback(bool bRemote, uint32_t dwNetCode){
if(m_socketfd != INVALID_SOCKET){
if(bRemote){
dwNetCode |= BASIC_NETCODE_CLOSE_REMOTE;
}
event_del(&m_revent);
event_del(&m_wevent);

evutil_closesocket(m_socketfd);
InitMember();
OnDisconnect(dwNetCode);
}
}

void ITCPSocket::OnDisconnect(uint32_t dwNetCode) {
//+1 ref makesure callback will not free the object NetSessionNotify
std::shared_ptr<NetSessionNotify> pNotify = m_pNotify;
if (pNotify)
pNotify->OnDisconnect(dwNetCode);
}
void ITCPSocket::OnSendData(uint32_t dwIoSize) {
if (IsToClose() || !IsConnected())
return;
//+1 ref makesure callback will not free the object NetSessionNotify
std::shared_ptr<NetSessionNotify> pNotify = m_pNotify;
if(pNotify)
pNotify->OnSendData(dwIoSize);
}

void ITCPSocket::OnError(uint32_t dwNetCode, int32_t lRetCode) {
//+1 ref makesure callback will not free the object NetSessionNotify
std::shared_ptr<NetSessionNotify> pNotify = m_pNotify;
if (pNotify) {
pNotify->OnError(dwNetCode, lRetCode);
}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void ITCPSocket::Shutdown(){
SetToSafeDelete();
SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
pSession->CloseCallback(false);
m_gNetMgrPoint->DelToTimer(pSession);
});
}

bool ITCPSocket::CanClose(){
return GetSessionStatus(TIL_SS_LINK) != TIL_SS_CONNECTING;
}


int32_t ITCPSocket::SendData(SendBufferCacheMgr& sendData){
SendBufferCache* pCache = sendData.SwapCache();
SetLibEvent([](CBasicNet_Socket* pSession, intptr_t lRevert)->void{
CBasicNet_SocketTransfer* pClientSession = (CBasicNet_SocketTransfer*)pSession;
pClientSession->AddSendQueue((SendBufferCache*)lRevert);
}, (intptr_t)pCache);
return pCache->m_cbData;
}


bool ITCPSocket::IsRecTimeout(time_t tmNow, uint16_t nTimeoutSecond){
if(tmNow - m_stNet.m_tmLastRecTime >= nTimeoutSecond && nTimeoutSecond != 0)
return true;
return false;
}


int32_t ITCPSocket::Send(void *pData, int32_t cbData, uint32_t dwFlag){
if(IsToClose()){
return BASIC_NET_TOCLOSE_ERROR;
}
if(!IsConnected()){
return BASIC_NET_NO_CONNECT;
}
SendBufferCacheMgr cacheMgr;
cacheMgr.Reset((const char*)pData, cbData);
return SendData(cacheMgr);
}
/////////////////////////////////////////////////////////////////////////////////////
void ITCPSocket::SetToSafeDelete(){
SetSessionStatus(TIL_SS_TOSAFEDELETE, TIL_SS_RELEASE_MASK);
}
/////////////////////////////////////////////////////////////////////////////////////

void ITCPSocket::OnIdle(){
m_unIdleCount++;
if(GetSessionStatus(TIL_SS_SHAKEHANDLE_MASK) == TIL_SS_SHAKEHANDLE_TRANSMIT){
if(nullptr != m_pSession)
m_pSession->OnIdle(m_unIdleCount);
}
else if(m_unIdleCount > m_usTimeoutShakeHandle){
Close();
return;
}
}



void ITCPSocket::OnReceiveData(const char* pszData, uint32_t dwIoSize){
int nReceived = (int)dwIoSize;
int32_t lRet = 0;
m_stNet.OnReceiveData(nReceived);
OnReceive(BASIC_NETCODE_SUCC, pszData, nReceived);
}

void CBasicNet_SocketTransfer::AddSendQueue(SendBufferCache* pSendCache){
m_msgQueue.AppendDataEx(pSendCache->m_cRevertData, pSendCache->m_cbData);
SendBufferCache::ReleaseCache(pSendCache);
SendDataFromQueue();
}

*/


///////////////////////////////////////////////////////////////////////////////
void OnLinkRead(evutil_socket_t fd, short event, void *arg){
    ((TcpSocket*)arg)->OnReadEvent();
}

void OnLinkWrite(evutil_socket_t fd, short event, void *arg){
    ((TcpSocket*)arg)->OnWriteEvent();
}

//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



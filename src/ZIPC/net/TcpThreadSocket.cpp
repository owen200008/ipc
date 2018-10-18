#include "TcpThreadSocket.h"
#include "ZIPC/net/TcpDefine.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/net/TcpServer.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////
TcpThreadSocket::TcpThreadSocket(NetThread* pThread) {
    m_pThread = pThread;
}

TcpThreadSocket::~TcpThreadSocket() {
    if (m_socketfd != INVALID_SOCKET) {
        LogFuncLocation(IPCLog_Error, "ITCPSocket::~ITCPSocket m_socketfd != INVALID_SOCKET");
        //try to clear event
        event_del(&m_revent);
        event_del(&m_wevent);

        evutil_closesocket(m_socketfd);
    }
}

bool TcpThreadSocket::IsConnected() {
    return m_statLink == TIL_SS_CONNECTED;
}

bool TcpThreadSocket::IsTransmit() {
    return m_statTransmit == TIL_SS_SHAKEHANDLE_TRANSMIT;
}

//! 
bool TcpThreadSocket::CanClose() {
    if (m_statLink != TIL_SS_CONNECTING) {
        if (m_qSocketBuf.size() == 0 && m_nReadySendBufferLength == 0)
            return true;
        return !IsConnected();
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////
//!
void TcpThreadSocket::Send(const SocketSendBuf& buf) {
    if (IsConnected()) {
        m_qSocketBuf.push(std::move(buf));
        SendDataFromQueue();
    }
}

//!
int32_t TcpThreadSocket::Close() {
    if (m_socketfd == INVALID_SOCKET)
        return BASIC_NET_OK;
    if (!OnClose()) {
        return BASIC_NET_CLOSE_WAITCLOSE;
    }
    return BASIC_NET_OK;
}

//!
bool TcpThreadSocket::Shutdown() {
    if (m_socketfd != INVALID_SOCKET) {
        LogFuncLocation(IPCLog_Error, "ITCPSocket::Shutdown m_socketfd != INVALID_SOCKET");
        return false;
    }
    //ref del
    m_pNotify = nullptr;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void TcpThreadSocket::ClearMemberData() {
    event_del(&m_revent);
    event_del(&m_wevent);

    evutil_closesocket(m_socketfd);
    m_socketfd = INVALID_SOCKET;
    m_statTransmit = TIL_SS_SHAKEHANDLE_IDLE;
    m_statLink = TIL_SS_IDLE;
    IPCQueue<SocketSendBuf> queuetmp;
    swap(queuetmp, m_qSocketBuf);
    m_nReadySendBufferLength = 0;
}
///////////////////////////////////////////////////////////////////////////////
void TcpThreadSocket::OnConnect() {
    m_statLink = TIL_SS_CONNECTED;
    int32_t lRet = m_pNotify->OnConnect();

    //
    if (lRet == BASIC_NET_OK) {
        m_statTransmit = TIL_SS_SHAKEHANDLE_TRANSMIT;
    }
    else if (lRet == BASIC_NET_GENERIC_ERROR) {
        OnClose();
    }
}

void TcpThreadSocket::OnDisconnect(uint32_t dwNetCode){
    m_pNotify->OnDisconnect(dwNetCode);
}

uint32_t TcpThreadSocket::OnReceive(const char *pszData, int32_t cbData) {
    int32_t lRet = m_pNotify->OnReceive(pszData, cbData);
    //
    if ((lRet & BASIC_NET_HR_RET_HANDSHAKE) && !(lRet & NET_ERROR)) {
        m_statTransmit = TIL_SS_SHAKEHANDLE_TRANSMIT;
    }
    return lRet;
}

void TcpThreadSocket::OnSendData(uint32_t dwIoSize) {
    if (!IsConnected())
        return;
    m_pNotify->OnSendData(dwIoSize);  
}

bool TcpThreadSocket::ReadBuffer(int16_t lSend) {
    int16_t nLeft = m_nReadySendBufferLength - lSend;
    if (nLeft > 0){
        if (lSend == 0)
            return true;
        memmove(m_szReadySendBuffer, &m_szReadySendBuffer[lSend], nLeft);
        m_nReadySendBufferLength = nLeft;
    }
    else{
        int16_t nRevert = READBUFFERSIZE_MSG;
        while (m_qSocketBuf.size() > 0 && nRevert > 0) {
            auto& data = m_qSocketBuf.front();
            int32_t nDataLength = data.m_nLength - data.m_nReadLength;
            if (nDataLength > nRevert) {
                memcpy(m_szReadySendBuffer, &data.m_pBuffer.get()[data.m_nReadLength], nRevert);
                data.m_nReadLength += nRevert;
                nRevert = 0;
                break;
            }
            else if (nDataLength == nRevert) {
                memcpy(m_szReadySendBuffer, &data.m_pBuffer.get()[data.m_nReadLength], nRevert);
                m_qSocketBuf.pop();
                nRevert = 0;
                break;
            }
            else {
                memcpy(m_szReadySendBuffer, &data.m_pBuffer.get()[data.m_nReadLength], nDataLength);
                nRevert -= nDataLength;
                m_qSocketBuf.pop();
            }
        }
        m_nReadySendBufferLength = READBUFFERSIZE_MSG - nRevert;
    }
    return m_nReadySendBufferLength > 0;
}

void TcpThreadSocket::SendDataFromQueue() {
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

bool TcpThreadSocket::OnClose(bool bRemote) {
    if (m_socketfd == INVALID_SOCKET) {
        return true;
    }
    if (CanClose() || bRemote) {
        ClearMemberData();

        OnDisconnect(bRemote ? BASIC_NETCODE_CLOSE_REMOTE : 0);
        return true;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////
/*

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
*/
///////////////////////////////////////////////////////////////////////////////
void TcpThreadSocket::OnReadEvent() {
    char szBuf[READBUFFERSIZE_MSG];
    int nReceived = 0;
    do {
        while ((nReceived = recv(m_socketfd, szBuf, READBUFFERSIZE_MSG, 0)) > 0) {
            OnReceive(szBuf, nReceived);
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


void TcpThreadSocket::OnWriteEvent() {
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
///////////////////////////////////////////////////////////////////////////////
void OnLinkRead(evutil_socket_t fd, short event, void *arg){
    ((TcpThreadSocket*)arg)->OnReadEvent();
}

void OnLinkWrite(evutil_socket_t fd, short event, void *arg){
    ((TcpThreadSocket*)arg)->OnWriteEvent();
}


//////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END



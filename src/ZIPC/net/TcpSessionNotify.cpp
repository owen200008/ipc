#include "ZIPC/net/TcpSessionNotify.h"
#include "ZIPC/base/sys/SysInfo.h"
#include "NetMgr.h"
#include "NetThread.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BasicNetStat::BasicNetStat(){
    Empty();
}
void BasicNetStat::Empty(){
    memset(this, 0, sizeof(BasicNetStat));
}
void BasicNetStat::OnSendData(int nSend){
    if(nSend > 0){
        m_dwSendBytes += nSend;
        m_dwSendTimes++;
    }
}
void BasicNetStat::OnReceiveData(int nRece){
    if(nRece > 0){
        m_dwReceBytes += nRece;
        m_dwReceTimes++;
    }
    m_tmLastRecTime = time(nullptr);
}
void BasicNetStat::GetTransRate(BasicNetStat& lastData, double& dSend, double& dRecv){
    auto tNow = IPCGetTickTime();
    if(m_tLastStat > 0 && (tNow - m_tLastStat)  < 10000){
        dSend = m_fLastSendRate;
        dRecv = m_fLastRecvRate;
        return;
    }

    if(m_tLastStat == 0){
        m_tLastStat = tNow;
        lastData = *this;
        return;
    }

    auto dwTotalBytes0 = lastData.m_dwReceBytes;
    dwTotalBytes0 += lastData.m_dwSendBytes;
    auto dwTotalBytes1 = m_dwReceBytes;
    dwTotalBytes1 += m_dwSendBytes;
    m_fLastSendRate = double(m_dwSendBytes - lastData.m_dwSendBytes) / 1024 / (double(tNow - m_tLastStat) / 1000);
    m_fLastRecvRate = double(m_dwReceBytes - lastData.m_dwReceBytes) / 1024 / (double(tNow - m_tLastStat) / 1000);

    dSend = m_fLastSendRate;
    dRecv = m_fLastRecvRate;

    m_tLastStat = tNow;
    lastData = *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
NetSessionNotify::NetSessionNotify(){
    m_pNetThread = NetMgr::GetInstance().AssignNetThread();
}

NetSessionNotify::~NetSessionNotify(){
    if (m_pSocket) {
        //m_pNetThread->Re
    }
}

void NetSessionNotify::GetNetStatInfo(BasicNetStat& netState) {
    netState = m_stNet;
}

NetThread* NetSessionNotify::GetNetThread() {
    return m_pNetThread;
}
TcpSocket* NetSessionNotify::GetTcpSocket() {
    return m_pSocket;
}

bool NetSessionNotify::IsConnected() {
    return m_pSocket ? m_pSocket->IsConnected() : false;
}
//!
bool NetSessionNotify::IsTransmit() {
    return m_pSocket ? m_pSocket->IsTransmit() : false;
}


bool NetSessionNotify::Close(){
    if (!m_pSocket) {
        return true;
    }
    std::promise<int32_t> promiseObj;
    auto future = promiseObj.get_future();
    
    GotoNetThread([](std::shared_ptr<NetSessionNotify>& pSession, TcpSocket* pSocket, intptr_t lRevert) {
        std::promise<int32_t>* pRevert = (std::promise<int32_t>*)lRevert;
        pRevert->set_value(pSocket->Close());
    }, (intptr_t)&promiseObj);
    return future.get() == BASIC_NET_OK;
}

int32_t NetSessionNotify::Send(char *pData, int32_t cbData) {
    if (!m_pSocket) {
        return BASIC_NET_SOCKET_NOEXIST;
    }
    GotoNetThread([](std::shared_ptr<NetSessionNotify>& pSession, TcpSocket* pSocket, intptr_t lRevert) {
        pSocket->Send((SocketSendBuf*)lRevert);
    }, (intptr_t)SocketSendBuf::CreateSocketSendBuf(pData, cbData));
    return BASIC_NET_OK;
}
int32_t NetSessionNotify::Send(std::shared_ptr<char> pData, int32_t cbData) {
    GotoNetThread([](std::shared_ptr<NetSessionNotify>& pSession, TcpSocket* pSocket, intptr_t lRevert) {
        pSocket->Send((SocketSendBuf*)lRevert);
    }, (intptr_t)SocketSendBuf::CreateSocketSendBuf(pData, cbData));
    return BASIC_NET_OK;
}

//!
void NetSessionNotify::InitSocket(TcpSocket* pSocket) {
    m_pSocket = pSocket;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//! change to netthread
void NetSessionNotify::GotoNetThread(pNetThreadCallback pCallback, intptr_t lRevert) {
    NetThreadEvent setEvent(shared_from_this(), pCallback, lRevert);
    m_pNetThread->SetEvent(setEvent);
}

__NS_ZILLIZ_IPC_END

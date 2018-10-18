#include "ZIPC/net/TcpSessionNotify.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"
#include <chrono>
#include "TcpAfx.h"
#include "TcpThreadSocket.h"

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
    auto tNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
}

NetSessionNotify::~NetSessionNotify(){
}

void NetSessionNotify::GetNetStatInfo(BasicNetStat& netState) {
    netState = m_stNet;
}

bool NetSessionNotify::IsConnected() {
    return GetThreadSocket()->IsConnected();
}
//!
bool NetSessionNotify::IsTransmit() {
    return GetThreadSocket()->IsTransmit();
}

bool NetSessionNotify::Close(){
    //no check socketid inner thread check
    std::promise<int32_t> promiseObj;
    auto future = promiseObj.get_future();
    
    m_pNetThread->SetEvent(shared_from_this(), [&promiseObj, this](){
        promiseObj.set_value(GetThreadSocket()->Close());
    });
    return future.get() == BASIC_NET_OK;
}

int32_t NetSessionNotify::Send(const char *pData, int32_t cbData) {
    if (!IsConnected())
        return BASIC_NET_NO_CONNECT;

    SocketSendBuf sendBuf(pData, cbData);
    m_pNetThread->SetEvent(shared_from_this(), [this, sendBuf]() {
        GetThreadSocket()->Send(sendBuf);
    });
    return BASIC_NET_OK;
}
int32_t NetSessionNotify::Send(const std::shared_ptr<char>& pData, int32_t cbData) {
    if (!GetThreadSocket()->IsConnected())
        return BASIC_NET_NO_CONNECT;
    SocketSendBuf sendBuf(pData, cbData);
    m_pNetThread->SetEvent(shared_from_this(), [this, sendBuf]() {
        GetThreadSocket()->Send(sendBuf);
    });
    return BASIC_NET_OK;
}

__NS_ZILLIZ_IPC_END

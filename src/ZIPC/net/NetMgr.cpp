#include "NetMgr.h"
#include <signal.h>
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/conf/IPCConfRead.h"
#include "NetThread.h"

__NS_ZILLIZ_IPC_START
/////////////////////////////////////////////////////////////////////////////
NetMgr& NetMgr::GetInstance(){
	static NetMgr instance;
	return instance;
}
/////////////////////////////////////////////////////////////////////////////
NetMgr::NetMgr(){
    //create net
    Initialize();
}

NetMgr::~NetMgr(){
    //release net
    CloseNetSocket();
}

void NetMgr::Initialize(){
    IPCLog::GetInstance().Log(IPCLog_Info, "Start NetMgr Initialize");
#ifndef _MSC_VER
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
#endif

    struct event_config *cfg = event_config_new();
    evdns_set_log_fn([](int is_warn, const char *msg){
    	IPCLog::GetInstance().Log(is_warn ? IPCLog_Warn : IPCLog_Info, msg);
    });
    auto confRead = IPCConfRead::GetInstance();
    m_nEventThreadCount = atoi(confRead.GetConf("IPCNet", "NetThreadCount", "1"));
    if(m_nEventThreadCount <= 0)
        //default one thread
        m_nEventThreadCount = 1;
    m_pEventThreads = new NetThread[m_nEventThreadCount];

    //windows no multithread
    for(int i = 0; i < m_nEventThreadCount; i++){
        m_pEventThreads[i].Init(i, cfg);
    }
    //wait init all thread
    while(GetInitNetThreadCount() != m_nEventThreadCount){
        CCSwitchToThread();
    }

    //m_thread_ontimer_ptr = std::make_shared<std::thread>(&NetMgr::OnTimer, this);

    event_config_free(cfg);
    IPCLog::GetInstance().Log(IPCLog_Info, "End NetMgr Initialize");
}


void NetMgr::CloseNetSocket(){
    IPCLog::GetInstance().Log(IPCLog_Info, "Start NetMgr CLose");
    m_bTimeToKill = true;
    if(m_thread_ontimer_ptr != nullptr)
        m_thread_ontimer_ptr->join();

    if(m_nEventThreadCount > 0 && m_pEventThreads != nullptr){
        delete[] m_pEventThreads;
        m_pEventThreads = nullptr;
    }
    IPCLog::GetInstance().Log(IPCLog_Info, "End NetMgr CLose");
}
////////////////////////////////////////////////////////////////////////////////////////
NetThread* NetMgr::AssignNetThread() {
    uint16_t nValue = m_nNetThreadIndexGetCount.fetch_add(1, std::memory_order_acquire);
    return nValue < 100 ? &m_pEventThreads[m_nCurrentNetThreadIndex] : &m_pEventThreads[ReCalcNetThreadIndex()];
}

//! calc net threadindex
uint16_t NetMgr::ReCalcNetThreadIndex() {
    uint16_t nIndex = 0;
    uint32_t nMin = m_pEventThreads[0].GetWeight();
    for (uint16_t i = 1; i < m_nEventThreadCount;i++) {
        uint32_t nWeight = m_pEventThreads[i].GetWeight();
        if (nMin > nWeight) {
            nIndex = i;
            nMin = nWeight;
        }
    }
    m_nCurrentNetThreadIndex = nIndex;
    m_nNetThreadIndexGetCount.store(0, std::memory_order_release);
    return nIndex;
}

void NetMgr::IncrementNetThreadCount() {
    m_nInitThreadCount.fetch_add(1, std::memory_order_relaxed);
}
void NetMgr::DecrementNetThreadCount() {
    m_nInitThreadCount.fetch_sub(1, std::memory_order_relaxed);
}
uint16_t NetMgr::GetInitNetThreadCount() {
    return m_nInitThreadCount.load(std::memory_order_relaxed);
}
////////////////////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END
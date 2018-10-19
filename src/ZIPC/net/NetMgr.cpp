#include "ZIPC/net/NetMgr.h"
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
}

NetMgr::~NetMgr(){
}

void NetMgr::Initialize(){
    //has init
    if (m_pEventThreads)
        return;
    LogFuncInfo(IPCLog_Info, "Start NetMgr Initialize");
#ifdef _MSC_VER
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);

    (void)WSAStartup(wVersionRequested, &wsaData);
#else
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
#endif

    struct event_config *cfg = event_config_new();
    evdns_set_log_fn([](int is_warn, const char *msg){
        LogFuncInfo(is_warn ? IPCLog_Warn : IPCLog_Info, msg);
    });
    auto confRead = IPCConfRead::GetInstance();
    m_nEventThreadCount = atoi(confRead.GetConf("IPCNet", "NetThreadCount", "2"));
    if(m_nEventThreadCount <= 0)
        //default one thread
        m_nEventThreadCount = 1;
    m_pEventThreads = new NetThread[m_nEventThreadCount];

    //windows no multithread
    for(int i = 0; i < m_nEventThreadCount; i++){
        m_pEventThreads[i].Init(i, cfg);
    }

    //m_thread_ontimer_ptr = std::make_shared<std::thread>(&NetMgr::OnTimer, this);

    event_config_free(cfg);
    LogFuncInfo(IPCLog_Info, "End NetMgr Initialize");
}


void NetMgr::CloseNetSocket(){
    LogFuncInfo(IPCLog_Info, "Start NetMgr CLose");
    if(m_thread_ontimer_ptr != nullptr)
        m_thread_ontimer_ptr->join();

    if(m_nEventThreadCount > 0 && m_pEventThreads != nullptr){
        delete[] m_pEventThreads;
        m_pEventThreads = nullptr;
    }
#ifdef _MSC_VER
    WSACleanup();
#endif
    LogFuncInfo(IPCLog_Info, "End NetMgr CLose");
}
////////////////////////////////////////////////////////////////////////////////////////
NetThread* NetMgr::AssignNetThread() {
    uint16_t nValue = m_nNetThreadIndexGetCount.fetch_add(1);
    return &m_pEventThreads[nValue % m_nEventThreadCount];
}
////////////////////////////////////////////////////////////////////////////////////////
__NS_ZILLIZ_IPC_END
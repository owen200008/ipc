/***********************************************************************************************
// filename : 	tcpnotify.h
// describ:		tcp net
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "NetBaseObject.h"
#include "ZIPC/net/TcpDefine.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TcpThreadSocket;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! net stat
struct  BasicNetStat{
    uint32_t	m_dwSendBytes;		
    uint32_t	m_dwSendTimes;		
    uint32_t	m_dwReceBytes;		
    uint32_t	m_dwReceTimes;		
    time_t		m_tmLastRecTime;
    double		m_fLastSendRate;
    double		m_fLastRecvRate;
    time_t		m_tLastStat;
    BasicNetStat();
    void Empty();
    void OnSendData(int nSend);
    void OnReceiveData(int nRece);
    void GetTransRate(BasicNetStat& lastData, double& dSend, double& dRecv);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetSessionNotify : public NetBaseObject {
public:
    typedef std::function<int32_t(NetSessionNotify*)> FuncHandleConnect;
    typedef std::function<int32_t(NetSessionNotify*, int32_t, const char *)> FuncHandleReceive;
    typedef std::function<int32_t(NetSessionNotify*, uint32_t)> FuncHandleDisConnect;
    

    void bind_rece(const FuncHandleReceive& func){ 
        if(m_funcReceive == nullptr)
            m_funcReceive = func; 
    }
    void bind_connect(const FuncHandleConnect& func){ 
        if(m_funcConnect == nullptr)
            m_funcConnect = func; 
    }
    void bind_disconnect(const FuncHandleDisConnect& func){ 
        if(m_funcDisconnect == nullptr)
            m_funcDisconnect = func; 
    }
    const FuncHandleReceive GetBindRece(){ return m_funcReceive; }
    const FuncHandleConnect GetBindConnect(){ return m_funcConnect; }
    const FuncHandleDisConnect GetBindDisconnect(){ return m_funcDisconnect; }
    virtual uint32_t OnConnect() {
        m_stNet.Empty();
        m_stNet.OnReceiveData(0);
        if (m_funcConnect)
            return m_funcConnect(this);
        return BASIC_NET_OK;
    }
    virtual int32_t OnReceive(const char *pszData, int32_t cbData){
        m_stNet.OnReceiveData(cbData);
        if(m_funcReceive)
            return m_funcReceive(this, cbData, pszData);
        return BASIC_NET_OK;
    }
    virtual uint32_t OnDisconnect(uint32_t dwNetCode){
        if(m_funcDisconnect)
            return m_funcDisconnect(this, dwNetCode);
        return BASIC_NET_OK;
    }
    virtual void OnSendData(uint32_t dwIoSize) {
        m_stNet.OnSendData(dwIoSize);
    }
public:
    //! safe to delete shared_ptr when £¨close return true or after disconnect callback£©  
    void Close();

    int32_t Send(const char *pData, int32_t cbData);
    int32_t Send(const std::shared_ptr<char>& pData, int32_t cbData);
    int32_t SendNoNotify(const char *pData, int32_t cbData); //for test

    //! 
    bool IsConnected();

    //!
    bool IsTransmit();

    //!
    void GetNetStatInfo(BasicNetStat& netState);
protected:
    //!
    NetSessionNotify();
    virtual ~NetSessionNotify();

    //!
    virtual TcpThreadSocket* GetThreadSocket() = 0;
protected:
    //callback
    FuncHandleReceive               m_funcReceive = nullptr;
    FuncHandleConnect               m_funcConnect = nullptr;
    FuncHandleDisConnect            m_funcDisconnect = nullptr;

    BasicNetStat					m_stNet;
    BasicNetStat					m_lastNet;

    friend class TcpThreadSocket;
    friend class NetThread;
};

__NS_ZILLIZ_IPC_END
/***********************************************************************************************
// filename : 	tcpnotify.h
// describ:		tcp net
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"
#include "ZIPC/base/misc/fastdelegate.h"
#include "ZIPC/base/misc/fastdelegatebind.h"
#include "TcpDefine.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetThread;
class TcpSocket;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NetSessionNotify : public std::enable_shared_from_this<NetSessionNotify> {
public:
    typedef void(*pNetThreadCallback)(std::shared_ptr<NetSessionNotify>& pSession, TcpSocket* pSocket, intptr_t lRevert);
    typedef fastdelegate::FastDelegate4<NetSessionNotify*, uint32_t, int32_t, const char *, int32_t>	HandleReceive;
    typedef fastdelegate::FastDelegate2<NetSessionNotify*, uint32_t, int32_t>							HandleConnect;
    typedef fastdelegate::FastDelegate2<NetSessionNotify*, uint32_t, int32_t>							HandleDisConnect;
    typedef fastdelegate::FastDelegate2<NetSessionNotify*, uint32_t, int32_t>							HandleIdle;
    typedef fastdelegate::FastDelegate3<NetSessionNotify*, uint32_t, int32_t, int32_t>				    HandleError;
    typedef fastdelegate::FastDelegate1<NetSessionNotify*>				                                HandleSafeRelease;

    void bind_rece(const HandleReceive& func){ m_funcReceive = func; }
    void bind_connect(const HandleConnect& func){ m_funcConnect = func; }
    void bind_disconnect(const HandleDisConnect& func){ m_funcDisconnect = func; }
    void bind_idle(const HandleIdle& func){ m_funcIdle = func; }
    void bind_error(const HandleError& func){ m_funcError = func; }
    const HandleReceive GetBindRece(){ return m_funcReceive; }
    const HandleConnect GetBindConnect(){ return m_funcConnect; }
    const HandleDisConnect GetBindDisconnect(){ return m_funcDisconnect; }
    const HandleIdle GetBindIdle(){ return m_funcIdle; }
    const HandleError GetBindError(){ return m_funcError; }
    virtual uint32_t OnConnect(uint32_t dwNetCode) {
        m_stNet.Empty();
        m_stNet.OnReceiveData(0);
        if (m_funcConnect)
            return m_funcConnect(this, dwNetCode);
        return BASIC_NET_OK;
    }
    virtual int32_t OnReceive(uint32_t dwNetCode, const char *pszData, int32_t cbData){
        m_stNet.OnReceiveData(cbData);
        if(m_funcReceive)
            return m_funcReceive(this, dwNetCode, cbData, pszData);
        return BASIC_NET_OK;
    }
    virtual uint32_t OnDisconnect(uint32_t dwNetCode){
        if(m_funcDisconnect)
            return m_funcDisconnect(this, dwNetCode);
        return BASIC_NET_OK;
    }
    virtual uint32_t OnError(uint32_t dwNetCode, int32_t lRetCode){
        if(m_funcError)
            return m_funcError(this, dwNetCode, lRetCode);
        return BASIC_NET_OK;
    }
    virtual int32_t OnIdle(uint32_t dwIdleCount){
        if(m_funcIdle)
            return m_funcIdle(this, dwIdleCount);
        return BASIC_NET_OK;
    }
    virtual void OnSendData(uint32_t dwIoSize) {
        m_stNet.OnSendData(dwIoSize);
    }
public:
    //! safe to delete shared_ptr when £¨close return true or after disconnect callback£©  
    bool Close();

    virtual int32_t Send(char *pData, int32_t cbData);
    virtual int32_t Send(std::shared_ptr<char> pData, int32_t cbData);

    //! 
    bool IsConnected();

    //!
    bool IsTransmit();

    //!
    void GetNetStatInfo(BasicNetStat& netState);
protected:
    //
    NetSessionNotify();
    virtual ~NetSessionNotify();

    NetThread* GetNetThread();
    TcpSocket* GetTcpSocket();

    //!
    void InitSocket(TcpSocket* pSocket);

protected:
    //! change to netthread
    void GotoNetThread(pNetThreadCallback pCallback, intptr_t lRevert = 0);
protected:
    NetThread*                      m_pNetThread = nullptr;
    TcpSocket*						m_pSocket = nullptr;

    //callback
    HandleReceive					m_funcReceive = nullptr;
    HandleConnect					m_funcConnect = nullptr;
    HandleDisConnect				m_funcDisconnect = nullptr;
    HandleIdle						m_funcIdle = nullptr;
    HandleError						m_funcError = nullptr;

    BasicNetStat					m_stNet;
    BasicNetStat					m_lastNet;


    friend class TcpSocket;
    friend class NetThread;
public:
    //! 
    virtual int32_t Send(void* pData, int32_t cbData);
};

template<class T>
std::shared_ptr<NetSessionNotify> CreateTcpSession() {
    std::shared_ptr<NetSessionNotify> pRet = std::make_shared<NetSessionNotify>(new T());
    NetSessionNotify::InitNetSessionNotify(pRet);
    return pRet;
}

__NS_ZILLIZ_IPC_END
#include "TcpSocketTransmit.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////

ITCPSocketTransmit::ITCPSocketTransmit(uint32_t nSessionID, std::shared_ptr<NetSession> pSession, uint16_t usRecTimeout) :
ITCPSocket(nSessionID, pSession){
    m_usRecTimeout = usRecTimeout;
}
ITCPSocketTransmit::~ITCPSocketTransmit(){
}

bool ITCPSocketTransmit::CanClose(){
	bool bRet = CBasicNet_Socket::CanClose();
    if(bRet){
        if(m_msgQueue.GetDataLength() == 0)
            return true;
        return !IsConnected();
    }
    return false;
}

__NS_ZILLIZ_IPC_END
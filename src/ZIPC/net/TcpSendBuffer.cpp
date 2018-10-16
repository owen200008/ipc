#include "TcpSendBuffer.h"
#include "ZIPC/base/log/IPCLog.h"

__NS_ZILLIZ_IPC_START
////////////////////////////////////////////////////////////////////////////////////////
CMsgSendBuffer::CMsgSendBuffer(){
    SetDataLength(1024);
    SetDataLength(0);
}
CMsgSendBuffer::~CMsgSendBuffer(){

}

int32_t CMsgSendBuffer::SendBuffer(int32_t lSend){
    if(m_cbBuffer < lSend){
    	IPCLog::GetInstance().Log(IPCLog_Error, "CMsgSendBuffer::SendBuffer %d < %d", m_cbBuffer, lSend);
        m_cbBuffer = 0;
        return -1;
    }
    m_cbBuffer -= lSend;
    if(m_cbBuffer > 0){
        memmove(m_pszBuffer, m_pszBuffer + lSend, m_cbBuffer);
    }
    return m_cbBuffer;
}

__NS_ZILLIZ_IPC_END
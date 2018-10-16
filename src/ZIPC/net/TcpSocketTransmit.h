/***********************************************************************************************
// filename : 	TcpSocketTransmit.h
// describ:		tcp socket link
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "ZIPC/base/inc/IPCDefine.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
class ITCPSocketTransmit : public ITCPSocket{
public:
	ITCPSocketTransmit(uint32_t nSessionID, std::shared_ptr<NetSession> pSession,  uint16_t usRecTimeout = 0);
	virtual ~ITCPSocketTransmit();

	virtual bool CanClose();
protected:
	constexpr static int 			m_usTimeoutShakeHandle = 10;
	constexpr static int 			m_nSendMaxBufferPerTime = 4096;

	uint16_t						m_usRecTimeout = 0;				//
    uint32_t						m_unIdleCount = 0;				//

    event							m_wevent;

    BasicNetStat					m_stNet;
    BasicNetStat					m_lastNet;
};

__NS_ZILLIZ_IPC_END
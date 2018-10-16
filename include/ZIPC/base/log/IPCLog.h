/***********************************************************************************************
// filename : 	ipclog.h
// describ:		bind log function
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"
#include <functional>
#include <math.h>
#include <stdarg.h>

__NS_ZILLIZ_IPC_START

enum IPCLogType{
	IPCLog_Info,
	IPCLog_Warn,
	IPCLog_Error,
	IPCLog_Throw,//not continue
};

#define LOG_MESSAGE_SIZE 256

class IPCLog{
public:
	static IPCLog& GetInstance();

	//bind log func
	void BindLogFunc(const std::function<void(IPCLogType, const char*)>& func){
		m_logFunc = func;
	}

	template<class... _Types>
	void Log(IPCLogType type, const char* pszLog, _Types&&... _Args){
		char tmp[LOG_MESSAGE_SIZE];
        ccsnprintf(szBuf, LOG_MESSAGE_SIZE, pLog, std::forward<_Types>(_Args)...);
        Log(type, tmp);
	}

	void Log(IPCLogType type, const char* pszLog){
		if(nullptr != m_logFunc){
			m_logFunc(type, pszLog);
		}
	}
protected:
	std::function<void(IPCLogType, const char*)> m_logFunc;
};

__NS_ZILLIZ_IPC_END

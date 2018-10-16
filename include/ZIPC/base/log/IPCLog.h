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
		if(!LogFormat(tmp, LOG_MESSAGE_SIZE, type, pszLog, std::forward<_Types>(_Args)...)){
			for(auto i = 0;i < 10;i++){
				char* pBuf = malloc(LOG_MESSAGE_SIZE * pow(2, i + 1));
				if(pBuf){
					if(!LogFormat(tmp, LOG_MESSAGE_SIZE, type, pszLog, std::forward<_Types>(_Args)...)){
						continue;
					}
					free(pBuf);
				}
				break;
			}
		}
	}

	void Log(IPCLogType type, const char* pszLog){
		if(nullptr != m_logFunc){
			m_logFunc(type, pszLog);
		}
	}
protected:
	bool LogFormat(char* pBuf, int nLength, IPCLogType type, const char* pszLog, ...){
		va_list argList;
		va_start(argList, pszLog);
		int len = snprintf(pBuf, nLength, pszLog, argList);
		va_end(argList);
		bool bRet = len >=0 && len < nLength;
		if(bRet && nullptr != m_logFunc){
			m_logFunc(type, pBuf);
		}
		return bRet;
	}
protected:
	std::function<void(IPCLogType, const char*)> m_logFunc;
};

__NS_ZILLIZ_IPC_END

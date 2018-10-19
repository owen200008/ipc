/***********************************************************************************************
// filename : 	ipclog.h
// describ:		bind log function
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"
#include <sstream>

__NS_ZILLIZ_IPC_START

enum IPCLogType{
	IPCLog_Info,
	IPCLog_Warn,
	IPCLog_Error,
	IPCLog_Throw,//not continue
};

#define LOG_MESSAGE_SIZE 512

class IPCLog{
public:
	static IPCLog& GetInstance();

	//bind log func
	void BindLogFunc(const std::function<void(IPCLogType, const char*)>& func){
		m_logFunc = func;
	}

	template<class... _Types>
	void LogFunc(IPCLogType type, _Types&&... _Args){
        std::stringstream stream;
        LogStream(stream, _Args...);
        stream << std::endl;
        if (m_logFunc) {
            m_logFunc(type, stream.str().c_str());
        }
        else {
            printf(stream.str().c_str());
        }
	}
    template<class T, class... _Types>
	void LogStream(std::stringstream& stream, T first, _Types&&... _Args){
        stream << first;
        LogStream(stream, _Args...);
	}
    template<class T>
    void LogStream(std::stringstream& stream, T first) {
        stream << first;
    }
protected:
	std::function<void(IPCLogType, const char*)> m_logFunc;
};

#define LogFuncInfo(Type, ...)  zilliz::lib::IPCLog::GetInstance().LogFunc(Type, __VA_ARGS__)

#define LogFuncLocation(Type, ...)  zilliz::lib::IPCLog::GetInstance().LogFunc(Type, __FUNCTION__, "[", __FILE__, ":", __LINE__,"]", __VA_ARGS__)

__NS_ZILLIZ_IPC_END

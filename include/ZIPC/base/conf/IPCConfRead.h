/***********************************************************************************************
// filename : 	confread.h
// describ:		bind log function
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"
#include <functional>

__NS_ZILLIZ_IPC_START

class IPCConfRead{
public:
	static IPCConfRead& GetInstance();

	//bind readconf func
	void BindConfReadFunc(const std::function<const char*(const char*, const char*)>& func){
		m_func = func;
	}

	const char* GetConf(const char* pSection, const char* pKey, const char* pDefault){
		const char* pRet = nullptr;
		if(m_func != nullptr)
			pRet = m_func(pSection, pKey);
		return pRet ? pRet : pDefault;
	}
protected:
	std::function<const char*(const char*, const char*)> m_func;
};

__NS_ZILLIZ_IPC_END
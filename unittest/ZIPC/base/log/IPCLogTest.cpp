#include "IPCLogTest.h"
#include "ZIPC/base/log/IPCLog.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>


bool IPCLogTest(){
	bool bRet = true;
	auto logObject = zilliz::lib::IPCLog::GetInstance();
	logObject.BindLogFunc([&](zilliz::lib::IPCLogType type, const char* pData){
		if(type == zilliz::lib::IPCLog_Info)
			bRet &= strcmp("INFO", pData) == 0;
		else if(type == zilliz::lib::IPCLog_Warn)
			bRet &= strcmp("WARN", pData) == 0;
		else if(type == zilliz::lib::IPCLog_Error)
			bRet &= strcmp("ERROR", pData) == 0;
		else if(type == zilliz::lib::IPCLog_Throw)
			bRet &= strcmp("THROW", pData) == 0;
		else
			bRet = false;
		printf("Type: %d pData\n", type);
	});
    LogFuncInfo(zilliz::lib::IPCLog_Info, "INFO");
    LogFuncInfo(zilliz::lib::IPCLog_Warn, "WARN");
    LogFuncInfo(zilliz::lib::IPCLog_Error, "ERROR");
    LogFuncInfo(zilliz::lib::IPCLog_Throw, "THROW");

    LogFuncLocation(zilliz::lib::IPCLog_Info, "INFO");
    LogFuncLocation(zilliz::lib::IPCLog_Warn, "WARN");
    LogFuncLocation(zilliz::lib::IPCLog_Error, "ERROR");
    LogFuncLocation(zilliz::lib::IPCLog_Throw, "THROW");

	return bRet;
}
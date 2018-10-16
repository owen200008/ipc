#include "ZIPC/base/log/IPCLog.h"


__NS_ZILLIZ_IPC_START

IPCLog& IPCLog::GetInstance() {
    static IPCLog instance;
    return instance;
};

__NS_ZILLIZ_IPC_END
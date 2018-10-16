#include "ZIPC/base/conf/IPCConfRead.h"

__NS_ZILLIZ_IPC_START

IPCConfRead& IPCConfRead::GetInstance() {
    static IPCConfRead instance;
    return instance;
};

__NS_ZILLIZ_IPC_END
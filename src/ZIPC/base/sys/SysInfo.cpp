#include "ZIPC/base/sys/SysInfo.h"
#ifdef _MSC_VER
#include <Windows.h>
#endif

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
unsigned long IPCGetTickTime() {
#ifdef _MSC_VER
    //return ::GetTickCount();
    LARGE_INTEGER lFreq, lCounter;
    QueryPerformanceFrequency(&lFreq);
    QueryPerformanceCounter(&lCounter);
    return DWORD((((double)lCounter.QuadPart) / lFreq.QuadPart) * 1000);
#else
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
#endif
}

__NS_ZILLIZ_IPC_END
/***********************************************************************************************
// filename : 	ipcdefine.h
// describ:		define for ipc
// version:   	1.0V
************************************************************************************************/
#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include <future>
#include <unordered_map>

//定义命名空间
#define __NS_ZILLIZ_IPC_START	namespace zilliz { namespace lib {
#define __NS_ZILLIZ_IPC_END	}}

#ifdef _MSC_VER
#define CCSwitchToThread() SwitchToThread();
#define ccsnprintf sprintf_s
#else
#include <emmintrin.h>
#include <thread>
#define CCSwitchToThread() std::this_thread::yield();
#define ccsnprintf snprintf
#endif

typedef void*(*pIPCMalloc)(size_t);
typedef void(*pIPCFree)(void*);

#define IPCVector           std::vector
#define IPCMap              std::unordered_map
#define IPCQueue            std::queue
#define IPCString           std::string



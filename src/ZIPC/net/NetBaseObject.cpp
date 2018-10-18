#include "ZIPC/net/NetBaseObject.h"
#include "ZIPC/net/NetMgr.h"
#include "NetThread.h"

__NS_ZILLIZ_IPC_START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetBaseObject::NetBaseObject() {
	m_pNetThread = NetMgr::GetInstance().AssignNetThread();
}

NetBaseObject::~NetBaseObject() {

}

__NS_ZILLIZ_IPC_END
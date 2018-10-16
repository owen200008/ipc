#include "ZIPC/base/mt/SpinLock.h"
#include "ZIPC/base/mt/AtomicBackOff.h"

__NS_ZILLIZ_IPC_START
////////////////////////////////////////////////////////////////////////////////////////////////
CSpinLockFunc::CSpinLockFunc(SpinLock* pLock, bool bInitialLock){
	m_pLock = pLock;
	m_bAcquired = false;
	if (bInitialLock)
		Lock();
}
CSpinLockFunc::~CSpinLockFunc(){
	UnLock();
}

void CSpinLockFunc::Lock(){
	AtomicBackOff pauseObj;
	while(m_pLock->m_nLock.exchange(1)){
		pauseObj.Pause();
	}
	m_bAcquired = true;
}
bool CSpinLockFunc::TryLock(){
	while(m_pLock->m_nLock.exchange(1)){
		return false;
	}
	m_bAcquired = true;
	return true;
}

void CSpinLockFunc::UnLock(){
	if (m_bAcquired){
		m_bAcquired = false;
		m_pLock->m_nLock.exchange(0);
	}
}

bool CSpinLockFunc::IsLock(){
	return m_pLock->m_nLock.load() != 0;
}
//////////////////////////////////////////////////////////////////////////

__NS_ZILLIZ_IPC_END

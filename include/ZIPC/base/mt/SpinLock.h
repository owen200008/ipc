/***********************************************************************************************
// filename : 	spinlock.h
// describ:		spin lock 
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"

__NS_ZILLIZ_IPC_START
//////////////////////////////////////////////////////////////////////////
struct SpinLock{
	std::atomic<int>		m_nLock = {0};
};

class CSpinLockFunc
{
public:
	CSpinLockFunc(SpinLock* pLock, bool bInitialLock = false);
	virtual ~CSpinLockFunc();

	void Lock();
	bool TryLock();
	void UnLock();
	bool IsLock();
protected:
	SpinLock* 		m_pLock;
	bool			m_bAcquired;
};


__NS_ZILLIZ_IPC_END
/***********************************************************************************************
// filename : 	SmartBuffer.h
// describ:		auto change length
// version:   	1.0V
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"

__NS_ZILLIZ_IPC_START

class SmartBuffer{
public:
	SmartBuffer();
	SmartBuffer(int nAllocaSize);
	SmartBuffer(const SmartBuffer &smBuf);
	virtual ~SmartBuffer();
public:
	bool IsEmpty() const { return m_pszBuffer == nullptr; }
	void Free();

	char* GetDataBuffer(long& lLength) const;
	char* GetDataBuffer() const { return m_pszBuffer; }

	long GetDataLength() const{ return m_cbBuffer; }
	bool SetDataLength(long lLength);

	char* AppendData(const char* pszData, long lLength);
	char* AppendDataEx(const char* pszData, long lLength);
	char* CommitData(long lLength);

	void AppendString(const char* lpszText);
	void AppendData(double fVal);

	SmartBuffer& operator = (const SmartBuffer& buffer);
	friend bool operator == (SmartBuffer& b1, SmartBuffer& b2);

	SmartBuffer& operator << (const char* lpszText){
		AppendString(lpszText);
		return *this;
	}
	//
	bool ReadData(void* pData, int nLength);

	//
	long GetAllocBufferLength(){ return m_cbAlloc; }

	//
	void ThrowDataLength(int nLength);
protected:
	char* AllocBuffer(long lLength, long lGrowLength = 0);
	void  EmptyBuffer();
protected:
	char*	m_pszBuffer;
	long	m_cbAlloc;
	long	m_cbBuffer;
};
bool operator == (SmartBuffer& b1, SmartBuffer& b2);

__NS_ZILLIZ_IPC_END


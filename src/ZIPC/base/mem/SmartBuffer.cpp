#include "ZIPC/base/mem/SmartBuffer.h"
#include <algorithm>
#include <cstring>

__NS_ZILLIZ_IPC_START

#define GROW_SIZE		1024

#define GROW_MODE_NORMAL		0			//GROW *2
#define GROW_MODE_FAST			-1			//quick grow

inline long _GetGrowLength(long lOldLength, long lLength, long lGrowMode){
	if (lGrowMode == GROW_MODE_NORMAL){
		return ((lOldLength + lLength - 1) / GROW_SIZE + 1) * GROW_SIZE;
	}
	else if (lGrowMode == GROW_MODE_FAST){
		long lStep = std::max(lOldLength + lLength, 2 * lOldLength);
		return ((lStep - 1) / GROW_SIZE + 1) * GROW_SIZE;
	}
	return lLength;
}

SmartBuffer::SmartBuffer(){
	EmptyBuffer();
}

SmartBuffer::SmartBuffer(int nAllocaSize){
	EmptyBuffer();
	if (nullptr == AllocBuffer(nAllocaSize)){
		return;
	}
}

SmartBuffer::SmartBuffer(const SmartBuffer &buffer){
	EmptyBuffer();
	Free();
	AppendData(buffer.m_pszBuffer, buffer.m_cbBuffer);
}

SmartBuffer::~SmartBuffer(){
	Free();
}

void SmartBuffer::EmptyBuffer(){
	m_pszBuffer = nullptr;
	m_cbAlloc = 0;
	m_cbBuffer = 0;
}

void SmartBuffer::Free(){
	if (m_pszBuffer){
		free(m_pszBuffer);
	}
	EmptyBuffer();
}

bool SmartBuffer::SetDataLength(long lLength){
	if (lLength < 0){
		//special case
	}
	else{
		long lAddLength = lLength - m_cbBuffer;
		if (lAddLength > 0){
			if (AllocBuffer(lAddLength) == nullptr){
				return false;
			}
		}
		m_cbBuffer = lLength;
	}
	return true;
}


char* SmartBuffer::AppendData(const char* pszData, long lLength){
	char* pBuffer = AllocBuffer(lLength);
	if (pBuffer != nullptr && pszData != nullptr){
		memcpy(pBuffer, pszData, lLength);
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

char* SmartBuffer::AppendDataEx(const char* pszData, long lLength){
	char* pBuffer = AllocBuffer(lLength, GROW_MODE_FAST);
	if (pBuffer != nullptr && pszData != nullptr){
		memcpy(pBuffer, pszData, lLength);
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

char* SmartBuffer::CommitData(long lLength){
	char* pBuffer = AllocBuffer(lLength, GROW_MODE_FAST);
	if (pBuffer != nullptr){
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

void SmartBuffer::AppendString(const char* lpszText){
	if (lpszText != nullptr && lpszText[0] != '\0'){
		long lLength = strlen(lpszText) * sizeof(char);
        AllocBuffer(lLength + sizeof(char), GROW_MODE_FAST);
		AppendData((const char*)lpszText, lLength);
	}
}

void SmartBuffer::AppendData(double fVal){
	char szBuf[64];
	snprintf(szBuf, 64, "%f", fVal);
	AppendString(szBuf);
}

char* SmartBuffer::AllocBuffer(long lLength, long lGrowLength){
	if (m_cbBuffer + lLength > m_cbAlloc){
		long lNewSize = _GetGrowLength(m_cbBuffer, lLength, lGrowLength);

		char* pTemp = (char*)malloc(lNewSize);
		if (pTemp == nullptr){
			return nullptr;
		}
		memset(pTemp, 0, lNewSize);
		if (m_pszBuffer != nullptr){
			memcpy(pTemp, m_pszBuffer, m_cbBuffer);
		}
		m_cbAlloc = lNewSize;
		m_pszBuffer = pTemp;
	}
	return &m_pszBuffer[m_cbBuffer];
}

SmartBuffer& SmartBuffer::operator = (const SmartBuffer& buffer){
	SetDataLength(0);
	AppendData(buffer.m_pszBuffer, buffer.m_cbBuffer);
	return *this;
}

char* SmartBuffer::GetDataBuffer(long& lLength) const{
	lLength = m_cbBuffer;
	return m_pszBuffer;
}

//throw data length
void SmartBuffer::ThrowDataLength(int nLength){
	if(m_cbBuffer < nLength)
		m_cbBuffer = 0;
	m_cbBuffer -= nLength;
	memmove(m_pszBuffer, m_pszBuffer + nLength, m_cbBuffer);
}

//read data
bool SmartBuffer::ReadData(void* pBuffer, int nLength){
	if (m_cbBuffer < nLength || nLength <= 0){
		return false;
	}
	if (pBuffer)
		memcpy(pBuffer, m_pszBuffer, nLength);
	m_cbBuffer -= nLength;
	memmove(m_pszBuffer, m_pszBuffer + nLength, m_cbBuffer);
	return true;
}

bool operator == (SmartBuffer& b1, SmartBuffer& b2){
	if (b1.GetDataLength() != b2.GetDataLength())
		return false;
	return memcmp(b1.GetDataBuffer(), b2.GetDataBuffer(), b1.GetDataLength()) == 0;
}



__NS_ZILLIZ_IPC_END
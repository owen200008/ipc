/***********************************************************************************************
// filename : 	BitStream.h
// describ:		auto change length
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

#include "../inc/IPCDefine.h"
#include "SmartBuffer.h"

__NS_ZILLIZ_IPC_START

int SerializeUChar(unsigned char* pBuffer, const uint8_t v);
int SerializeUShort(unsigned char* pBuffer, const uint16_t v);
int SerializeUInt3Bit(unsigned char* pBuffer, const uint32_t v);
int SerializeUInt(unsigned char* pBuffer, const uint32_t v);
int SerializeLONGLONG(unsigned char* pBuffer, const int64_t v);
int SerializeCBasicString(unsigned char* pBuffer, const int8_t* v, uint16_t usLength);
int UnSerializeUChar(unsigned char* pBuffer, uint8_t& v);
int UnSerializeChar(unsigned char* pBuffer, int8_t& v);
int UnSerializeUShort(unsigned char* pBuffer, uint16_t& v);
int UnSerializeShort(unsigned char* pBuffer, int16_t& v);
int UnSerializeUInt3Bit(unsigned char* pBuffer, uint32_t& v);
int UnSerializeUInt(unsigned char* pBuffer, uint32_t& v);
int UnSerializeInt(unsigned char* pBuffer, int32_t& v);
int UnSerializeLONGLONG(unsigned char* pBuffer, int64_t& v);

class BitStream : public SmartBuffer{
public:
    BitStream();
    BitStream(const std::string& s);
    BitStream(void* buf, int size);
    BitStream(const char* buf, int size);
    virtual ~BitStream();

    /////////////////////////////////////////////////////////////////////////////
    //串行化
    //字符串操作
    BitStream& operator << (const uint8_t v);
    BitStream& operator << (const int8_t v);
    BitStream& operator << (const uint16_t v);
    BitStream& operator << (const int16_t v);
    BitStream& operator << (const uint32_t v);
    BitStream& operator << (const int32_t v);
    BitStream& operator << (const int64_t v);
    BitStream& operator << (const double v);

    BitStream& operator << (const SmartBuffer* pV);
    BitStream& operator << (const int8_t* v);
    BitStream& operator << (const uint8_t* v);
    BitStream& operator << (const SmartBuffer& pV);
    template<typename A>
    BitStream& operator << (const IPCSet<A>& data){
        uint16_t uSize = data.size();
        *this << uSize;
        for (auto& key : data){
            *this << key;
        }
        return *this;
    }
    template<class A>
    BitStream& operator << (const IPCVector<A>& data){
        uint16_t uSize = (uint16_t)data.size();
        *this << uSize;
        for (auto& key : data){
            *this << key;
        }
        return *this;
    }
    template<class A, class B>
    BitStream& operator << (const IPCMap<A, B>& data) {
        uint16_t uSize = (uint16_t)data.size();
        *this << uSize;
        for (auto& key : data){
            *this << key.first;
            *this << key.second;
        }
        return *this;
    }
    /////////////////////////////////////////////////////////////////////////////
    //反串行化
    //字符串
    BitStream& operator >> (uint8_t& v);
    BitStream& operator >> (int8_t& v);
    BitStream& operator >> (uint16_t& v);
    BitStream& operator >> (int16_t& v);
    BitStream& operator >> (uint32_t& v);
    BitStream& operator >> (int32_t& v);
    BitStream& operator >> (int64_t& v);
    BitStream& operator >> (double& v);
    BitStream& operator >> (SmartBuffer* pV);
    BitStream& operator >> (int8_t* v);
    BitStream& operator >> (uint8_t* v);
    BitStream& operator >> (SmartBuffer& pV);
    template<class A>
    BitStream& operator >> (IPCSet<A>& data){
        uint16_t uSize = 0;
        *this >> uSize;
        if (uSize > 0){
            for (uint16_t i = 0; i < uSize; i++){
                A intKey;
                *this >> intKey;
                data.insert(intKey);
            }
        }
        return *this;
    }
    template<class A>
    BitStream& operator >> (IPCVector<A>& data){
        uint16_t uSize = 0;
        *this >> uSize;
        if (uSize > 0){
            data.resize(uSize);
            for (uint16_t i = 0; i < uSize; i++){
                *this >> data[i];
            }
        }
        return *this;
    }
    template<class A, class B>
    BitStream& operator >> (IPCMap<A, B>& data) {
        uint16_t uSize = 0;
        *this >> uSize;
        for (uint16_t i = 0; i < uSize; i++){
            A intKey;
            *this >> intKey;
            *this >> data[intKey];
        }
        return *this;
    }
    /////////////////////////////////////////////////////////////////////////////
    void SerializeDataBuffer(const int8_t* pData, uint16_t usLength);
    void UnSerializeSmbuf(SmartBuffer* pV);
protected:
    unsigned char m_szBuf[8];			//最大编码8字节整型
};


__NS_ZILLIZ_IPC_END


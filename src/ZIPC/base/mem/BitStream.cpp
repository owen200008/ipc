#include "ZIPC/base/mem/BitStream.h"
#include <algorithm>
#include <cstring>

__NS_ZILLIZ_IPC_START


int SerializeUChar(unsigned char* pBuffer, const uint8_t v) {
    pBuffer[0] = v;
    return 1;
}

int SerializeUShort(unsigned char* pBuffer, const uint16_t v) {
    pBuffer[0] = (v) & 0xFF;
    pBuffer[1] = (v >> 8) & 0xFF;
    return 2;
}

int SerializeUInt(unsigned char* pBuffer, const uint32_t v) {
    pBuffer[0] = (v) & 0xFF;
    pBuffer[1] = (v >> 8) & 0xFF;
    pBuffer[2] = (v >> 16) & 0xFF;
    pBuffer[3] = (v >> 24) & 0xFF;
    return 4;
}
int SerializeUInt3Bit(unsigned char* pBuffer, const uint32_t v) {
    pBuffer[0] = (v) & 0xFF;
    pBuffer[1] = (v >> 8) & 0xFF;
    pBuffer[2] = (v >> 16) & 0xFF;
    return 3;
}

int SerializeLONGLONG(unsigned char* pBuffer, const int64_t v) {
    pBuffer[0] = (v) & 0xFF;
    pBuffer[1] = (v >> 8) & 0xFF;
    pBuffer[2] = (v >> 16) & 0xFF;
    pBuffer[3] = (v >> 24) & 0xFF;
    pBuffer[4] = (v >> 32) & 0xFF;
    pBuffer[5] = (v >> 40) & 0xFF;
    pBuffer[6] = (v >> 48) & 0xFF;
    pBuffer[7] = (v >> 56) & 0xFF;
    return 8;
}

int UnSerializeUChar(unsigned char* pBuffer, uint8_t& v) {
    v = pBuffer[0];
    return 1;
}
int UnSerializeChar(unsigned char* pBuffer, int8_t& v) {
    v = pBuffer[0];
    return 1;
}

int UnSerializeUShort(unsigned char* pBuffer, uint16_t& v) {
    v = pBuffer[0] | pBuffer[1] << 8;
    return 2;
}
int UnSerializeShort(unsigned char* pBuffer, int16_t& v) {
    v = pBuffer[0] | pBuffer[1] << 8;
    return 2;
}
int UnSerializeUInt3Bit(unsigned char* pBuffer, uint32_t& v) {
    v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16;
    return 3;
}
int UnSerializeUInt(unsigned char* pBuffer, uint32_t& v) {
    v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
    return 4;
}
int UnSerializeInt(unsigned char* pBuffer, int32_t& v) {
    v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
    return 4;
}
int UnSerializeLONGLONG(unsigned char* pBuffer, int64_t& v) {
    int64_t vHigh = pBuffer[4] << 0 | pBuffer[5] << 8 | pBuffer[6] << 16 | pBuffer[7] << 24;
    int64_t vLow = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
    v = vLow | vHigh << 32;
    return 8;
}
int SerializeCBasicString(unsigned char* pBuffer, const int8_t* v, uint16_t usLength){
    int nRetSize = 0;
    nRetSize = SerializeUShort(pBuffer, usLength);
    if (usLength > 0)
    {
        memcpy(pBuffer + nRetSize, v, usLength);
    }
    return nRetSize + usLength;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BitStream::BitStream(){
}

BitStream::~BitStream(){
}

BitStream::BitStream(const std::string& s){
    AppendData(s.c_str(), s.size());
}

BitStream::BitStream(void* buf, int size){
    AppendData((char*)buf, size);
}
BitStream::BitStream(const char* buf, int size){
    AppendData(buf, size);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BitStream& BitStream::operator >> (int8_t* v){
    uint16_t l = 0;
    *this >> l;
    if (l > 0)
        ReadData(v, l);
    return *this;
}
BitStream& BitStream::operator >> (uint8_t* v){
    uint16_t l = 0;
    *this >> l;
    if (l > 0)
        ReadData((char*)v, l);
    return *this;
}

BitStream& BitStream::operator >> (SmartBuffer& os){
    uint16_t l = 0;
    *this >> l;
    if (l > 0)
    {
        os.SetDataLength(l);
        ReadData(os.GetDataBuffer(), l);
    }
    return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BitStream& BitStream::operator << (const uint8_t v){
    AppendData((const char*)&v, sizeof(v));
    return *this;
}
BitStream& BitStream::operator << (const int8_t v){
    AppendData((const char*)&v, sizeof(v));
    return *this;
}
BitStream& BitStream::operator << (const uint16_t v){
    AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
    return *this;
}
BitStream& BitStream::operator << (const int16_t v){
    AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
    return *this;
}
BitStream& BitStream::operator << (const uint32_t v){
    AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
    return *this;
}
BitStream& BitStream::operator << (const int32_t v){
    AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
    return *this;
}
BitStream& BitStream::operator << (const int64_t v){
    AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, v));
    return *this;
}
BitStream& BitStream::operator << (const double v){
    AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, (*(int64_t*)&v)));
    return *this;
}


BitStream& BitStream::operator << (const SmartBuffer* pV){
    SerializeDataBuffer((int8_t*)pV->GetDataBuffer(), (uint16_t)pV->GetDataLength());
    return *this;
}
BitStream& BitStream::operator << (const int8_t* v){
    SerializeDataBuffer(v, (uint16_t)strlen((char*)v));
    return *this;
}
BitStream& BitStream::operator << (const uint8_t* v){
    SerializeDataBuffer((const int8_t*)v, (uint16_t)strlen((const char*)v));
    return *this;
}
BitStream& BitStream::operator << (const SmartBuffer& insRet){
    SerializeDataBuffer((int8_t*)insRet.GetDataBuffer(), (uint16_t)insRet.GetDataLength());
    return *this;
}
void BitStream::SerializeDataBuffer(const int8_t* pData, uint16_t usLength){
    uint32_t nResLength = m_cbBuffer;
    SetDataLength(nResLength + sizeof(uint16_t) + usLength);
    SerializeCBasicString((unsigned char*)(m_pszBuffer + nResLength), pData, usLength);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BitStream& BitStream::operator >> (uint8_t& v){
    ReadData(&v, sizeof(v));
    return *this;
}
BitStream& BitStream::operator >> (int8_t& v){
    ReadData(&v, sizeof(v));
    return *this;
}
BitStream& BitStream::operator >> (uint16_t& v){
    ReadData(m_szBuf, 2);
    UnSerializeUShort(m_szBuf, v);
    return *this;
}
BitStream& BitStream::operator >> (int16_t& v){
    ReadData(m_szBuf, 2);
    UnSerializeShort(m_szBuf, v);
    return *this;
}
BitStream& BitStream::operator >> (uint32_t& v){
    ReadData(m_szBuf, 4);
    UnSerializeUInt(m_szBuf, v);
    return *this;
}
BitStream& BitStream::operator >> (int32_t& v){
    ReadData(m_szBuf, 4);
    UnSerializeInt(m_szBuf, v);
    return *this;
}
BitStream& BitStream::operator >> (int64_t& v){
    ReadData(m_szBuf, 8);
    UnSerializeLONGLONG(m_szBuf, v);
    return *this;
}
BitStream& BitStream::operator >> (double& v){
    int64_t vValue;
    *this >> vValue;
    v = *(double*)&vValue;
    return *this;
}

BitStream& BitStream::operator >> (SmartBuffer* pV){
    UnSerializeSmbuf(pV);
    return *this;
}

void BitStream::UnSerializeSmbuf(SmartBuffer* pV){
    uint16_t l = 0;
    *this >> l;
    if (l > 0){
        pV->SetDataLength(l);
        ReadData(pV->GetDataBuffer(), l);
    }
}


__NS_ZILLIZ_IPC_END
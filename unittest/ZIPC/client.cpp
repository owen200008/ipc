#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <cstring>
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/conf/IPCConfRead.h"
#include "ZIPC/base/time/OnTimer.h"
#include "ZIPC/base/mem/BitStream.h"
#include "ZIPC/net/TcpClient.h"

#include "tcpdefinetest.h"

class TcpClientTest : public zilliz::lib::TcpClient {
public:
    //! 
    static std::shared_ptr<TcpClient> CreateTcpClientTest(const std::shared_ptr<char> pSendData, uint32_t nLength) {
        auto p = new TcpClientTest();
        p->m_pSendData = pSendData;
        p->m_nSendLength = nLength;
        auto pClient = std::shared_ptr<TcpClient>(p);
        pClient->InitTcpClient();
        return pClient;
    }
    void Reset() {
        m_bAuth = false;
        n_bCheckFinish = false;
        m_nActionIndex = 0;
        m_nCheckLength = 0;
        m_receive.SetDataLength(0);
    }
    bool CheckReceive() {
        auto nReceiveLength = m_receive.GetDataLength();
        if (nReceiveLength > 0) {
            if (m_nCheckLength + nReceiveLength > m_nSendLength) {
                printf("error receive too much %d %d %d\n", m_nCheckLength, nReceiveLength, m_nSendLength);
                return false;
            }
            if (memcmp(m_pSendData.get() + m_nCheckLength, m_receive.GetDataBuffer(), nReceiveLength) != 0) {
                printf("check buffer error %d %d %d\n", m_nCheckLength, nReceiveLength, m_nSendLength);
                return false;
            }
            m_receive.SetDataLength(0);
            m_nCheckLength += nReceiveLength;
            if (m_nCheckLength == m_nSendLength) {
                n_bCheckFinish = true;
                printf("check finish close!\n");
                Close();
            }
        }
        return true;
    }

    bool                    m_bAuth = false;
    bool                    n_bCheckFinish = false;
    uint32_t                m_nActionIndex = 0;
    uint32_t                m_nCheckLength = 0;
    std::shared_ptr<char>   m_pSendData;
    uint32_t                m_nSendLength = 0;
    //receive
    zilliz::lib::BitStream  m_receive;

    //use only in callback
    zilliz::lib::BitStream  m_send;

    //calc use time
    uint64_t                m_begin;

};

void StartServerSend(const char* pAddress, const std::shared_ptr<char>& pSendData, uint32_t nLength) {
    do {
        bool bContinue = true;
        std::shared_ptr<zilliz::lib::TcpClient> pClient = TcpClientTest::CreateTcpClientTest(pSendData, nLength);
        pClient->bind_connect([](zilliz::lib::NetSessionNotify* pSession)->int32_t {
            return BASIC_NET_OK;
        });
        pClient->bind_rece([](zilliz::lib::NetSessionNotify* pNotify, int32_t cbData, const char * pData)->int32_t {
            TcpClientTest* pSession = (TcpClientTest*)pNotify;
            pSession->m_receive.AppendData(pData, cbData);
            if (!pSession->m_bAuth) {
                if (pSession->m_receive.GetDataLength() != 4) {
                    printf("auth error (%d)\n", cbData);
                    pSession->Close();
                    return BASIC_NET_OK;
                }
                pSession->m_receive >> pSession->m_nActionIndex;
                pSession->m_bAuth = true;
                pSession->Send(pData, cbData);
                pSession->m_begin = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                return BASIC_NET_OK;
            }
            switch (pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max) {
            case TCPTestUseCaseServerSendFirst_ConnectCloseByServer: {
                printf("TCPTestUseCaseServerSendFirst_ConnectCloseByServer error(%d)\n", pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max);
                break;
            }
            case TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer: {
                if (pSession->m_receive.GetDataLength() != 0) {
                    printf("TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer error length(%d) (%d) \n", pSession->m_receive.GetDataLength(), pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max);
                }
                break;
            }
            default: {
                //deal with data
                pSession->CheckReceive();
                pSession->Send(pData, cbData);
                break;
            }

            }
            

            //pSession->Send(pData, cbData);
            return BASIC_NET_OK;
        });
        pClient->bind_disconnect([&](zilliz::lib::NetSessionNotify* pNotify, uint32_t dwNetCode)->int32_t {
            if (dwNetCode & BASIC_NETCODE_CLOSE_REMOTE) {
                printf("Disconnect by remote\n");
            }
            else {
                printf("Disconnect\n");
            }
            
            TcpClientTest* pSession = (TcpClientTest*)pNotify;
            if (pSession->n_bCheckFinish) {
                uint64_t nEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                double dReceiveMB = ((double)pSession->m_nSendLength) * 1000 / (1024 * 1024) / (nEnd - pSession->m_begin);
                printf("CheckFinish %.2fMB/s\n", dReceiveMB);
            }
            if (bContinue) {
                pSession->Reset();
                pSession->DoConnect();
            }
            return BASIC_NET_OK;
        });
        auto retConnect = pClient->Connect(pAddress);
        if (retConnect != BASIC_NET_OK) {
            printf("connect return error %d\n", retConnect);
            break;
        }
        getchar();
        bContinue = false;
        if (!pClient->IsShutdown()) {
            pClient->Shutdown();
            std::chrono::milliseconds dura(1000);
            std::this_thread::sleep_for(dura);
        }
    } while (false);
}

int main() {
    //bind log function
    zilliz::lib::IPCLog::GetInstance().BindLogFunc([](zilliz::lib::IPCLogType, const char* pLog) {
        printf(pLog);
    });
    //bind read conf
    zilliz::lib::IPCConfRead::GetInstance().BindConfReadFunc([](const char* pSection, const char* pKey)->const char* {
        return nullptr;
    });
    //must init before use net
    zilliz::lib::NetMgr::GetInstance().Initialize();

    std::shared_ptr<char> pSendBuffer;
    int32_t nSendLength = 0;
    do {
        //构建发送的buffer
        {
            zilliz::lib::BitStream sendBuffer;
            sendBuffer.SetDataLength(1024 * 1024 * 400);
            sendBuffer.SetDataLength(0);
            for (int i = 0; i < 1024; i++) {
                sendBuffer << i;
            }
            for (int j = 0; j < 2; j++) {
                for (int i = 0; i < 10; i++) {
                    sendBuffer.AppendData(sendBuffer.GetDataBuffer(), sendBuffer.GetDataLength());
                }
            }
            nSendLength = sendBuffer.GetDataLength();
            pSendBuffer = std::shared_ptr<char>(new char[nSendLength]);
            memcpy(pSendBuffer.get(), sendBuffer.GetDataBuffer(), nSendLength);
        }
        
        StartServerSend("127.0.0.1:19999", pSendBuffer, nSendLength);
    } while (0);

    getchar();
    zilliz::lib::NetMgr::GetInstance().CloseNetSocket();
    return 0;
}
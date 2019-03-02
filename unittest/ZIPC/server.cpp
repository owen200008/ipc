#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <cstring>
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/base/log/IPCLog.h"
#include "ZIPC/base/conf/IPCConfRead.h"
#include "ZIPC/base/time/OnTimer.h"
#include "ZIPC/base/mem/BitStream.h"
#include "ZIPC/net/TcpServer.h"
#include "tcpdefinetest.h"

class TcpServerSessionTest : public zilliz::lib::TcpServerSession {
public:
    virtual ~TcpServerSessionTest() {

    }

    //!
    static std::shared_ptr<TcpServerSession> CreateTcpServerSessionTest(const std::shared_ptr<char>& pSendData, uint32_t nLength) {
        auto p = new TcpServerSessionTest();
        p->m_pSendData = pSendData;
        p->m_nSendLength = nLength;
        return std::shared_ptr<TcpServerSession>(p);
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
    std::shared_ptr<char>                   m_pSendData;
    uint32_t                m_nSendLength = 0;
    //receive
    zilliz::lib::BitStream  m_receive;

    //use only in callback
    zilliz::lib::BitStream  m_send;

    //calc use time
    uint64_t                m_begin;
protected:
    TcpServerSessionTest() {
        m_nActionIndex = m_allocateActionIndex.fetch_add(1);
    }

    static std::atomic<uint32_t> m_allocateActionIndex;
};

std::atomic<uint32_t> TcpServerSessionTest::m_allocateActionIndex = { 0 };

void StartServerSend(const char* pListen, const std::shared_ptr<char>& pSendData, uint32_t nLength) {
    do {
        //init ontimer
        zilliz::lib::OnTimer ipcOnTimer;
        if (!ipcOnTimer.InitTimer()) {
            printf("InitTimer error\n");
            break;
        }

        auto pServer = zilliz::lib::TcpServer::CreateTcpServer();
        auto retInitServer = pServer->InitTcpServer(pListen, [&](std::shared_ptr<zilliz::lib::TcpServerSession>& pSession) {
            pSession = TcpServerSessionTest::CreateTcpServerSessionTest(pSendData, nLength);

            pSession->bind_connect([&](zilliz::lib::NetSessionNotify* pNotify)->int32_t {
                TcpServerSessionTest* pSession = (TcpServerSessionTest*)pNotify;
                switch (pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max) {
                case TCPTestUseCaseServerSendFirst_ConnectCloseByServer:
                    pSession->Close();
                    break;
                case TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer: {
                    pSession->m_send.SetDataLength(0);
                    pSession->m_send << pSession->m_nActionIndex;
                    pSession->Send(pSession->m_send.GetDataBuffer(), pSession->m_send.GetDataLength());
                    pSession->Close();
                    break;
                }
                default:
                    pSession->m_send.SetDataLength(0);
                    pSession->m_send << pSession->m_nActionIndex;
                    pSession->Send(pSession->m_send.GetDataBuffer(), pSession->m_send.GetDataLength());
                }
                return BASIC_NET_OK;
            });
            pSession->bind_rece([&](zilliz::lib::NetSessionNotify* pNotify, int32_t cbData, const char* pData)->int32_t {
                TcpServerSessionTest* pSession = (TcpServerSessionTest*)pNotify;
                pSession->m_receive.AppendData(pData, cbData);

                switch (pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max) {
                case TCPTestUseCaseServerSendFirst_ConnectCloseByServer: {
                    printf("TCPTestUseCaseServerSendFirst_ConnectCloseByServer error(%d)\n", pSession->m_nActionIndex);
                    break;
                }
                case TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer: {
                    if (pSession->m_receive.GetDataLength() != 4) {
                        printf("TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer error(%d)\n", pSession->m_nActionIndex);
                    }
                    else {
                        uint32_t nActionIndex = 0;
                        pSession->m_receive >> nActionIndex;
                        if (nActionIndex != pSession->m_nActionIndex) {
                            printf("%d nActionIndex error %d(%d)\n", pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max, pSession->m_nActionIndex, nActionIndex);
                        }
                    }
                    break;
                }
                    break;
                default:{
                    if (!pSession->m_bAuth) {
                        if (pSession->m_receive.GetDataLength() < 4) {
                            printf("%d auth error (%d)\n", pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max, cbData);
                            pSession->Close();
                            break;
                        }
                        uint32_t nActionIndex = 0;
                        pSession->m_receive >> nActionIndex;
                        if (nActionIndex != pSession->m_nActionIndex) {
                            printf("%d nActionIndex error %d(%d)\n", pSession->m_nActionIndex % TCPTestUseCaseServerSendFirst_Max, pSession->m_nActionIndex, nActionIndex);
                            pSession->Close();
                            break;
                        }
                        pSession->m_bAuth = true;
                        pSession->m_begin = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        pSession->Send(pSession->m_pSendData, pSession->m_nSendLength);
                    }
                    //deal with data
                    pSession->CheckReceive();
                    break;
                }

                }
                return BASIC_NET_OK;
            });
            pSession->bind_disconnect([](zilliz::lib::NetSessionNotify* pNotify, uint32_t dwNetCode)->int32_t {
                if (dwNetCode & BASIC_NETCODE_CLOSE_REMOTE) {
                    printf("Disconnect session %d by remote\n", ((zilliz::lib::TcpServerSession*)pNotify)->GetSessionID());
                }
                else {
                    printf("Disconnect session %d by local\n", ((zilliz::lib::TcpServerSession*)pNotify)->GetSessionID());
                }
                TcpServerSessionTest* pSession = (TcpServerSessionTest*)pNotify;
                if (pSession->n_bCheckFinish) {
                    uint64_t nEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    double dReceiveMB = ((double)pSession->m_nSendLength) * 1000 / (1024 * 1024) / (nEnd - pSession->m_begin);
                    printf("CheckFinish %.2fMB/s\n", dReceiveMB);
                }
                return BASIC_NET_OK;
            });
        });
        if (retInitServer != BASIC_NET_OK) {
            printf("InitTcpServer error %d\n", retInitServer);
            break;
        }
        auto retListen = pServer->DoListen();
        if (retListen != BASIC_NET_OK) {
            printf("Listen error %d\n", retListen);
            break;
        }
        printf("Listen OK %s\n", pListen);

        getchar();

        while (pServer->IsShutdown()) {
            pServer->Shutdown();
            std::chrono::milliseconds dura(10000);
            std::this_thread::sleep_for(dura);
        }
    } while (false);
}

int main(){
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
            for (uint32_t i = 0; i < 100; i++) {
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
        StartServerSend("0.0.0.0:19999", pSendBuffer, nSendLength);
    } while (0);

    getchar();
    zilliz::lib::NetMgr::GetInstance().CloseNetSocket();
	return 0;
}
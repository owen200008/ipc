#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/net/TcpClient.h"

int main() {
    //must init before use net
    zilliz::lib::NetMgr::GetInstance().Initialize();
    do {
        const char* pAddress = "127.0.0.1:19999";
        std::shared_ptr<zilliz::lib::TcpClient> pClient = zilliz::lib::TcpClient::CreateTcpServerSession();
        pClient->bind_connect([](zilliz::lib::NetSessionNotify* pSession)->int32_t {
            printf("Connected\n");
            return BASIC_NET_OK;
        });
        pClient->bind_rece([](zilliz::lib::NetSessionNotify* pSession, int32_t cbData, const char * pData)->int32_t {
            //pSession->Send(pData, cbData);
            return BASIC_NET_OK;
        });
        pClient->bind_disconnect([](zilliz::lib::NetSessionNotify* pSession, uint32_t dwNetCode)->int32_t {
            if (dwNetCode & BASIC_NETCODE_CLOSE_REMOTE) {
                printf("Disconnect by remote\n");
            }
            else {
                printf("Disconnect\n");
            }
            return BASIC_NET_OK;
        });
        auto retConnect = pClient->Connect(pAddress);
        if (retConnect != BASIC_NET_OK) {
            printf("connect return error %d\n", retConnect);
            break;
        }
        getchar();
        if (!pClient->Shutdown()) {
            std::chrono::milliseconds dura(1000);
            std::this_thread::sleep_for(dura);
        }
    } while (false);

    getchar();
    zilliz::lib::NetMgr::GetInstance().CloseNetSocket();
    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/net/TcpServer.h"

//func call is success
#define PrintSuccessOrFail(Func)\
{\
    char szFormat[128] = {'/'};\
    int nRetValue = sprintf(szFormat, "F:%s", #Func);\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
    if(Func())\
        nRetValue = sprintf(szFormat, "R:Success");\
    else\
        nRetValue = sprintf(szFormat, "R:Fail???");\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
}

int main(){
    //must init before use net
    zilliz::lib::NetMgr::GetInstance().Initialize();
    do {
        const char* pListen = "0.0.0.0:19999";
        auto pServer = zilliz::lib::TcpServer::CreateTcpServer();
        auto retInitServer = pServer->InitTcpServer(pListen, [](std::shared_ptr<zilliz::lib::TcpServerSession>& pSession) {
            pSession = zilliz::lib::TcpServerSession::CreateTcpServerSession();
            pSession->bind_connect([](zilliz::lib::NetSessionNotify* pSession)->int32_t {
                printf("Connect session %d\n", ((zilliz::lib::TcpServerSession*)pSession)->GetSessionID());
                return BASIC_NET_OK;
            });
            pSession->bind_rece([](zilliz::lib::NetSessionNotify* pSession, int32_t cbData, const char * pData)->int32_t {
                pSession->Send(pData, cbData);
                return BASIC_NET_OK;
            });
            pSession->bind_disconnect([](zilliz::lib::NetSessionNotify* pSession, uint32_t dwNetCode)->int32_t {
                if (dwNetCode & BASIC_NETCODE_CLOSE_REMOTE) {
                    printf("Disconnect session %d by remote\n", ((zilliz::lib::TcpServerSession*)pSession)->GetSessionID());
                }
                else {
                    printf("Disconnect session %d by local\n", ((zilliz::lib::TcpServerSession*)pSession)->GetSessionID());
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
        //wait close
        pServer->Shutdown();
    } while (false);
    
    getchar();
    zilliz::lib::NetMgr::GetInstance().CloseNetSocket();
	return 0;
}
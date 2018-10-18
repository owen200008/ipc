#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include "ZIPC/net/NetMgr.h"
#include "ZIPC/net/TcpClient.h"

int main() {
    //must init before use net
    zilliz::lib::NetMgr::GetInstance().Initialize();
    do {

    } while (false);

    getchar();
    zilliz::lib::NetMgr::GetInstance().CloseNetSocket();
    return 0;
}
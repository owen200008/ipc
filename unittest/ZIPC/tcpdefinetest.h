#pragma once

#include <stdint.h>

enum TCPTestUseCaseClientSendFirst {
    TCPTestUseCaseClientSendFirst_ConnectCloseByServer = 0,
    TCPTestUseCaseClientSendFirst_Max,
};

enum TCPTestUseCaseServerSendFirst {
    TCPTestUseCaseServerSendFirst_Data = 0,
    TCPTestUseCaseServerSendFirst_ConnectCloseByServer,
    TCPTestUseCaseServerSendFirst_ConnectSendIndexCloseByServer,
    TCPTestUseCaseServerSendFirst_Max,
};


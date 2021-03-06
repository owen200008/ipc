#include "ZIPC/net/TcpClient.h"
#include "NetThread.h"
#include "TcpThreadSocketClient.h"
#include "ZIPC/base/log/IPCLog.h"

__NS_ZILLIZ_IPC_START

bool ParseAddress(const char *ip_as_string, IPCString& strAddress, uint16_t& nPort, bool& bIPV6) {
    /* recognized formats are:
    * [ipv6]:port
    * ipv4:port
    */
    char buf[128];
    const char* cp = strchr(ip_as_string, ':');
    if (*ip_as_string == '[') {
        size_t len;
        if (!(cp = strchr(ip_as_string, ']'))) {
            return false;
        }
        len = (cp - (ip_as_string + 1));
        if (len > sizeof(buf) - 1) {
            return false;
        }
        memcpy(buf, ip_as_string + 1, len);
        buf[len] = '\0';
        strAddress = buf;
        if (cp[1] == ':')
            nPort = atoi(cp + 2);
        else
            return false;
        bIPV6 = true;
        return true;
    }
    else if (cp) {
        if (cp - ip_as_string > (int)sizeof(buf) - 1) {
            return false;
        }
        memcpy(buf, ip_as_string, cp - ip_as_string);
        buf[cp - ip_as_string] = '\0';
        strAddress = buf;
        nPort = atoi(cp + 1);
        bIPV6 = false;
        return true;
    }
    return false;
}

TcpClient::TcpClient() {

}

TcpClient::~TcpClient() {
    if (m_pSocket) {
        LogFuncLocation(IPCLog_Error, "TcpClient::~TcpClient m_pSocket exist�� must be call shutdown");
    }
}

//! must be call first
void TcpClient::InitTcpClient() {
    m_pSocket = new TcpThreadSocketClient(m_pNetThread);
    auto pClient = std::static_pointer_cast<NetSessionNotify>(shared_from_this());
    m_pSocket->InitTcpSocket(pClient);
}

TcpThreadSocket * TcpClient::GetThreadSocket(){
    return m_pSocket;
}

int32_t TcpClient::Connect(const char* lpszAddress) {
    if (lpszAddress == nullptr || lpszAddress[0] == '\0') {
        return BASIC_NET_INVALID_ADDRESS;
    }
    if (evutil_parse_sockaddr_port(lpszAddress, (sockaddr*)&m_addr, &m_addrlen) != 0) {
        return BASIC_NET_INVALID_ADDRESS;
    }
    m_bAddrSuccess = true;
    return DoConnect();
}

//! reconnect
int32_t TcpClient::DoConnect() {
    if (!m_bAddrSuccess)
        return BASIC_NET_INVALID_ADDRESS;
    uint8_t statLink = m_pSocket->GetStatLink();
    if (statLink != TIL_SS_IDLE) {
        int32_t lRet = BASIC_NET_GENERIC_ERROR;
        switch (statLink) {
        case TIL_SS_CONNECTING: {
            lRet = BASIC_NET_CONNECTING_ERROR;
            break;
        }
        case TIL_SS_CONNECTED: {
            lRet = BASIC_NET_ALREADY_CONNECT;
            break;
        }
        }
        return lRet;
    }
    int32_t lReturn = BASIC_NET_OK;
    evutil_socket_t socketfd = INVALID_SOCKET;
    do {
        evutil_socket_t socketfd = socket(m_addr.ss_family, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET) {
            lReturn = BASIC_NET_SOCKET_ERROR;
            break;
        }
        evutil_make_socket_nonblocking(socketfd);
        evutil_make_listen_socket_reuseable(socketfd);
        evutil_make_listen_socket_reuseable_port(socketfd);
        int nRet = connect(socketfd, (::sockaddr*)&m_addr, m_addrlen);
        if (nRet == 0) {
            m_pNetThread->SetEvent(shared_from_this(), [this, socketfd]() {
                auto p = (TcpThreadSocketClient*)m_pSocket;
                p->ConnectSuccess(socketfd);
            });
        }
#ifdef _MSC_VER
        else if (errno == EINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        else if (errno == EINPROGRESS) {
#endif

            m_pNetThread->SetEvent(shared_from_this(), [this, socketfd]() {
                auto p = (TcpThreadSocketClient*)m_pSocket;
                p->ConnectWaitEvent(socketfd);
            });
        }
        else {
            //int nRetErrorNo = errno;
            lReturn = BASIC_NET_GENERIC_ERROR;
        }
    } while (0);

    if (lReturn != BASIC_NET_OK && socketfd != INVALID_SOCKET) {
        evutil_closesocket(socketfd);
    }
    return BASIC_NET_OK;
}

//! shutdown
void TcpClient::Shutdown() {
    m_pNetThread->SetEvent(shared_from_this(), [this]() {
        if (m_pSocket) {
            m_pSocket->Shutdown();
            delete m_pSocket;
            m_pSocket = nullptr;
        }
    });
}

//!
bool TcpClient::IsShutdown() {
    return m_pSocket == nullptr;
}

__NS_ZILLIZ_IPC_END

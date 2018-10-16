#include "ZIPC/net/TcpClient.h"
#include "TcpSocket.h"

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

NetClient::NetClient() {

}

NetClient::~NetClient() {

}

int32_t NetClient::Connect(const char* lpszAddress) {
    if (lpszAddress == nullptr || lpszAddress[0] == '\0') {
        return BASIC_NET_INVALID_ADDRESS;
    }
    if (evutil_parse_sockaddr_port(lpszAddress, (sockaddr*)&m_addr, &m_addrlen) != 0) {
        return BASIC_NET_INVALID_ADDRESS;
    }
    m_bAddrSuccess = true;
    return DoConnect();
}

struct ConnectRevert {
    
    evutil_socket_t         m_socketfd;
};
//! reconnect
int32_t NetClient::DoConnect() {
    if (m_pSocket)
        return BASIC_NET_ALREADY_CONNECT;
    if (!m_bAddrSuccess)
        return BASIC_NET_INVALID_ADDRESS;
    std::promise<int32_t> promiseObj;
    auto future = promiseObj.get_future();
    
    GotoNetThread([](std::shared_ptr<NetSessionNotify>& pSession, TcpSocket* pSocket, intptr_t lRevert) {
        auto p = (NetClient*)pSession.get();
        std::promise<int32_t>* pRevert = (std::promise<int32_t>*)lRevert;
        pRevert->set_value(TcpSocket::AssignConnect(pSession, p->GetAddr(), p->GetAddrLen()));
    }, (intptr_t)&promiseObj);
    return future.get();
}

__NS_ZILLIZ_IPC_END
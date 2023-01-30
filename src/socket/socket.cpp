#include "socket/socket.h"

SocketWrapper::SocketWrapper(const std::string &ip, int port) {

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Couldn't open socket");
    }

    struct sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    int res =
        bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
    if (res != 0) {
        close(sock);
        throw std::runtime_error("Failed to bind UDP socket on 127.0.0.1:6000");
    }

    tv.tv_sec = timeout_seconds;
    tv.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
               reinterpret_cast<const char *>(&rcvBufSize), sizeof(rcvBufSize));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char *>(&tv), sizeof(tv));
}

std::shared_ptr<char[]> SocketWrapper::recvBuffer(int &len) {
    char *buffer = new char[BUFFER_SIZE];
    int res = recv(sock, buffer, BUFFER_SIZE, 0);
    if (res < 0) {
        throw std::runtime_error("Error in reading from socket");
    }

    len = res;

    return std::shared_ptr<char[]>(buffer);
}

SocketWrapper::~SocketWrapper() { close(sock); }


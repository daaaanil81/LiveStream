#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <string>

class SocketWrapper {
  private:
    const int rcvBufSize = 212992;
    const int BUFFER_SIZE = 2048;
    int sock;

  public:
    SocketWrapper(const std::string &ip, int port);
    ~SocketWrapper();
    std::shared_ptr<char[]> recvBuffer(int &len);
};

#endif /* __SOCKET_H__ */

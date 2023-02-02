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

/**
 * Class Wrapper for linux socket.
 */
class SocketWrapper {
  private:
    //! Socket buffer size.
    const int rcvBufSize = 212992;
    //! Size of buffer for reading from socket.
    const int BUFFER_SIZE = 2048;
    //! Count seconds for timeout.
    const long timeout_seconds = 5;
    //! Structure timeout for reading from socket.
    struct timeval tv;
    //! Socket descriptor.
    int sock;

  public:
    //! Create wrapper.
    /*!
     * Open socket. bind socket.
     * \param image image in bgr24 pixel format.
     * \return avframe in yuv420p pixel format.
     */
    SocketWrapper(const std::string &ip, int port);
    //! Destructor.
    /*!
     * Close socket descriptor.
     */
    ~SocketWrapper();
    //! Receive buffer from socket.
    /*!
     * \param len Set len after reading.
     * \return Buffer with data.
     */
    std::shared_ptr<char[]> recvBuffer(int &len);
};

#endif /* __SOCKET_H__ */

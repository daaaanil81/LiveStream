#pragma once

#include <iostream>
#include <string>
#include <csignal>
#include <stdio.h>
#include <exception>
#include <unistd.h>
#include <map>
#include <ctime>
#include <random>
#include <cstdint>
#include <netinet/in.h>

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/sha.h"
#include "base64.h"

#include "ace/Get_Opt.h"
#include "ace/SOCK_IO.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/Thread_Manager.h"
#include "ace/Thread.h"
#include "ace/Time_Value_T.h"
#include "ace/FILE_IO.h"
#include "ace/Message_Block.h"
#include "ace/CDR_Stream.h"
#include "ace/ACE.h"
#include "ace/Log_Record.h"


#define MAX_LEN_IPV4 16
#define LOG_DEBUG      "\033[1;32mDEBUG\033[0m"
#define LOG_ERROR      "\033[1;31mERROR\033[0m"
#define LOG_THREAD     "\033[1;33mTHREAD\033[0m"



class ACE_Message_Block;

class ACE_SSL_CTX
{
public:
    explicit ACE_SSL_CTX(const char* cert, const char* key);
    ACE_SSL_CTX(const ACE_SSL_CTX& ssl_ctx);
    ~ACE_SSL_CTX();
    SSL_CTX* getCTX() { return ctx_; } 
private:
    SSL_CTX* ctx_;
};

class ACE_SSL_SOCK_Stream : public ACE_SOCK_Stream
{
public:
    /// Constructor
    ACE_SSL_SOCK_Stream(void);
    
    /// Constructor with handle
    ACE_SSL_SOCK_Stream(ACE_HANDLE h);
    
    /// Construct copy
    ACE_SSL_SOCK_Stream(const ACE_SSL_SOCK_Stream& ssl_stream);
    
    /// Message connection
    void MessageConnection(SSL_CTX* ctx);
    
    /// Receive len bytes from opened socket 
    int recv_n (char* message, size_t len);
    
    /// Send len bytes
    int send_n (const char* message, size_t len);

    /// Destructor
    ~ACE_SSL_SOCK_Stream(void);
    
        
private:
    SSL *ssl_;
};



static char message_connect[] = "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %s\r\n\r\n";

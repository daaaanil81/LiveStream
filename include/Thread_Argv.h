#pragma once
#include <iostream>
#include <mutex>

#include "ace/SOCK_Stream.h"
#include "ace/Thread_Manager.h"
#include "ws-server.h"
#include "externfile.h"

static std::mutex g_mutex;

class Thread_Argv
{
public:
    explicit Thread_Argv(ACE_SSL_SOCK_Stream* ssl_stream, SSL_CTX* ctx);
    ~Thread_Argv();
    enum class SOCK_EVENT {HOST, SDP, ICE};
    static void* thread_connection(void* argv);
    ACE_SSL_SOCK_Stream* ssl_stream_;
    SSL_CTX* ctx_;
};


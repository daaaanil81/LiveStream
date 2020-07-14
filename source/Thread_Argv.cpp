#include "Thread_Argv.h"

Thread_Argv::Thread_Argv(ACE_SSL_SOCK_Stream* ssl_stream, SSL_CTX* ctx) : ssl_stream_(ssl_stream), ctx_(ctx)
{
    ACE_DEBUG((LM_DEBUG, "%s:Construct Thread arguments.\n", LOG_DEBUG));
}

Thread_Argv::~Thread_Argv()
{
    ACE_DEBUG((LM_DEBUG, "%s:Destructor Thread arguments.\n", LOG_DEBUG));
}

bool Thread_Argv::wait(int fd, long seconds)
{
    if (fd < 0) {
        return false;
    }

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(fd, &fdSet);
    
    timeval tv = { seconds, 0 };
    int rc = select(fd + 1, &fdSet, NULL, NULL, &tv);

    return (rc != 0);
}

void* Thread_Argv::thread_connection(void* argv)
{
    Thread_Argv* arg = static_cast<Thread_Argv*>(argv);
    ACE_thread_t id = ACE_Thread::self();
    ACE_SSL_SOCK_Stream *ssl_stream = arg->ssl_stream_;
    SSL_CTX* ctx = arg->ctx_;
    try
    {
        ssl_stream->MessageConnection(ctx);
        char message[4096] = {0};
        Thread_Argv::wait(ssl_stream->get_handle(), 3);
        ssl_stream->send_n("Con", ACE_OS::strlen("Con"));
        ssl_stream->recv_n(message, sizeof(message));
        
        // while (thread_ids.at(id))
        // {
        //     continue;
        // }
    }
    catch (std::logic_error e)
    {
        ACE_DEBUG((LM_ERROR, "%s::%u::%s:%s\n", LOG_THREAD, id, LOG_ERROR, e.what()));
    }
    
    g_mutex.lock();
    thread_ids.erase(id);
    g_mutex.unlock();

    ssl_stream->close();
    delete ssl_stream;
    delete arg;
    ACE_DEBUG((LM_DEBUG, "%s::%u::%s: Free memory in thread.\n", LOG_THREAD, id, LOG_DEBUG));
    return 0;
}

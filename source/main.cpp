#include "Thread_Argv.h"
#include "ws-server.h"
#include "externfile.h"
static bool finish = false;

void signal_int(int signal)
{
    ACE_DEBUG((LM_DEBUG, "%s:Signal finish.\n", LOG_DEBUG));
    finish = true;
}

int main (int argc, char* argv[])
{
    static const ACE_TCHAR options[] = ACE_TEXT ("h:d");
    ACE_OS::signal(SIGINT, signal_int);
    ACE_Get_Opt cmd_opts (argc, argv, options);
    if (cmd_opts.long_option(ACE_TEXT ("host"), 'h', ACE_Get_Opt::ARG_REQUIRED) == -1) // Same options --host and -h and only one arguments after their
        return -1;
    int option = 0;
    bool flag_parce = false;
    char ip_v4[MAX_LEN_IPV4] = {0};
    while ((option = cmd_opts ()) != EOF)
    {
        switch (option)
        {
            case 'h':
            {                
                ACE_OS_String::strncpy(ip_v4, cmd_opts.opt_arg(), MAX_LEN_IPV4);
                flag_parce = true;
                break;
            }
            case 'd':
            {
                DEBUG = true;
                break;
            }
            default:
            {
                ACE_DEBUG((LM_ERROR, "%s:Parse error.\n", LOG_ERROR));
                return 1;
            }
        }
    }
    if (!flag_parce)
    {
        ACE_DEBUG((LM_ERROR, "%s:Enter please host name.\n", LOG_ERROR));
        return 1;
    }
    ACE_OS::signal(SIGINT,signal_int);
    try
    {
        ACE_DEBUG((LM_DEBUG, "%s:Start ...\n", LOG_DEBUG));
        ACE_INET_Addr server_addr;
        ACE_SOCK_Acceptor acceptor;
        ACE_SSL_CTX ssl_ctx("../certificate/danil_petrov.crt", "../certificate/danil_petrov.key");
        ACE_Time_Value t(0, 0);
        if (server_addr.set(8080) == -1) return 1;
        if (acceptor.open(server_addr) == -1) return 1;
        while(!finish)
        {
            ACE_SSL_SOCK_Stream* ssl_peer = new ACE_SSL_SOCK_Stream;
            if (acceptor.accept(*ssl_peer, 0, &t) == -1)
            {
                delete ssl_peer;
                continue;
            }
            ACE_DEBUG((LM_DEBUG, "%s: Connect new client .......\n", LOG_DEBUG));
            ACE_thread_t id;
            Thread_Argv* arg = new Thread_Argv(ssl_peer, ssl_ctx.getCTX());            
            ACE_Thread::spawn(Thread_Argv::thread_connection, static_cast<void*>(arg), THR_DETACHED | THR_SCOPE_SYSTEM, &id);
            ACE_DEBUG((LM_DEBUG, "%s::%u::%s: Create thread ....\n", LOG_THREAD, id, LOG_DEBUG));
            thread_ids[id] = true;
        }
        for (auto& id : thread_ids)
        {
            id.second = false;
            //ACE_Thread::join(id.first, 0, 0);
        }
    }
    catch (std::logic_error e)
    {
        ACE_DEBUG((LM_ERROR, "%s:%s\n", LOG_ERROR, e.what()));
    }
    
    return 0;
}

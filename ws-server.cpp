#include "ws-server.h"
#include <csignal>
#include <libwebsockets.h>

struct lws_context *context;

static int callback_wss_server(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
        case LWS_CALLBACK_PROTOCOL_INIT: /// Initial protocols
        {
            printf("Initialization\n");
            break;
        }
        
        case LWS_CALLBACK_ESTABLISHED: /// Connection clients
        {
            printf("Connection established\n");   
            break;
        }
        
        case LWS_CALLBACK_RECEIVE: // LWS_CALLBACK_RECEIVE
        {
            printf("Receive");
            break;
        }
        
        case LWS_CALLBACK_CLOSED: /// Close server
        {
            printf("Close\n");
            break;
        }
        
        default:
        {
            break;
        }
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {"lws-minimal", callback_wss_server, 0, 0, 0, NULL, 0},
    {NULL, NULL, 0, 0} /* terminator*/
};

int main (int argc, char* argv[])
{
    static const ACE_TCHAR options[] = ACE_TEXT ("h:");
    ACE_Get_Opt cmd_opts (argc, argv, options);
    if (cmd_opts.long_option(ACE_TEXT ("host"), 'h', ACE_Get_Opt::ARG_REQUIRED) == -1) // Same options --config and -f and only one arguments after their
        return -1;
    int option = 0;
    bool flag_parce = false;
    char ip_v4[MAX_LEN_IPV4] = {0};
    struct lws_context_creation_info info;

    
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
            default:
            {
                ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("Parse error.\n")), -1);
                break;
            }
        }
    }

    if (!flag_parce)
        ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("Enter please host name.\n")), -1);

    std::string log_ip = "WebSocket security: http://" + ip_v4 + ":9999"; 
    LOG(LOG_INFO, log_ip.c_str());
    LOG(LOG_INFO, "Enter Ctrl + C for exit.");
    memset(&info, 0, sizeof info);
    info.port = 9999; /// Server port
    info.mounts = NULL;
    info.protocols = protocols;
    info.vhost_name = ip_server_program; /// Server ip
    info.ws_ping_pong_interval = 10;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    lwsl_user("Server using TLS\n");
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.ssl_cert_filepath = "./certificate/danil_petrov.cert";
    info.ssl_private_key_filepath = "./certificate/danil_petrov.key";

    context = lws_create_context(&info);
    if (!context)
    {
        lwsl_err("lws init failed\n");
        return 1;
    }

    while (n >= 0 && !interrupted)
        n = lws_service(context, 0);

    lws_context_destroy(context);
    //LOG(LOG_DEBUG, "Test logging system for LOG_DEBUG");
    //LOG(LOG_INFO, "Test logging system for LOG_INFO");
    //LOG(LOG_NOTICE, "Test logging system for LOG_NOTICE");
    //LOG(LOG_WARNING, "Test logging system for LOG_WARNING");
    //LOG(LOG_ERROR, "Test logging system for LOG_ERROR");
    return 0;
}

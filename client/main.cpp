#include <ace/CDR_Stream.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Record.h>
#include <ace/OS.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/streams.h>
#include <string>

class Logging_Client {
  private:
    ACE_SOCK_Stream logging_peer_;

  public:
    int send(const ACE_Log_Record &log_record);
    ACE_SOCK_Stream &peer() { return logging_peer_; }
    ~Logging_Client() { logging_peer_.close(); }
};

int Logging_Client::send(const ACE_Log_Record &log_record) {
    const size_t max_payload_size = 4   // Type()
                                    + 8 // Timestamp
                                    + 4 // id process
                                    + 4 // length of data
                                    + ACE_Log_Record::MAXLOGMSGLEN // Data
                                    + ACE_CDR::MAX_ALIGNMENT;

    ACE_OutputCDR payload(max_payload_size);
    payload << log_record;
    ACE_CDR::ULong length = payload.total_length();

    ACE_OutputCDR header(ACE_CDR::MAX_ALIGNMENT + 8);
    header << ACE_OutputCDR::from_boolean(ACE_CDR_BYTE_ORDER);
    header << ACE_CDR::ULong(length);

    iovec iov[2];

    iov[0].iov_base = header.begin()->rd_ptr();
    iov[0].iov_len = 8;
    iov[1].iov_base = payload.begin()->rd_ptr();
    iov[1].iov_len = length;

    return logging_peer_.sendv_n(iov, 2);
}

int main(int argc, char *argv[]) {
    u_short logger_port = argc > 1 ? std::atoi(argv[1]) : 0;
    const char *logger_host = argc > 2 ? argv[2] : ACE_DEFAULT_SERVER_HOST;
    int result;

    ACE_INET_Addr server_addr;

    if (logger_port != 0) {
        result = server_addr.set(logger_port, logger_host);
    } else {
        result = server_addr.set("ace_logger", logger_host);
    }

    if (result == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "look up %N:%I %s %p\n",
                          logger_port == 0 ? "ace_logger" : argv[1],
                          logger_host),
                         1);
    }

    ACE_SOCK_Connector connector;
    Logging_Client client;

    if (connector.connect(client.peer(), server_addr) < 0) {
        ACE_ERROR_RETURN((LM_ERROR, "%p\n", "connect()"), 1);
    }

    std::cin.width(ACE_Log_Record::MAXLOGMSGLEN);
    for (;;) {
        std::string user_input;
        getline(std::cin, user_input, '\n');
        if (!std::cin || std::cin.eof()) {
            break;
        }

        ACE_Time_Value now(ACE_OS::gettimeofday());
        ACE_Log_Record log_record(LM_INFO, now, ACE_OS::getpid());
        log_record.msg_data(user_input.c_str());

        if (client.send(log_record) == -1) {
            ACE_ERROR_RETURN((LM_ERROR, "%p\n", "client send"), 1);
        }
    }
    return 0;
}


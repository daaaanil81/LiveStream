#ifndef __LOGGING_SERVER__
#define __LOGGING_SERVER__

#include <ace/FILE_IO.h>
#include <ace/SOCK_Acceptor.h>

class ACE_SOCK_Stream;

class Logging_Server {
  public:
    virtual int run(int argc, char *argv[]);

  protected:
    virtual int open(u_short port = 0);
    virtual int wait_for_multiple_events() { return 0; }
    virtual int handle_connections() = 0;
    virtual int handle_data(ACE_SOCK_Stream * = nullptr) = 0;

    int make_log_file(ACE_FILE_IO &, ACE_SOCK_Stream * = nullptr);

    virtual ~Logging_Server() { acceptor_.close(); }

    ACE_SOCK_Acceptor &acceptor() { return acceptor_; }

  private:
    ACE_SOCK_Acceptor acceptor_;
};

#endif /* __LOGGING_SERVER__ */

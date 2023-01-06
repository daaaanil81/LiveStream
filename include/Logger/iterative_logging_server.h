#ifndef __ITERATIVE_LOGGING_SERVER__
#define __ITERATIVE_LOGGING_SERVER__

#include <ace/FILE_IO.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>

#include "Logger/logging_handler.h"
#include "Logger/logging_server.h"

class Iterative_Logging_Server : public Logging_Server {
  protected:
    ACE_FILE_IO log_file_;
    Logging_Handler logging_handler_;

  public:
    Iterative_Logging_Server() : logging_handler_(log_file_) {}
    Logging_Handler &logging_handler() { return logging_handler_; }

    virtual ~Iterative_Logging_Server() { log_file_.close(); }

  protected:
    virtual int open(u_short port) override;
    virtual int handle_connections() override;
    virtual int handle_data(ACE_SOCK_Stream * = nullptr) override;
};

#endif /* __ITERATIVE_LOGGING_SERVER__ */


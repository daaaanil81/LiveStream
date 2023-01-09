#ifndef __REACTIVE_LOGGING_SERVER__
#define __REACTIVE_LOGGING_SERVER__

#include "Logger/iterative_logging_server.h"
#include <ace/Handle_Set.h>

class Reactive_Logging_Server : public Iterative_Logging_Server {
  protected:
    // Following all desctiptors
    ACE_Handle_Set master_handle_set_;

    // Following descriptors after <select>
    ACE_Handle_Set active_handles_;

  public:
    Reactive_Logging_Server() = default;
    virtual ~Reactive_Logging_Server() = default;

  protected:
    virtual int open(u_short port) override;
    virtual int wait_for_multiple_events() override;
    virtual int handle_connections() override;
    virtual int handle_data(ACE_SOCK_Stream * = nullptr) override;
};

#endif /* __REACTIVE_LOGGING_SERVER__ */

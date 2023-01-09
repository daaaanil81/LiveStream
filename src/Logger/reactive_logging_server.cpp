#include "Logger/reactive_logging_server.h"

int Reactive_Logging_Server::open(u_short port) {
    Iterative_Logging_Server::open(port);
    master_handle_set_.set_bit(acceptor().get_handle());
    acceptor().enable(ACE_NONBLOCK);
    return 0;
}

int Reactive_Logging_Server::wait_for_multiple_events() {

    active_handles_ = master_handle_set_;
    int width = (int)active_handles_.max_set() + 1;
    if (select(width, active_handles_.fdset(), 0, 0, 0) == -1) {
        return -1;
    }

    active_handles_.sync((ACE_HANDLE)active_handles_.max_set() + 1);

    return 0;
}

int Reactive_Logging_Server::handle_connections() {
    if (active_handles_.is_set(acceptor().get_handle())) {
        while (acceptor().accept(logging_handler().peer()) == 0) {
            master_handle_set_.set_bit(logging_handler().peer().get_handle());
        }
        active_handles_.clr_bit(acceptor().get_handle());
    }
    return 0;
}

int Reactive_Logging_Server::handle_data(ACE_SOCK_Stream *) {
    ACE_Handle_Set_Iterator peer_iterator(active_handles_);
    for (ACE_HANDLE handle; (handle = peer_iterator()) != ACE_INVALID_HANDLE;) {
        logging_handler().peer().set_handle(handle);

        if (logging_handler().log_record() == -1) {
            master_handle_set_.clr_bit(handle);
            logging_handler().close();
        }
    }
    return 0;
}

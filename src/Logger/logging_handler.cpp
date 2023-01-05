#include "Logger/logging_handler.h"
#include <ace/CDR_Base.h>
#include <ace/CDR_Stream.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Record.h>
#include <ace/Message_Block.h>
#include <iostream>

struct Message_Block_Deleter {
    void operator()(ACE_Message_Block *ptr) {
        ptr->release();
        delete ptr;
    }
};

int Logging_Handler::recv_log_record(ACE_Message_Block *&mblk) {

    ACE_INET_Addr peer_addr;
    logging_peer_.get_remote_addr(peer_addr);
    mblk = new ACE_Message_Block(MAXHOSTNAMELEN + 1);
    peer_addr.get_host_name(mblk->wr_ptr(), MAXHOSTNAMELEN);
    mblk->wr_ptr(strlen(mblk->wr_ptr()) + 1);

    ACE_Message_Block *payload = new ACE_Message_Block(ACE_DEFAULT_CDR_BUFSIZE);
    ACE_CDR::mb_align(payload);

    if (logging_peer_.recv_n(payload->wr_ptr(), 8) == 8) {
        payload->wr_ptr(8);

        ACE_InputCDR cdr(payload);
        ACE_CDR::Boolean byte_order;

        cdr >> ACE_InputCDR::to_boolean(byte_order);
        cdr.reset_byte_order(byte_order);

        ACE_CDR::ULong length;

        cdr >> length;

        payload->size(length + 8);

        if (logging_peer_.recv_n(payload->wr_ptr(), length) > 0) {
            payload->wr_ptr(length);

            mblk->cont(payload);
            return length;
        }
    }
    payload->release();
    mblk->release();
    return -1;
}

int Logging_Handler::write_log_record(ACE_Message_Block *mblk) {
    if (log_file_.send_n(mblk) == -1) {
        return -1;
    }

    if (ACE::debug()) {
        ACE_InputCDR cdr(mblk->cont());

        ACE_CDR::Boolean byte_order;
        ACE_CDR::ULong length;

        cdr >> ACE_InputCDR::to_boolean(byte_order);
        cdr.reset_byte_order(byte_order);

        cdr >> length;

        ACE_Log_Record log_record;

        cdr >> log_record;

        log_record.print(mblk->rd_ptr(), 1, std::cerr);
    }

    return mblk->total_length();
}

int Logging_Handler::log_record() {
    ACE_Message_Block *mblk = nullptr;

    if (recv_log_record(mblk) == -1) {
        return -1;
    }

    int result = write_log_record(mblk);
    mblk->release();
    return result == -1 ? -1 : 0;
}

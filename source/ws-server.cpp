#include "ws-server.h"
#include "externfile.h"
ACE_SSL_CTX::ACE_SSL_CTX(const char* cert, const char* key)
{
    ctx_ = nullptr;
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:Construct SSL_CTX.\n", LOG_DEBUG));
    SSL_library_init();
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    const SSL_METHOD *method = SSLv23_server_method();  /* create new server-method instance */
    ctx_ = SSL_CTX_new(method);   /* create new context from method */

    if (ctx_ == nullptr)
        throw std::logic_error("Error with ctx.");
    
    if (SSL_CTX_use_certificate_file(ctx_, cert, SSL_FILETYPE_PEM) <= 0)
        throw std::logic_error("Error with certificate.");
    
    if (SSL_CTX_use_PrivateKey_file(ctx_, key, SSL_FILETYPE_PEM) <= 0 )
        throw std::logic_error("Error with private key.");
    
    if (!SSL_CTX_check_private_key(ctx_))
        throw std::logic_error("Private key does not match the public certificate.");
}

ACE_SSL_CTX::~ACE_SSL_CTX()
{
    SSL_CTX_free(ctx_);
    EVP_cleanup();
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:Destruct SSL_CTX.\n", LOG_DEBUG));
}

ACE_SSL_CTX::ACE_SSL_CTX(const ACE_SSL_CTX& ssl_ctx)
{
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:Construct copy SSL CTX .\n", LOG_DEBUG));
    this->ctx_ = ssl_ctx.ctx_;
}


ACE_SSL_SOCK_Stream::ACE_SSL_SOCK_Stream(void)
{
    ssl_ = nullptr;
    // if (DEBUG)
    //     ACE_DEBUG((LM_DEBUG, "%s:Construct SSL Stream.\n", LOG_DEBUG));
}

ACE_SSL_SOCK_Stream::ACE_SSL_SOCK_Stream(ACE_HANDLE h)
{
    this->set_handle(h);
    ssl_ = nullptr;
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:Construct with arguments SSL Stream.\n", LOG_DEBUG));
}

ACE_SSL_SOCK_Stream::ACE_SSL_SOCK_Stream(const ACE_SSL_SOCK_Stream& ssl_stream)
{
    this->set_handle(ssl_stream.get_handle());
    this->ssl_ = ssl_stream.ssl_;
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:Construct copy SSL Stream.\n", LOG_DEBUG));
}

ACE_SSL_SOCK_Stream::~ACE_SSL_SOCK_Stream(void)
{
    //if (DEBUG)
    //ACE_DEBUG((LM_DEBUG, "%s:Destruct SSL Stream.\n", LOG_DEBUG));
    if (ssl_ != nullptr)
    {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        if (DEBUG)
            ACE_DEBUG((LM_DEBUG, "%s:Free SSL Stream.\n", LOG_DEBUG));
    }
}

/// Message connection
void ACE_SSL_SOCK_Stream::MessageConnection(SSL_CTX* ctx)
{
    char request[1024] = {0};
    char response[1024] = {0};
    char result_str[64] = {0};
    unsigned char outbuf[20];
    unsigned char key_out[64] = {0};
    ACE_thread_t id = ACE_Thread::self();
    char* start;
    int bytes = 0;
    char GUIDKey[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // 36
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:MessageConnection.\n", LOG_DEBUG));
    if (this->get_handle () == ACE_INVALID_HANDLE)
        throw std::logic_error("Error with Handle");
    ssl_ = SSL_new(ctx);
    
    SSL_set_fd(ssl_, this->get_handle());
    if (SSL_accept(ssl_) == 0)
    {
        long error = ERR_get_error();
        const char* error_str = ERR_error_string(error, NULL);
        ACE_DEBUG((LM_ERROR, "%s:%s.\n", LOG_ERROR, error_str));
        throw std::logic_error("Error with ssl_accept.");
    }
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s:OK.\n", LOG_DEBUG));
    bytes = SSL_read(ssl_, request, sizeof(request)); /* get request */
    if (bytes <= 0)
        throw std::logic_error("Error with read request.");
    request[bytes] = '\0';
    
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s::%u::%s\n%s", LOG_THREAD, id, LOG_DEBUG, request));
    
    if((start = ACE_OS::strstr(request, "Sec-WebSocket-Key:")) != NULL)
    {                                                  
        uint16_t i = 0, it = 0;
        for(i = 19; it < 24; i++, it++)
            result_str[it] = start[i];
    }
    else
    {
        throw std::logic_error("Error with Sec-WebSocket-Key.");
    }
    
    ACE_OS::strcat(result_str, GUIDKey);
    
    SHA1(reinterpret_cast<unsigned char*>(result_str), ACE_OS::strlen(result_str), outbuf);
    base64_encode2(outbuf, key_out, 20);
   
    ACE_OS::sprintf(response, message_connect, key_out);
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s::%u::%s\n%s", LOG_THREAD, id, LOG_DEBUG, response));        
    SSL_write(ssl_, response, ACE_OS::strlen(response));
}

/// Receive len bytes from opened sock
int ACE_SSL_SOCK_Stream::recv_n(char *message, size_t len)
{
    ACE_thread_t id = ACE_Thread::self();
    uint8_t fin, opcode, bit_mask;
    ACE_CDR::UShort length;
    ACE_CDR::Char mask[4] = {0};
    ACE_CDR::Char first_byte, second_byte;
    ACE_CDR::Char letter;
    ACE_Message_Block* payload = new ACE_Message_Block(len);
    ACE_CDR::mb_align(payload);
    size_t l = SSL_read(ssl_, payload->wr_ptr(), len);
    payload->wr_ptr(l+1);
    if (payload->length() <= 0)
        throw std::logic_error("Error with message block.");
    
    ACE_InputCDR cdr(payload);
    cdr.reset_byte_order(0);
    
    cdr >> first_byte;
    cdr >> second_byte;
    
    fin = (static_cast<unsigned char>(first_byte) & 0x80 ) >> 7;
    opcode = static_cast<unsigned char>(first_byte) & 0x0F;
    bit_mask = (static_cast<unsigned char>(second_byte) & 0x80) >> 7;
    length = static_cast<unsigned char>(second_byte) & 0x7F;
    
        
    if (length < 126)
    {
        cdr.read_char_array(mask, 4);
        for (int i = 0; i < length; i++)
        {
            cdr >> letter;
            message[i] = letter^mask[i % 4];
        }
    }
    else if (length == 126)
    {
        ACE_CDR::UShort new_length;
        cdr >> new_length;
        cdr.read_char_array(mask, 4);
        for (int i = 0; i < new_length; i++)
        {
            cdr >> letter;
            message[i] = letter^mask[i % 4];
        }
        length = new_length;
    }
    
    if (DEBUG)
    {
        ACE_DEBUG((LM_DEBUG, "%s::%u::%s\n\tFin = %u\n\tOpcode = 0x%02x\n\tMask = %u\n\tLength = %u\n", LOG_THREAD, id, LOG_DEBUG, fin, opcode, bit_mask, length));
        ACE_DEBUG((LM_DEBUG, "%s::%u::%s\nMessage = %s\n", LOG_THREAD, id, LOG_DEBUG, message));
    }
    payload->release();
    payload = nullptr;
    return l; 
}

/// Send len bytes
int ACE_SSL_SOCK_Stream::send_n(const char* message, size_t len)
{
    ACE_thread_t id = ACE_Thread::self();
    ACE_OutputCDR header(len + 4);
    uint32_t length = ACE_OS::strlen(message);
    uint8_t code = 0;
    uint8_t fin = 129;
    size_t l = 0;
    header << ACE_CDR::Char(fin); // 1000 0001
    if (length < 126)
    {
        code = length;
        l += 2;
    }
    else if (length > 125 && length < 65536)
    {
        code = 126;
        l += 4;
    }
    header << ACE_CDR::Char(code);
    if (code == 126)
        header << ACE_CDR::UShort(htons(len));
    header.write_char_array(message, length);
    l = SSL_write(ssl_, header.begin()->rd_ptr(), header.length()) - l;
    if (l <= 0)
        throw std::logic_error("Error with write message.");    
    if (DEBUG)
        ACE_DEBUG((LM_DEBUG, "%s::%u::%s\n\tLength = %u\n\tLen = %u\n", LOG_THREAD, id, LOG_DEBUG, l, len));
    return l;
}


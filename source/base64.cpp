
#include "base64.h"

char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};


size_t base64_encode(char* in, char* out) {

    uint8_t counts = 0;
    char buffer[3];
    //char* out = (char*)malloc(strlen(in) * 4 / 3 + 4);

    size_t i = 0, c = 0;
    for(i = 0; in[i] != '\0'; i++) {
        buffer[counts++] = in[i];
        if(counts == 3) {
            out[c++] = base46_map[buffer[0] >> 2];
            out[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            out[c++] = base46_map[((buffer[1] & 0x0f) << 2) + (buffer[2] >> 6)];
            out[c++] = base46_map[buffer[2] & 0x3f];
            counts = 0;
        }
    }

    if(counts > 0) {
        out[c++] = base46_map[buffer[0] >> 2];
        if(counts == 1) {
            out[c++] = base46_map[(buffer[0] & 0x03) << 4];
            out[c++] = '=';
        } else {                      // if counts == 2
            out[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            out[c++] = base46_map[(buffer[1] & 0x0f) << 2];
        }
        out[c++] = '=';
    }

    out[c] = '\0';   /* string padding character */
    return c;
}


size_t base64_decode(char* in, char* out) {

    uint8_t counts = 0;
    char buffer[4];
    size_t i = 0, p = 0;
    for(i = 0; in[i] != '\0'; i++) {
        uint8_t k;
        for(k = 0 ; k < 64 && base46_map[k] != in[i]; k++);
        buffer[counts++] = k;
        if(counts == 4) {
            out[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if(buffer[2] != 64)
                out[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if(buffer[3] != 64)
                out[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }
    out[p] = '\0';    /* string padding character */
    return p;
}

int base64_encode2(unsigned char* in, unsigned char* out, int len)
{
    int idx, idx2, blks, left_over;
    
    blks = (len / 3) * 3;
    for(idx=0, idx2=0; idx < blks; idx += 3, idx2 += 4) 
    {
        out[idx2] = base46_map[in[idx] >> 2];
        out[idx2+1] = base46_map[((in[idx] & 0x03) << 4) + (in[idx+1] >> 4)];
        out[idx2+2] = base46_map[((in[idx+1] & 0x0f) << 2) + (in[idx+2] >> 6)];
        out[idx2+3] = base46_map[in[idx+2] & 0x3F];
    }

    left_over = len % 3;
    
    if(left_over == 1) 
    {
        out[idx2] = base46_map[in[idx] >> 2];
        out[idx2+1] = base46_map[(in[idx] & 0x03) << 4];
        out[idx2+2] = '=';
        out[idx2+3] = '=';
        idx2 += 4;
    }
    
    else if(left_over == 2) 
    {
        out[idx2] = base46_map[in[idx] >> 2];
        out[idx2+1] = base46_map[((in[idx] & 0x03) << 4) + (in[idx+1] >> 4)];
        out[idx2+2] = base46_map[(in[idx+1] & 0x0F) << 2];
        out[idx2+3] = '=';
        idx2 += 4;
    }
    
    out[idx2] = 0;
    return idx2;
} 

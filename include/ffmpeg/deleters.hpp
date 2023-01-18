#ifndef __DELETERS__
#define __DELETERS__

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <iostream>

struct AVFormatContext_Deleter {
    void operator()(AVFormatContext *ptr) {
        if (ptr != nullptr) {
            std::cout << "Call AVFormatContext_Deleter" << std::endl;
            avformat_close_input(&ptr);
            avformat_free_context(ptr);
        }
    }
};

struct AVCodecContext_Deleter {
    void operator()(AVCodecContext *ptr) {
        if (ptr != nullptr) {
            std::cout << "Call AVCodecContext_Deleter" << std::endl;
            avcodec_free_context(&ptr);
        }
    }
};

struct AVPacket_Deleter {
    void operator()(AVPacket *ptr) {
        if (ptr != nullptr) {
            std::cout << "Call AVPacket_Deleter" << std::endl;
            av_packet_free(&ptr);
        }
    }
};

struct timebase_t {
    int numerator;
    int denominator;
};

struct stream_desc_t {
    int64_t bit_rate;
    int width;
    int height;
    int channels;
    int sample_rate;
    timebase_t timebase;
    // media_type_t media_type;
    std::string codec_name;
};

#endif

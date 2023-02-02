#ifndef __DELETERS_HPP__
#define __DELETERS_HPP__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

/**
 * Functor for freeing SwsContext ffmpeg structure.
 */
struct SwsContext_Deleter {
    //! Free pointer on sws context.
    /*!
        \param ptr Pointer which will be freed.
    */
    void operator()(SwsContext *ptr) {
        if (ptr != nullptr) {
            sws_freeContext(ptr);
        }
    }
};

/**
 * Functor for freeing AVFormatContext ffmpeg structure.
 */
struct AVFormatContext_Deleter {
    //! Close and free pointer on input context.
    /*!
        \param ptr Pointer which will be freed.
    */
    void operator()(AVFormatContext *ptr) {
        if (ptr != nullptr) {
            avformat_close_input(&ptr);
            avformat_free_context(ptr);
        }
    }
};

/**
 * Functor for freeing AVCodecContext ffmpeg structure.
 */
struct AVCodecContext_Deleter {
    //! Free pointer on codec context.
    /*!
        \param ptr Pointer which will be freed.
    */
    void operator()(AVCodecContext *ptr) {
        if (ptr != nullptr) {
            avcodec_free_context(&ptr);
        }
    }
};

/**
 * Functor for freeing AVPacket ffmpeg structure.
 */
struct AVPacket_Deleter {
    //! Free pointer on AVPacket.
    /*!
        \param ptr Pointer which will be freed.
    */
    void operator()(AVPacket *ptr) {
        if (ptr != nullptr) {
            av_packet_free(&ptr);
        }
    }
};

/**
 * Functor for freeing AVFrame ffmpeg structure.
 */
struct AVFrame_Deleter {
    //! Free pointer on AVFrame.
    /*!
        \param ptr pointer which will be freed.
    */
    void operator()(AVFrame *ptr) {
        if (ptr != nullptr) {
            av_frame_free(&ptr);
        }
    }
};

/**
 * Structure for changing parameters between FFmpegInput and FFmpegOutput.
 */
struct stream_desc_t {
    //! The average bitrate of the encoded data.
    int64_t bit_rate;
    //! The dimensions of the video frame in pixels.
    int width;
    //! The height of the video frame in pixels.
    int height;
    //! the number of pictures in a group of pictures, or 0 for intra_only.
    int gop_size;
    /*! This is the fundamental unit of time (in seconds) in terms of which
     * frame timestamps are represented.
     */
    AVRational time_base;
    //! Maximum number of B-frames between non-B-frames.
    int max_b_frames;
    //! Real base framerate of the stream.
    AVRational r_frame_rate;
    //! Average framerate.
    AVRational avg_frame_rate;
};

#endif /* __DELETERS_HPP__ */

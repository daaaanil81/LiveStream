#ifndef __FFMPEG_OUTPUT_H__
#define __FFMPEG_OUTPUT_H__

#include "ffmpeg/deleters.hpp"
#include <memory>
#include <string>

/**
 * Class for output stream to rtp port.
 */
class FFmpegOutput {
  private:
    //! RTP output video url.
    std::string video_url_;
    //! RTP output format.
    AVOutputFormat *format_;
    //! Format context for output streaming.
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    //! Pointer on RTP video stream.
    AVStream *video_stream_;
    //! Codec context for settings stream parameters.
    std::shared_ptr<AVCodecContext> cctx_;

    //! Convert cv::Mat to AVFrame.
    /*!
     * \param image Image in BGR24 pixel format.
     * \return AVFrame in YUV420P pixel format.
     */
    std::shared_ptr<AVFrame> mat2frame(cv::Mat &image);

    //! Open video stream.
    /*!
     * Open video stream with parameters from FFmpegInput.
     * Fill AVFormatContext for output stream.
     * \param desc Structure with FFmpegInput stream parameters.
     * \param context Pointer on format context which will be filled.
     * \return AVFrame in YUV420P pixel format.
     */
    bool open_video_stream(const std::shared_ptr<stream_desc_t> &desc,
                           AVFormatContext *&context);

    //! Generate SDP of output stream.
    /*!
     * By using filled AVFormatContext, generates SDP file for debugging.
     * \param context Filled AVFormatContext.
     */
    void generate_sdp(AVFormatContext *context);

  public:
    //! Constructor for streaming to RTP url.
    /*!
     * \param video_url RTP url with ip and port.
     * \param video_desc Video parameters from FFmpegInput.
     */
    FFmpegOutput(const std::string &video_url,
                 const std::shared_ptr<stream_desc_t> &video_desc);

    //! Send image to RTP port.
    /*!
     * Receive cv::Mat.
     * Convert cv::Mat to AVFrame.
     * AVPacket collects with AVFrame.
     * AVPacket to RTP port.
     * \param image Image in BGR24 pixel format.
     * \param pts Set origin PTS from AVFrame before convertation.
     * \return True is successful, false is error.
     */
    bool send_image(cv::Mat &image, int64_t pts);
};

#endif

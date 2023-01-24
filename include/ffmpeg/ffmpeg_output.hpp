#ifndef __FFMPEG_OUTPUT_H__
#define __FFMPEG_OUTPUT_H__

#include "ffmpeg/deleters.hpp"
#include <memory>
#include <string>

class FFmpegOutput {
  public:
    FFmpegOutput(std::string video_url,
                 std::shared_ptr<stream_desc_t> video_desc);
    // ~FFmpegOutput();

    bool send_image(cv::Mat image);

  private:
    static int pts_frame_;
    std::string m_video_url_;
    AVOutputFormat *m_format_;
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    AVStream *m_video_stream_;
    std::shared_ptr<AVCodecContext> cctx_;

    bool open_video_stream_(std::string __url,
                            std::shared_ptr<stream_desc_t> __desc,
                            AVFormatContext **__context, AVStream **__stream);
};

#endif
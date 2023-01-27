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

    bool send_image(cv::Mat &image, int64_t pts);

  private:
    static uint64_t pts_frame_;
    std::string m_video_url_;
    AVOutputFormat *m_format_;
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    AVStream *m_video_stream_;
    std::shared_ptr<AVCodecContext> cctx_;

    std::shared_ptr<AVFrame> mat2frame(cv::Mat &image, int64_t pts);
    bool open_video_stream(std::string url, std::shared_ptr<stream_desc_t> desc,
                           AVFormatContext *&context, AVStream *&stream);
    void generate_sdp(AVFormatContext *pFormatCtx);
};

#endif

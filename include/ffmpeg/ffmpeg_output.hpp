#include "deleters.hpp"
#include <memory>
#include <string>

class FFmpegOutput {
  public:
    FFmpegOutput(std::string video_url,
                 std::shared_ptr<stream_desc_t> video_desc);
    // ~FFmpegOutput();

    bool send(std::shared_ptr<AVPacket> pPacket);

  private:
    std::string m_video_url_;
    AVOutputFormat *m_format_;
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    AVStream *m_video_stream_;

    bool open_video_stream_(std::string __url,
                            std::shared_ptr<stream_desc_t> __desc,
                            AVFormatContext **__context, AVStream **__stream);
};
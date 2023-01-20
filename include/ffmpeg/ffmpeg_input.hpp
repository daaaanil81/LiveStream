#ifndef __FFMPEG_INPUT_H__
#define __FFMPEG_INPUT_H__

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "ffmpeg/deleters.hpp"

class FFmpegInput {
  protected:
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    std::shared_ptr<AVCodecContext> spCodecContext_;
    std::list<std::shared_ptr<AVPacket>> packet_list_;
    std::thread thread_;
    std::atomic<bool> active_;
    std::mutex mutex_;
    int video_stream_index_;

  public:
    std::shared_ptr<AVPacket> get();
    cv::Mat get_mat();
    bool stream_status() const;
    void read_video_stream();
    void stop_stream();
    std::shared_ptr<stream_desc_t> get_stream_desc() const;
};

class FFmpegInputFile : public FFmpegInput {
  public:
    FFmpegInputFile(const char *path_to_file);
    ~FFmpegInputFile();
};

class FFmpegInputWebCamera : public FFmpegInput {
  public:
    FFmpegInputWebCamera(const char *device_name);
    ~FFmpegInputWebCamera();
};

#endif

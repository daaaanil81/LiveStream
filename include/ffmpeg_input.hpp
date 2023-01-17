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
#include <atomic>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

enum Code {
    SUCCESS = 0,
    ERROR = -1,
};

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
    virtual std::shared_ptr<AVPacket> get() = 0;
    bool stream_status();
    void read_video_stream();
    void stop_stream(bool new_actions);
};

class FFmpegInputFile : public FFmpegInput {
  public:
    std::shared_ptr<AVPacket> get() override;
    FFmpegInputFile(const char *path_to_file);
    ~FFmpegInputFile();
};

class FFmpegInputWebCamera : public FFmpegInput {
  public:
    std::shared_ptr<AVPacket> get() override;
    FFmpegInputWebCamera(const char *device_name);
    ~FFmpegInputWebCamera();
};

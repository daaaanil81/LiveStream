#include "ffmpeg_input.hpp"
#include <exception>
#include <string>

bool FFmpegInput::stream_status() { return active_; };

void FFmpegInput::stop_stream(bool new_actions) { active_ = new_actions; };

FFmpegInputFile::~FFmpegInputFile() { thread_.join(); };

FFmpegInputWebCamera::~FFmpegInputWebCamera() { thread_.join(); };

void FFmpegInput::read_video_stream() {

    AVPacket *pPacket = nullptr;
    std::shared_ptr<AVPacket> spPacket =
        std::shared_ptr<AVPacket>(pPacket, AVPacket_Deleter());
    while (active_) {
        pPacket = av_packet_alloc();
        if (pPacket == nullptr) {
            throw std::logic_error("Failed to allocate memory for packet");
        }
        int res = av_read_frame(spAVFormatContext_.get(), pPacket);
        if (res >= 0) {
            if (pPacket->stream_index == video_stream_index_) {
                std::cout << pPacket->pos << std::endl;
                std::lock_guard<std::mutex> lock(mutex_);
                // spPacket.reset(pPacket);
                packet_list_.push_back(spPacket);
                std::cout << "Pushing packet, list size: "
                          << packet_list_.size() << std::endl;

            } else {
                av_packet_unref(pPacket);
            }
        } else {
            active_ = false;
            break;
        }
    }
};

FFmpegInputFile::FFmpegInputFile(const char *path_to_file) {

    spAVFormatContext_ = std::shared_ptr<AVFormatContext>(
        avformat_alloc_context(), AVFormatContext_Deleter());

    if (spAVFormatContext_.get() == nullptr) {
        std::cerr << "Failed with allocate memory for context" << std::endl;
    }

    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;

    packet_list_ = std::list<std::shared_ptr<AVPacket>>{};

    int res;
    video_stream_index_ = -1;
    AVFormatContext *pAVFormatContext_ = spAVFormatContext_.get();

    res =
        avformat_open_input(&pAVFormatContext_, path_to_file, nullptr, nullptr);
    if (res != SUCCESS) {
        throw std::logic_error("Failed with receiving format context of file");
    }

    res = avformat_find_stream_info(spAVFormatContext_.get(), nullptr);

    if (res != SUCCESS) {
        throw std::logic_error("Failed with find stream in file");
    }

    std::cout << "Format: " << spAVFormatContext_->iformat->name
              << " Duration: " << spAVFormatContext_->duration / AV_TIME_BASE
              << " us" << std::endl;

    std::cout << "Count of Stream: " << spAVFormatContext_->nb_streams
              << std::endl;

    for (int i = 0; i < spAVFormatContext_->nb_streams; i++) {

        AVCodecParameters *pLocalCodecParameters =
            spAVFormatContext_->streams[i]->codecpar;

        const AVCodec *pLocalCodec =
            avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == nullptr) {
            std::cerr << "Unsupported codec" << std::endl;
            continue;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index_ == -1) {
                video_stream_index_ = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }
            std::cout << "Video Codec: resolution "
                      << pLocalCodecParameters->width << " x "
                      << pLocalCodecParameters->height << std::endl;
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "Audio Codec: " << pLocalCodecParameters->channels
                      << " channels, sample rate "
                      << pLocalCodecParameters->sample_rate << std::endl;
        }

        std::cout << "Codec " << pLocalCodec->name << " ID " << pLocalCodec->id
                  << " bit_rate " << pLocalCodecParameters->bit_rate
                  << std::endl;
    }

    if (video_stream_index_ == -1) {
        throw std::logic_error("File does not contain a video stream!");
    }

    spCodecContext_ = std::shared_ptr<AVCodecContext>(
        avcodec_alloc_context3(pCodec), AVCodecContext_Deleter());

    if (!spCodecContext_.get()) {
        throw std::logic_error("Failed to allocated memory for AVCodecContext");
    }

    res =
        avcodec_parameters_to_context(spCodecContext_.get(), pCodecParameters);
    if (res != SUCCESS) {
        throw std::logic_error("Failed to fill codec context");
    }

    res = avcodec_open2(spCodecContext_.get(), pCodec, nullptr);
    if (res != SUCCESS) {
        throw std::logic_error(
            "Failed to initialize context to use the given codec");
    }

    active_ = true;

    thread_ = std::thread(&FFmpegInputFile::read_video_stream, this);
};

FFmpegInputWebCamera::FFmpegInputWebCamera(const char *device_name) {

    avdevice_register_all();

    spAVFormatContext_ = std::shared_ptr<AVFormatContext>(
        avformat_alloc_context(), AVFormatContext_Deleter());

    if (spAVFormatContext_.get() == nullptr) {
        std::cerr << "Failed with allocate memory for context" << std::endl;
    }

    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;

    packet_list_ = std::list<std::shared_ptr<AVPacket>>{};

    int res;
    video_stream_index_ = -1;
    AVFormatContext *pAVFormatContext_ = spAVFormatContext_.get();

    const AVInputFormat *cinputFormat = av_find_input_format("v4l2");
    AVInputFormat *inputFormat = const_cast<AVInputFormat *>(cinputFormat);

    res = avformat_open_input(&pAVFormatContext_, device_name, inputFormat,
                              nullptr);
    if (res != SUCCESS) {
        throw std::logic_error("Failed with receiving format context of file");
    }

    res = avformat_find_stream_info(spAVFormatContext_.get(), nullptr);

    if (res != SUCCESS) {
        throw std::logic_error("Failed with find stream in file");
    }

    std::cout << "Format: " << spAVFormatContext_->iformat->name
              << " Duration: " << spAVFormatContext_->duration / AV_TIME_BASE
              << " us" << std::endl;

    std::cout << "Count of Stream: " << spAVFormatContext_->nb_streams
              << std::endl;

    for (int i = 0; i < spAVFormatContext_->nb_streams; i++) {

        AVCodecParameters *pLocalCodecParameters =
            spAVFormatContext_->streams[i]->codecpar;

        const AVCodec *pLocalCodec =
            avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == nullptr) {
            std::cerr << "Unsupported codec" << std::endl;
            continue;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index_ == -1) {
                video_stream_index_ = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }
            std::cout << "Video Codec: resolution "
                      << pLocalCodecParameters->width << " x "
                      << pLocalCodecParameters->height << std::endl;
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "Audio Codec: " << pLocalCodecParameters->channels
                      << " channels, sample rate "
                      << pLocalCodecParameters->sample_rate << std::endl;
        }

        std::cout << "Codec " << pLocalCodec->name << " ID " << pLocalCodec->id
                  << " bit_rate " << pLocalCodecParameters->bit_rate
                  << std::endl;
    }

    if (video_stream_index_ == -1) {
        throw std::logic_error("File does not contain a video stream!");
    }

    spCodecContext_ = std::shared_ptr<AVCodecContext>(
        avcodec_alloc_context3(pCodec), AVCodecContext_Deleter());

    if (!spCodecContext_.get()) {
        throw std::logic_error("Failed to allocated memory for AVCodecContext");
    }

    res =
        avcodec_parameters_to_context(spCodecContext_.get(), pCodecParameters);
    if (res != SUCCESS) {
        throw std::logic_error("Failed to fill codec context");
    }

    res = avcodec_open2(spCodecContext_.get(), pCodec, nullptr);
    if (res != SUCCESS) {
        throw std::logic_error(
            "Failed to initialize context to use the given codec");
    }

    active_ = true;

    thread_ = std::thread(&FFmpegInputFile::read_video_stream, this);
};

std::shared_ptr<AVPacket> FFmpegInputFile::get() {

    std::shared_ptr<AVPacket> result_packet = nullptr;
    std::lock_guard<std::mutex> lock(mutex_);
    if (packet_list_.size() != 0) {
        std::cout << "Before receiving packet: " << packet_list_.size();
        result_packet = packet_list_.front();
        packet_list_.pop_front();
        std::cout << " After: " << packet_list_.size() << std::endl;
    }

    return result_packet;
}

std::shared_ptr<AVPacket> FFmpegInputWebCamera::get() {

    std::shared_ptr<AVPacket> result_packet = nullptr;
    std::lock_guard<std::mutex> lock(mutex_);
    if (packet_list_.size() != 0) {
        std::cout << "Before receiving packet: " << packet_list_.size();
        result_packet = packet_list_.front();
        packet_list_.pop_front();
        std::cout << " After: " << packet_list_.size() << std::endl;
    }

    return result_packet;
};

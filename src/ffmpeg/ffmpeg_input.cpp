#include "ffmpeg/ffmpeg_input.hpp"
#include <exception>

cv::Mat decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext) {

    AVFrame *pFrame = av_frame_alloc();
    int res = -1;
    int cvLinesizes[1] = {0};
    std::ostringstream os("frame");
    AVPixelFormat src_pix_fmt = AV_PIX_FMT_YUYV422;
    AVPixelFormat dst_pix_fmt = AV_PIX_FMT_BGR24;
    std::unique_ptr<SwsContext, SwsContext_Deleter> sws_ctx(
        nullptr, SwsContext_Deleter());
    cv::Mat image;

    /* Supply raw packet data as input to a decoder. */
    res = avcodec_send_packet(pCodecContext, pPacket);
    if (res != 0) {
        throw std::logic_error("Failed to send packet to decoder\n");
    }

    /* Return decoded output data from a decoder. */
    res = avcodec_receive_frame(pCodecContext, pFrame);
    if (res == AVERROR(EAGAIN) || res == AVERROR_EOF) {
        return image;
    } else if (res != 0) {
        throw std::logic_error("Failed to receive frame from decoder\n");
    }

    std::cout << "Frame " << pCodecContext->frame_number
              << " (type=" << av_get_picture_type_char(pFrame->pict_type)
              << ", size=" << pFrame->pkt_size
              << " bytes, format=" << pFrame->format << ") pts " << pFrame->pts
              << " " << pFrame->width << " x " << pFrame->height
              << " key_frame " << pFrame->key_frame << " [DTS "
              << pFrame->coded_picture_number << "]" << std::endl;

    /* create scaling context */
    sws_ctx.reset(sws_getContext(pFrame->width, pFrame->height, src_pix_fmt,
                                 pFrame->width, pFrame->height, dst_pix_fmt,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr));
    image = cv::Mat(pFrame->height, pFrame->width, CV_8UC3);

    cvLinesizes[0] = image.step1();

    /* convert to destination format */
    sws_scale(sws_ctx.get(), pFrame->data, pFrame->linesize, 0, pFrame->height,
              &image.data, cvLinesizes);

    os << pFrame->pts << ".jpg";

    cv::imwrite("images/" + os.str(), image);

    return image;
};

bool FFmpegInput::stream_status() const {
    if (!active_ && (packet_list_.size() == 0)) {
        return false;
    } else {
        return true;
    }
};

void FFmpegInput::stop_stream() { active_ = false; };

FFmpegInputFile::~FFmpegInputFile() { thread_.join(); };

FFmpegInputWebCamera::~FFmpegInputWebCamera() { thread_.join(); };

void FFmpegInput::read_video_stream() {

    while (active_) {
        std::shared_ptr<AVPacket> spPacket =
            std::shared_ptr<AVPacket>(av_packet_alloc(), AVPacket_Deleter());
        if (!spPacket) {
            throw std::logic_error("Failed to allocate memory for packet");
        }
        int res = av_read_frame(spAVFormatContext_.get(), spPacket.get());
        if (res >= 0) {
            if (spPacket->stream_index == video_stream_index_) {
                // std::cout << pPacket->pos << std::endl;
                std::lock_guard<std::mutex> lock(mutex_);
                packet_list_.push_back(spPacket);
                std::cout << "Pushing packet, list size: "
                          << packet_list_.size() << std::endl;

            } else {
                av_packet_unref(spPacket.get());
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

    if (!spAVFormatContext_) {
        std::cerr << "Failed with allocate memory for context" << std::endl;
    }

    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;

    int res = -1;
    video_stream_index_ = -1;
    AVFormatContext *pAVFormatContext_ = spAVFormatContext_.get();

    res =
        avformat_open_input(&pAVFormatContext_, path_to_file, nullptr, nullptr);
    if (res != 0) {
        throw std::logic_error("Failed with receiving format context of file");
    }

    res = avformat_find_stream_info(spAVFormatContext_.get(), nullptr);
    if (res != 0) {
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
    if (!spCodecContext_) {
        throw std::logic_error("Failed to allocated memory for AVCodecContext");
    }

    res =
        avcodec_parameters_to_context(spCodecContext_.get(), pCodecParameters);
    if (res != 0) {
        throw std::logic_error("Failed to fill codec context");
    }

    res = avcodec_open2(spCodecContext_.get(), pCodec, nullptr);
    if (res != 0) {
        throw std::logic_error(
            "Failed to initialize context to use the given codec");
    }

    av_dump_format(spAVFormatContext_.get(), 0, path_to_file, 0);

    active_ = true;

    thread_ = std::thread(&FFmpegInputFile::read_video_stream, this);
};

FFmpegInputWebCamera::FFmpegInputWebCamera(const char *device_name) {

    avdevice_register_all();

    spAVFormatContext_ = std::shared_ptr<AVFormatContext>(
        avformat_alloc_context(), AVFormatContext_Deleter());

    if (!spAVFormatContext_) {
        std::cerr << "Failed with allocate memory for context" << std::endl;
    }

    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;

    int res = -1;
    video_stream_index_ = -1;
    AVFormatContext *pAVFormatContext_ = spAVFormatContext_.get();

    const AVInputFormat *cinputFormat = av_find_input_format("v4l2");
    AVInputFormat *inputFormat = const_cast<AVInputFormat *>(cinputFormat);

    res = avformat_open_input(&pAVFormatContext_, device_name, inputFormat,
                              nullptr);
    if (res != 0) {
        throw std::logic_error("Failed with receiving format context of file");
    }

    res = avformat_find_stream_info(spAVFormatContext_.get(), nullptr);
    if (res != 0) {
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
    if (!spCodecContext_) {
        throw std::logic_error("Failed to allocated memory for AVCodecContext");
    }

    res =
        avcodec_parameters_to_context(spCodecContext_.get(), pCodecParameters);
    if (res != 0) {
        throw std::logic_error("Failed to fill codec context");
    }

    res = avcodec_open2(spCodecContext_.get(), pCodec, nullptr);
    if (res != 0) {
        throw std::logic_error(
            "Failed to initialize context to use the given codec");
    }

    av_dump_format(spAVFormatContext_.get(), 0, device_name, 0);

    active_ = true;

    thread_ = std::thread(&FFmpegInputFile::read_video_stream, this);
};

std::shared_ptr<AVPacket> FFmpegInput::get() {

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

cv::Mat FFmpegInput::get_mat() {

    cv::Mat packet_image;
    std::shared_ptr<AVPacket> result_packet{nullptr, AVPacket_Deleter()};
    std::lock_guard<std::mutex> lock(mutex_);
    if (packet_list_.size() != 0) {
        std::cout << "Before receiving packet: " << packet_list_.size();
        result_packet = packet_list_.front();
        packet_list_.pop_front();
        std::cout << " After: " << packet_list_.size() << std::endl;
        packet_image =
            decode_packet(result_packet.get(), spCodecContext_.get());
    }

    return packet_image;
}

std::shared_ptr<stream_desc_t> FFmpegInput::get_stream_desc() const {

    auto desc = std::make_shared<stream_desc_t>();
    memset(desc.get(), 0, sizeof(stream_desc_t));

    const auto &stream = spAVFormatContext_->streams[video_stream_index_];
    desc->bit_rate = stream->codecpar->bit_rate;

    desc->width = stream->codecpar->width;
    desc->height = stream->codecpar->height;

    desc->sample_rate = stream->codecpar->sample_rate;
    desc->timebase.numerator = stream->time_base.num;
    desc->timebase.denominator = stream->time_base.den;
    desc->codec_name = avcodec_get_name(stream->codecpar->codec_id);

    return desc;
}
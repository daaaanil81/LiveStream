#include "ffmpeg/ffmpeg_output.hpp"
#include <algorithm>
#include <exception>
#include <fstream>

bool FFmpegOutput::open_video_stream_(std::string url,
                                      std::shared_ptr<stream_desc_t> desc,
                                      AVFormatContext **context,
                                      AVStream **stream) {

    int status;
    // create context
    status = avformat_alloc_output_context2(context, m_format_, m_format_->name,
                                            url.c_str());
    if (status < 0) {
        std::cerr << "error: avformat_alloc_output_context2; line: " << __LINE__
                  << std::endl;
        return false;
    }

    // open IO interface
    status = avio_open(&(*context)->pb, url.c_str(), AVIO_FLAG_WRITE);
    if (status < 0) {
        std::cerr << "error: avio_open; line: " << __LINE__ << std::endl;
        return false;
    }

    // set stream description

    AVCodec *codec = avcodec_find_encoder_by_name(desc->codec_name.c_str());
    if (!codec) {
        auto name = desc->codec_name;
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (name == "h264") {
            name = "libx264";
        }

        codec = avcodec_find_encoder_by_name(name.c_str());
        if (!codec) {
            std::cerr << "error: avcodec_find_encoder_by_name; line: "
                      << __LINE__ << std::endl;
            std::cerr << "Input: " << desc->codec_name << std::endl;
            return false;
        }
    }

    AVCodecContext *cctx = avcodec_alloc_context3(codec);

    if (!cctx) {
        std::cout << "can't create codec context" << std::endl;
        return -1;
    }

    cctx->bit_rate = 200000;
    cctx->width = 1080;
    cctx->height = 720;
    cctx->gop_size = 25;
    cctx->time_base.num = 1;
    cctx->time_base.den = 25;
    cctx->max_b_frames = 1;
    cctx->pix_fmt = AV_PIX_FMT_YUV420P;
    cctx->codec_type = AVMEDIA_TYPE_VIDEO;
    cctx->framerate = AVRational{1, 25};
    cctx->time_base = AVRational{1, 25};

    // cctx->flags ^= AV_CODEC_FLAG_GLOBAL_HEADER;

    int tmp;
    if (cctx->codec_id == AV_CODEC_ID_H264) {
        tmp = av_opt_set(cctx->priv_data, "preset", "ultrafast", 0);
        tmp = av_opt_set(cctx->priv_data, "tune", "zerolatency", 0);
    }

    std::cout << "Flags " << cctx->flags << std::endl;

    *stream = avformat_new_stream(*context, codec);

    (*stream)->time_base = (AVRational){1, 25};
    (*stream)->r_frame_rate = (AVRational){1, 25};

    std::cout << "bitrate " << (*stream)->codecpar->bit_rate << std::endl;
    std::cout << "width " << (*stream)->codecpar->width << std::endl;
    std::cout << "height " << (*stream)->codecpar->height << std::endl;
    std::cout << "num " << (*stream)->time_base.num << std::endl;
    std::cout << "den " << (*stream)->time_base.den << std::endl;
    std::cout << "codec id " << (*stream)->codecpar->codec_id << std::endl;
    std::cout << "codec type " << (*stream)->codecpar->codec_type << std::endl;
    // std::cout << "Flags " << (*stream)->parser->priv_data << std::endl;

    if ((*context)->oformat->flags & AVFMT_GLOBALHEADER) {
        (*stream)->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    int res = avcodec_open2(cctx, codec, NULL);

    avcodec_parameters_from_context((*stream)->codecpar, cctx);

    std::cout << "bitrate " << (*stream)->codecpar->bit_rate << std::endl;
    std::cout << "width " << (*stream)->codecpar->width << std::endl;
    std::cout << "height " << (*stream)->codecpar->height << std::endl;
    std::cout << "num " << (*stream)->time_base.num << std::endl;
    std::cout << "den " << (*stream)->time_base.den << std::endl;
    std::cout << "codec id " << (*stream)->codecpar->codec_id << std::endl;
    std::cout << "codec type " << (*stream)->codecpar->codec_type << std::endl;

    return true;
}

FFmpegOutput::FFmpegOutput(std::string video_url,
                           std::shared_ptr<stream_desc_t> video_desc) {

    m_video_url_ = video_url;
    m_format_ = nullptr;
    spAVFormatContext_ = std::shared_ptr<AVFormatContext>(
        avformat_alloc_context(), AVFormatContext_Deleter());
    m_video_stream_ = nullptr;

    int status;

    m_format_ = av_guess_format("rtp", NULL, NULL);

    if (m_format_ == NULL) {
        throw std::logic_error("Format not available");
    }
    AVFormatContext *m_video_context = spAVFormatContext_.get();

    auto success = open_video_stream_(m_video_url_, video_desc,
                                      &m_video_context, &m_video_stream_);

    if (!success) {
        throw std::logic_error("Failed to open video stream");
    }

    status = avformat_write_header(m_video_context, NULL);

    char buf[100000];
    av_sdp_create(&m_video_context, 1, buf, 100000);
    std::string str_buf = buf;
    std::ofstream file_sdp;
    file_sdp.open("test.sdp", std::ios::out);
    file_sdp << str_buf;
    file_sdp.close();

    spAVFormatContext_.reset(m_video_context);
}

bool FFmpegOutput::send(std::shared_ptr<AVPacket> pPacket) {

    if (!spAVFormatContext_) {
        return false;
    }

    pPacket->stream_index = m_video_stream_->index;
    // std::cout << pPacket->dts << " " << pPacket->pts << std::endl;

    // int status = avformat_write_header(spAVFormatContext_.get(), NULL);
    auto status1 =
        av_interleaved_write_frame(spAVFormatContext_.get(), pPacket.get());
    // av_packet_unref(pPacket.get());

    return status1 == 0;
}
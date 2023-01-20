#include "ffmpeg/ffmpeg_output.hpp"
#include <algorithm>
#include <exception>
#include <fstream>

int FFmpegOutput::pts_frame_ = 0;

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

    // AVCodec *codec = avcodec_find_encoder_by_name(desc->codec_name.c_str());
    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
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

    cctx = avcodec_alloc_context3(codec);

    if (!cctx) {
        std::cout << "can't create codec context" << std::endl;
        return -1;
    }

    cctx->bit_rate = 147456000;
    cctx->width = 1280;
    cctx->height = 720;
    cctx->gop_size = 25;
    cctx->time_base.num = 1;
    cctx->time_base.den = 30;
    cctx->max_b_frames = 1;
    cctx->pix_fmt = AV_PIX_FMT_YUV420P;
    // cctx->pix_fmt = AV_PIX_FMT_YUYV422;
    cctx->codec_type = AVMEDIA_TYPE_VIDEO;
    cctx->framerate = AVRational{1, 30};
    cctx->time_base = AVRational{1, 30};
    std::cout << "Codec id: " << cctx->codec_id << std::endl;
    // cctx->flags ^= AV_CODEC_FLAG_GLOBAL_HEADER;

    int tmp;
    if (cctx->codec_id == AV_CODEC_ID_H264) {
        tmp = av_opt_set(cctx->priv_data, "preset", "ultrafast", 0);
        tmp = av_opt_set(cctx->priv_data, "tune", "zerolatency", 0);
    }

    std::cout << "Flags " << cctx->flags << std::endl;

    *stream = avformat_new_stream(*context, codec);

    (*stream)->time_base = (AVRational){1, 30};
    (*stream)->r_frame_rate = (AVRational){1, 30};

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

    // pPacket->stream_index = m_video_stream_->index;
    // std::cout << pPacket->dts << " " << pPacket->pts << std::endl;

    // int status = avformat_write_header(spAVFormatContext_.get(), NULL);
    // auto status1 =
    //     av_interleaved_write_frame(spAVFormatContext_.get(), pPacket.get());
    // av_packet_unref(pPacket.get());

    int j = 0;

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    int i, ret, x, y, got_output;

    frame->format = cctx->pix_fmt;
    frame->width = cctx->width;
    frame->height = cctx->height;
    ret = av_image_alloc(frame->data, frame->linesize, cctx->width,
                         cctx->height, cctx->pix_fmt, 32);

    /* prepare a dummy image */
    /* Y */
    for (y = 0; y < 720; y++) {
        for (x = 0; x < 1080; x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + 8 * 3;
        }
    }
    /* Cb and Cr */
    for (y = 0; y < 720 / 2; y++) {
        for (x = 0; x < 1080 / 2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + 8 * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + 8 * 5;
        }
    }
    frame->pts = i;
    /* encode the image */
    ret = avcodec_send_frame(cctx, frame);

    if (ret == AVERROR_EOF) {
        got_output = false;
        printf("Stream EOF 1\n");
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
        printf("Stream EAGAIN 1\n");
    } else {
        got_output = true;
    }

    ret = avcodec_receive_packet(cctx, pkt);

    if (ret == AVERROR_EOF) {
        got_output = false;
        printf("Stream EOF\n");
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
        printf("Stream EAGAIN\n");
    } else {
        got_output = true;
    }

    if (got_output) {
        printf("Write frame %3d (size=%5d)\n", j++, pkt->size);
        av_interleaved_write_frame(spAVFormatContext_.get(), pkt);
        av_packet_unref(pkt);
    }

    return 0;
}

bool FFmpegOutput::send_image(cv::Mat image) {

    int image_n = 0;
    if (image.empty()) {
        std::cout << "Empty" << std::endl;
    }

    int ret, got_output;
    int width = image.cols;
    int height = image.rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();

    AVPacket *pkt = av_packet_alloc();

    AVFrame *frame = av_frame_alloc();

    frame->format = cctx->pix_fmt;
    frame->width = cctx->width;
    frame->height = cctx->height;
    frame->pts = pts_frame_;
    pts_frame_ += 1;
    ret = av_image_alloc(frame->data, frame->linesize, cctx->width,
                         cctx->height, cctx->pix_fmt, 32);

    std::cout << "Image allocated " << frame->format << std::endl;

    SwsContext *conversion = sws_getContext(
        width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height,
        AVPixelFormat::AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    sws_scale(conversion, &image.data, cvLinesizes, 0, height, frame->data,
              frame->linesize);

    std::cout << "Image scaled" << std::endl;

    ret = avcodec_send_frame(cctx, frame);

    if (ret == AVERROR_EOF) {
        got_output = false;
        printf("Stream EOF\n");
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
        printf("Stream EAGAIN\n");
    }

    ret = avcodec_receive_packet(cctx, pkt);

    if (ret == AVERROR_EOF) {
        got_output = false;
        printf("Stream EOF\n");
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
        printf("Stream EAGAIN\n");
    } else {
        got_output = true;
    }

    if (got_output) {
        std::cout << "Write frame" << frame->pts << std::endl;
        av_interleaved_write_frame(spAVFormatContext_.get(), pkt);
        av_packet_unref(pkt);
    }

    sws_freeContext(conversion);
}
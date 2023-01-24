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

    status = avformat_alloc_output_context2(context, m_format_, m_format_->name,
                                            url.c_str());
    if (status < 0) {
        std::cerr << "error: avformat_alloc_output_context2; line: " << __LINE__
                  << std::endl;
        return false;
    }

    status = avio_open(&(*context)->pb, url.c_str(), AVIO_FLAG_WRITE);
    if (status < 0) {
        std::cerr << "error: avio_open; line: " << __LINE__ << std::endl;
        return false;
    }

    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        std::cerr << "error: avcodec_find_encoder_by_name; line: " << __LINE__
                  << std::endl;
        return false;
    }

    cctx_ = std::shared_ptr<AVCodecContext>{avcodec_alloc_context3(codec),
                                            AVCodecContext_Deleter()};
    if (!cctx_) {
        std::cerr << "Can't create codec context" << std::endl;
        return false;
    }

    cctx_->bit_rate = desc->bit_rate / 2;
    cctx_->width = desc->width;
    cctx_->height = desc->height;
    cctx_->gop_size = 25;
    cctx_->time_base.num = 1;
    cctx_->time_base.den = 30;
    cctx_->max_b_frames = 1;
    cctx_->pix_fmt = AV_PIX_FMT_YUV420P;
    cctx_->codec_type = AVMEDIA_TYPE_VIDEO;
    cctx_->framerate = AVRational{1, 30};
    cctx_->time_base = AVRational{1, 30};

    int tmp;
    if (cctx_->codec_id == AV_CODEC_ID_H264) {
        tmp = av_opt_set(cctx_->priv_data, "preset", "ultrafast", 0);
        tmp = av_opt_set(cctx_->priv_data, "tune", "zerolatency", 0);
    }

    *stream = avformat_new_stream(*context, codec);

    (*stream)->time_base = (AVRational){1, 30};
    (*stream)->r_frame_rate = (AVRational){1, 30};

    if ((*context)->oformat->flags & AVFMT_GLOBALHEADER) {
        (*stream)->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    int res = avcodec_open2(cctx_.get(), codec, NULL);

    avcodec_parameters_from_context((*stream)->codecpar, cctx_.get());

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

bool FFmpegOutput::send_image(cv::Mat image) {

    if (image.empty()) {
        std::cout << "Empty" << std::endl;
    }

    int ret, got_output;
    int width = image.cols;
    int height = image.rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();

    std::shared_ptr<AVPacket> pkt{av_packet_alloc(), AVPacket_Deleter()};

    std::shared_ptr<AVFrame> frame{av_frame_alloc(), AVFrame_Deleter()};

    frame->format = cctx_->pix_fmt;
    frame->width = cctx_->width;
    frame->height = cctx_->height;
    frame->pts = pts_frame_;
    pts_frame_ += 1;
    ret = av_image_alloc(frame->data, frame->linesize, cctx_->width,
                         cctx_->height, cctx_->pix_fmt, 32);

    std::cout << "Image allocated " << frame->format << std::endl;

    std::unique_ptr<SwsContext, SwsContext_Deleter> sws_ctx(
        nullptr, SwsContext_Deleter());

    sws_ctx.reset(sws_getContext(
        width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height,
        AVPixelFormat::AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL));

    sws_scale(sws_ctx.get(), &image.data, cvLinesizes, 0, height, frame->data,
              frame->linesize);

    std::cout << "Image scaled" << std::endl;

    ret = avcodec_send_frame(cctx_.get(), frame.get());

    if (ret == AVERROR_EOF) {
        got_output = false;
        printf("Stream EOF\n");
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
        printf("Stream EAGAIN\n");
    }

    ret = avcodec_receive_packet(cctx_.get(), pkt.get());

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
        std::ostringstream os("frame");
        os << frame->pts << ".jpg";
        cv::imwrite("images_2/" + os.str(), image);
        std::cout << "Write frame " << frame->pts << std::endl;
        av_interleaved_write_frame(spAVFormatContext_.get(), pkt.get());
        av_packet_unref(pkt.get());
    }
}
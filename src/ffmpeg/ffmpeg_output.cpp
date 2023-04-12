#include "ffmpeg/ffmpeg_output.hpp"
#include <algorithm>
#include <exception>
#include <fstream>

bool FFmpegOutput::open_video_stream(const std::shared_ptr<stream_desc_t> &desc,
                                     AVFormatContext *&context) {

    int status = avformat_alloc_output_context2(
        &context, format_, format_->name, video_url_.c_str());
    if (status < 0) {
        std::cerr << "error: avformat_alloc_output_context2; line: " << __LINE__
                  << std::endl;
        return false;
    }

    status = avio_open(&context->pb, video_url_.c_str(), AVIO_FLAG_WRITE);
    if (status < 0) {
        std::cerr << "error: avio_open; line: " << __LINE__ << std::endl;
        return false;
    }

    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "error: avcodec_find_encoder_by_name; line: " << __LINE__
                  << std::endl;
        return false;
    }

    video_stream_ = avformat_new_stream(context, nullptr);

    video_stream_->id = (int)(context->nb_streams - 1);


    cctx_ = std::shared_ptr<AVCodecContext>{avcodec_alloc_context3(codec),
                                            AVCodecContext_Deleter()};
    if (!cctx_) {
        std::cerr << "Can't create codec context" << std::endl;
        return false;
    }

    int bitrate = 1000000;
    if (desc->height > 1000)
        bitrate = 3000000;
    else if (desc->height > 700)
        bitrate = 1500000;

    cctx_->codec_id = context->oformat->video_codec;
	cctx_->bit_rate = bitrate;
	video_stream_->time_base = av_d2q(1.0 / 30, 120);

	cctx_->time_base = video_stream_->time_base;
	cctx_->gop_size = 12;
	cctx_->max_b_frames = 2;

    cctx_->width = desc->width;
    cctx_->height = desc->height;
    cctx_->pix_fmt = AV_PIX_FMT_YUV420P;
    cctx_->codec_type = AVMEDIA_TYPE_VIDEO;
    if (context->oformat->flags & AVFMT_GLOBALHEADER) {
        cctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (cctx_->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(cctx_->priv_data, "preset", "ultrafast", 0);
        av_opt_set(cctx_->priv_data, "tune", "zerolatency", 0);
    }

    int res = avcodec_open2(cctx_.get(), codec, NULL);
    if (res < 0) {
        std::cerr << "Cannot open video encoder for stream" << std::endl;
        return false;
    }

    avcodec_parameters_from_context(video_stream_->codecpar, cctx_.get());

    std::cout << "bitrate " << video_stream_->codecpar->bit_rate << std::endl;
    std::cout << "width " << video_stream_->codecpar->width << std::endl;
    std::cout << "height " << video_stream_->codecpar->height << std::endl;
    std::cout << "num " << video_stream_->time_base.num << std::endl;
    std::cout << "den " << video_stream_->time_base.den << std::endl;
    std::cout << "codec id " << video_stream_->codecpar->codec_id << std::endl;
    std::cout << "codec type " << video_stream_->codecpar->codec_type
              << std::endl;

    return true;
}

void FFmpegOutput::generate_sdp(AVFormatContext *context) {
    char buf[100000] = {0};

    av_sdp_create(&context, 1, buf, 100000);

    std::ofstream file_sdp("test.sdp");
    file_sdp << buf;
}

FFmpegOutput::FFmpegOutput(const std::string &video_url,
                           const std::shared_ptr<stream_desc_t> &video_desc)
    : video_url_(video_url), format_(nullptr), video_stream_(nullptr) {

    format_ = av_guess_format("rtp", NULL, NULL);
    if (format_ == NULL) {
        throw std::logic_error("Format not available");
    }

    AVFormatContext *video_context = nullptr;
    auto success = open_video_stream(video_desc, video_context);
    if (!success) {
        throw std::logic_error("Failed to open video stream");
    }

    int ret = avformat_write_header(video_context, nullptr);
    if (ret < 0) {
        throw std::logic_error("Error occurred when opening output file");
    }

    generate_sdp(video_context);

    av_dump_format(video_context, 0, video_url_.c_str(), 1);

    spAVFormatContext_.reset(video_context);
}

std::shared_ptr<AVFrame> FFmpegOutput::mat2frame(cv::Mat &image) {

    int width = image.cols;
    int height = image.rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();

    std::shared_ptr<AVFrame> frame{av_frame_alloc(), AVFrame_Deleter()};

    frame->format = cctx_->pix_fmt;
    frame->width = cctx_->width;
    frame->height = cctx_->height;
    av_image_alloc(frame->data, frame->linesize, cctx_->width, cctx_->height,
                   cctx_->pix_fmt, 32);

    /* std::cout << "Image allocated " << frame->format << std::endl; */

    std::unique_ptr<SwsContext, SwsContext_Deleter> sws_ctx(
        nullptr, SwsContext_Deleter());

    sws_ctx.reset(sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_BGR24,
                                 width, height,
                                 AVPixelFormat::AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                 nullptr, nullptr, nullptr));

    sws_scale(sws_ctx.get(), &image.data, cvLinesizes, 0, height, frame->data,
              frame->linesize);

    /* std::cout << "Image scaled" << std::endl; */

    return frame;
}

bool FFmpegOutput::send_image(cv::Mat &image, int64_t pts) {

    if (image.empty()) {
        std::cerr << "Mat is empty" << std::endl;
        return false;
    }

    int ret, got_output;

    std::shared_ptr<AVFrame> frame = mat2frame(image);

    frame->pts = pts++;

    ret = avcodec_send_frame(cctx_.get(), frame.get());
    if (ret == AVERROR_EOF) {
        got_output = false;
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
    }

    std::shared_ptr<AVPacket> pkt{av_packet_alloc(), AVPacket_Deleter()};
    ret = avcodec_receive_packet(cctx_.get(), pkt.get());

    if (ret == AVERROR_EOF) {
        got_output = false;
    } else if (ret == AVERROR(EAGAIN)) {
        got_output = false;
    } else {
        got_output = true;
    }

    av_packet_rescale_ts(pkt.get(), cctx_->time_base, video_stream_->time_base);
	pkt->stream_index = video_stream_->index;

    if (got_output) {
        std::ostringstream os("frame");
        os << frame->pts << ".jpg";
        cv::imwrite("images_2/" + os.str(), image);
        av_interleaved_write_frame(spAVFormatContext_.get(), pkt.get());
        av_packet_unref(pkt.get());
    }

    return true;
}

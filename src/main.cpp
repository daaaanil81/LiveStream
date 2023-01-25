#include <csignal>
#include <iostream>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

#include <rtc/rtc.hpp>

#include "ffmpeg/ffmpeg_input.hpp"
#include "ffmpeg/ffmpeg_output.hpp"
#include "opencv/opencv.hpp"
#include "socket/socket.h"
#include "socket/ws_socket.h"

std::atomic<bool> running = true;

void signalHandler(int signal) { running = false; }

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);

    avformat_network_init();

    std::string pathStr;
    std::string type;

    if (argc > 2) {
        pathStr = argv[1];
        type = argv[2];
    } else {
        std::cout << "Usage: " << argv[0] << " <path_to_file> <type -f/-d>"
                  << std::endl;

        return -1;
    }

    try {
        std::shared_ptr<FFmpegInput> ffmpegInput;
        if (type == "-f") {
            ffmpegInput.reset(new FFmpegInputFile(pathStr.c_str()));
        } else if (type == "-d") {
            ffmpegInput.reset(new FFmpegInputWebCamera(pathStr.c_str()));
        } else {
            std::cout << "Usage: " << argv[0] << " <path_to_file> <type -f/-d>"
                      << std::endl;
            return -1;
        }
        std::shared_ptr<AiTask> opencv_processing(new LPR(
            "../voc.txt", "../crnn_cs.onnx", "../plate_detection.onnx"));

        std::shared_ptr<FFmpegOutput> output(new FFmpegOutput(
            "rtp://127.0.0.1:5004", ffmpegInput->get_stream_desc()));

        rtc::InitLogger(rtc::LogLevel::Debug);

        rtc::WebSocketServer::Configuration config;
        config.port = 10001;
        /* config.enableTls = true; */
        /* config.certificatePemFile = "./../../certificates/cert.pem"; */
        /* config.keyPemFile = "./../../certificates/key.pem"; */
        WSServerFacade ws(config);
        SocketWrapper socket("127.0.0.1", 5004);

        auto stream = std::async(std::launch::async, [&] {
            while (running.load() && ffmpegInput->stream_status()) {
                cv::Mat image = ffmpegInput->get_mat();
                if (!image.empty()) {
                    image = opencv_processing->process_image(image);
                    output->send_image(image);
                }
            }
        });

        /* while (running.load() && ffmpegInput->stream_status()) { */
        while (running.load()) {
            int len;
            auto buffer = socket.recvBuffer(len);
            ws.send(buffer, len);
        }

        /* stream.wait(); */
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

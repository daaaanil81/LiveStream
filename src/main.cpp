#include <csignal>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

#include <rtc/rtc.hpp>

#include "ffmpeg/ffmpeg_input.hpp"
#include "socket/socket.h"
#include "socket/ws_socket.h"

std::atomic<bool> running = true;

void signalHandler(int signal) { running = false; }

int main(int argc, char *argv[]) {

    signal(SIGINT, signalHandler);

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

    std::shared_ptr<AVPacket> received_packet = nullptr;

    signal(SIGINT, signalHandler);

    try {
        rtc::InitLogger(rtc::LogLevel::Debug);

        rtc::WebSocketServer::Configuration config;
        config.port = 10001;
        /* config.enableTls = true; */
        /* config.certificatePemFile = "./../../certificates/cert.pem"; */
        /* config.keyPemFile = "./../../certificates/key.pem"; */
        WSServerFacade ws(config);
        SocketWrapper socket("127.0.0.1", 6000);

        while (running.load()) {
            if (ws.sizeTracks() > 0) {
                /* std::cout << "RECV" << std::endl; */
                /* int len; */
                /* auto buffer = socket.recvBuffer(len); */
                /* ws.send(buffer, len); */
                received_packet = ffmpegInput->get();
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ffmpegInput->stop_stream();
    return 0;
}

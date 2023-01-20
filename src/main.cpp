#include <csignal>
#include <iostream>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

#include <rtc/rtc.hpp>

#include "ffmpeg/ffmpeg_input.hpp"
#include "ffmpeg/ffmpeg_output.hpp"
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

    std::shared_ptr<FFmpegInput> ffmpegInput;
    FFmpegOutput output("rtp://localhost:5004", ffmpegInput->get_stream_desc());

    std::shared_ptr<AVPacket> received_packet;

    if (type == "-f") {
        ffmpegInput.reset(new FFmpegInputFile(pathStr.c_str()));
    } else if (type == "-d") {
        ffmpegInput.reset(new FFmpegInputWebCamera(pathStr.c_str()));
    } else {
        std::cout << "Usage: " << argv[0] << " <path_to_file> <type -f/-d>"
                  << std::endl;

        return -1;
    }

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
                if (received_packet) {
                    output.send(received_packet);
                }
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ffmpegInput->stop_stream();
    return 0;
}

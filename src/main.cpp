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

#include "socket/socket.h"
#include "socket/ws_socket.h"

std::atomic<bool> running = true;

void signalHandler(int signal) { running = false; }

int main(int argc, char *argv[]) {

    std::cout << "Ok" << std::endl;
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
                std::cout << "RECV" << std::endl;
                int len;
                auto buffer = socket.recvBuffer(len);
                ws.send(buffer, len);
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

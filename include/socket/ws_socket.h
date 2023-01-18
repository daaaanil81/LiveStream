#ifndef __WS_SOCKET_H__
#define __WS_SOCKET_H__

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include <rtc/rtc.hpp>

#include <rtc/peerconnection.hpp>
#include <rtc/rtp.hpp>
#include <rtc/websocket.hpp>
#include <rtc/websocketserver.hpp>

class WSServerFacade {
    using MapPeerConnections =
        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>>;
    using MapConnections =
        std::unordered_map<int, std::shared_ptr<rtc::WebSocket>>;
    using MapTracks = std::unordered_map<int, std::shared_ptr<rtc::Track>>;

  private:
    rtc::WebSocketServer server_;
    static MapPeerConnections mapPeerConnections_;
    static MapConnections mapConnections_;
    static MapTracks mapTracks_;
    static int index_clients_;
    static std::atomic<size_t> sizeTracks_;
    static std::string sdp_;
    static std::mutex m_;
    // https://www.rfc-editor.org/rfc/rfc3550#section-8
    static const rtc::SSRC ssrc = 42;
    static const int payload_number = 96;

    static void onClientCallback(std::shared_ptr<rtc::WebSocket> ws);
    static void setLocalDescription(int index);
    static void setRemoteDescription(int index, const std::string &sdp);
    static void errorProcessing(int index, const std::string &error);

    static void addPeerConnection(int index,
                                  std::shared_ptr<rtc::PeerConnection> pc);
    static void addWebSocketConnection(int index,
                                       std::shared_ptr<rtc::WebSocket> ws);
    static void disconnect(int index);

  public:
    WSServerFacade(rtc::WebSocketServer::Configuration config);

    void setVideoDescription(const std::string &sdp);
    bool send(std::shared_ptr<char[]> buffer, size_t len);
    size_t sizeTracks() const;
};

#endif /* __WS_SOCKET_H__ */

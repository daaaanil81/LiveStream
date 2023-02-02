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

/**
 * Class Wrapper for linux socket.
 */
class WSServerFacade {
    //! Alias maps of WebRTC connections.
    using MapPeerConnections =
        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>>;
    //! Alias maps of websocket connections.
    using MapConnections =
        std::unordered_map<int, std::shared_ptr<rtc::WebSocket>>;
    //! Alias maps of WebRTC tracks.
    using MapTracks = std::unordered_map<int, std::shared_ptr<rtc::Track>>;

  private:
    //! Object of Websocket server.
    rtc::WebSocketServer server_;
    //! Map of WebRTC connections.
    static MapPeerConnections mapPeerConnections_;
    //! Map of websocket connections.
    static MapConnections mapConnections_;
    //! Map of WebRTC tracks.
    static MapTracks mapTracks_;
    //! ID of last and next client.
    static int index_clients_;
    //! Count of enable tracks.
    static std::atomic<size_t> sizeTracks_;
    //! Description of current video stream.
    static std::string sdp_;
    //! Mutext for new connections.
    static std::mutex m_;
    //! SSRC for current video stream.
    static const rtc::SSRC ssrc = 42;
    //! Payload of RTP stream.
    static const int payload_number = 96;

    //! Callback of new connection.
    /*!
     * \param ws Object of new connection.
     */
    static void onClientCallback(std::shared_ptr<rtc::WebSocket> ws);
    //! Generate and set local description.
    /*!
     * \param index Index of connection.
     */
    static void setLocalDescription(int index);
    //! Event for setting of remote description.
    /*!
     * \param index Index of connection.
     * \param sdp Remote description.
     */
    static void setRemoteDescription(int index, const std::string &sdp);
    //! Processing of errors from client.
    /*!
     * \param index Index of connection.
     * \param error Text of errors.
     */
    static void errorProcessing(int index, const std::string &error);
    //! Add new WebRTC connection to map.
    /*!
     * \param index Index of connection.
     * \param pc WebRTC connection.
     */
    static void addPeerConnection(int index,
                                  std::shared_ptr<rtc::PeerConnection> pc);
    //! Add new Websocket connection to map.
    /*!
     * \param index Index of connection.
     * \param ws Websocket connection.
     */
    static void addWebSocketConnection(int index,
                                       std::shared_ptr<rtc::WebSocket> ws);
    //! Close WebRTC and websocket connections and delete them from maps.
    /*!
     * \param index Index of connection.
     */
    static void disconnect(int index);

  public:
    //! Contructor of Websocket server.
    /*!
     * \param config Object with configuration information.
     * Port, enabled TLS, certificates.
     */
    WSServerFacade(rtc::WebSocketServer::Configuration config);
    //! Set video description.
    /*!
     * \param sdp Description of video.
     */
    void setVideoDescription(const std::string &sdp);
    //! Send RTP packages into each track.
    /*!
     * \param buffer RTP package in bytes.
     * \param len Len of package.
     */
    bool send(std::shared_ptr<char[]> buffer, size_t len);
    //! Return count of enabled tracks.
    /*!
     * \return Count of enabled tracks.
     */
    size_t sizeTracks() const;
};

#endif /* __WS_SOCKET_H__ */

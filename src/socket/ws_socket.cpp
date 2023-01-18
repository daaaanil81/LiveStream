#include "socket/ws_socket.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class MsgType { ID = 0, SDP, ERROR };

std::map<std::string, MsgType> string2type = {
    {"id", MsgType::ID}, {"sdp", MsgType::SDP}, {"error", MsgType::ERROR}};

std::mutex WSServerFacade::m_;
std::string WSServerFacade::sdp_;
std::atomic<size_t> WSServerFacade::sizeTracks_ = 0;
int WSServerFacade::index_clients_;
WSServerFacade::MapConnections WSServerFacade::mapConnections_;
WSServerFacade::MapPeerConnections WSServerFacade::mapPeerConnections_;
WSServerFacade::MapTracks WSServerFacade::mapTracks_;

WSServerFacade::WSServerFacade(rtc::WebSocketServer::Configuration config)
    : server_(config) {
    index_clients_ = 0;
    server_.onClient(onClientCallback);
}

size_t WSServerFacade::sizeTracks() const { return sizeTracks_.load(); }

void WSServerFacade::setVideoDescription(const std::string &sdp) { sdp_ = sdp; }

void WSServerFacade::setLocalDescription(int index) {
    std::lock_guard<std::mutex> lm(m_);
    if (!mapPeerConnections_[index]) {
        std::cout << "Index isn't valid" << std::endl;
        return;
    }

    auto pc = mapPeerConnections_[index];

    // TODO: Parse video sdp_

    rtc::Description::Video media("video",
                                  rtc::Description::Direction::SendOnly);
    media.addH264Codec(payload_number);
    media.addSSRC(ssrc, "video-send");
    auto track = pc->addTrack(media);
    pc->setLocalDescription();

    mapTracks_.insert({index, track});
    sizeTracks_++;
}

void WSServerFacade::setRemoteDescription(int index, const std::string &sdp) {
    std::lock_guard<std::mutex> lm(m_);
    if (sdp.empty()) {
        std::cout << "SDP isn't correct" << std::endl;
        return;
    }

    if (!mapPeerConnections_[index]) {
        std::cout << "Index isn't valid" << std::endl;
        return;
    }

    auto pc = mapPeerConnections_[index];
    auto ws = mapConnections_[index];

    std::cout << sdp << std::endl;
    rtc::Description answer(sdp, "answer");
    pc->setRemoteDescription(answer);
}

void WSServerFacade::addPeerConnection(
    int index, std::shared_ptr<rtc::PeerConnection> pc) {

    m_.lock();
    mapPeerConnections_.insert({index, pc});
    m_.unlock();
}

void WSServerFacade::addWebSocketConnection(
    int index, std::shared_ptr<rtc::WebSocket> ws) {

    m_.lock();
    mapConnections_.insert({index, ws});
    m_.unlock();
}

void WSServerFacade::disconnect(int index) {
    auto pc = mapPeerConnections_[index];
    pc->close();

    auto ws = mapConnections_[index];
    ws->close();

    mapPeerConnections_.erase(index);
    mapConnections_.erase(index);
    mapTracks_.erase(index);
}

void WSServerFacade::errorProcessing(int index, const std::string &error) {
    std::lock_guard<std::mutex> lm(m_);
    if (error.empty()) {
        std::cout << "Error isn't correct" << std::endl;
        return;
    }

    if (!mapPeerConnections_[index]) {
        std::cout << "Index isn't valid" << std::endl;
        return;
    }

    std::cout << error << std::endl;

    disconnect(index);
}

void WSServerFacade::onClientCallback(std::shared_ptr<rtc::WebSocket> ws) {

    addWebSocketConnection(++index_clients_, ws);

    ws->onOpen([ws, index = index_clients_]() {
        std::cout << "Client: " << index << " connected, signaling ready"
                  << std::endl;

        auto pc = std::make_shared<rtc::PeerConnection>();
        pc->onStateChange([index = index](rtc::PeerConnection::State state) {
            std::cout << "RTC Client: " << index << " state: " << state
                      << std::endl;
        });

        pc->onGatheringStateChange(
            [pc, ws, index = index](rtc::PeerConnection::GatheringState state) {
                std::cout << "Gathering State: " << state << std::endl;
                if (state == rtc::PeerConnection::GatheringState::Complete) {
                    auto description = pc->localDescription();
                    json message = {{"type", "sdp"},
                                    {"sdp", std::string(description.value())}};
                    std::cout << std::string(description.value()) << std::endl;
                    ws->send(message.dump());
                }
            });
        addPeerConnection(index, pc);

        setLocalDescription(index);

        json j = {{"type", "id"}, {"id", index}};

        ws->send(j.dump());
    });

    ws->onClosed([index = index_clients_]() {
        std::lock_guard<std::mutex> lm(m_);
        std::cout << "Client: " << index << " closed" << std::endl;
        disconnect(index);
    });

    ws->onError([index = index_clients_](const std::string &error) {
        std::cout << "Client: " << index << " failed: " << error << std::endl;
    });

    ws->onMessage([&](std::variant<rtc::binary, std::string> data) {
        json j = json::parse(std::get<std::string>(data));

        MsgType type = string2type[j["type"]];
        switch (type) {
        case MsgType::ID:
            break;
        case MsgType::SDP:
            setRemoteDescription(j["id"], j["sdp"]);
            break;
        case MsgType::ERROR:
            errorProcessing(j["id"], j["error"]);
            break;
        default:
            break;
        }
    });
}

bool WSServerFacade::send(std::shared_ptr<char[]> buffer, size_t len) {
    if (len < sizeof(rtc::RtpHeader)) {
        return false;
    }

    auto rtp = reinterpret_cast<rtc::RtpHeader *>(buffer.get());
    rtp->setSsrc(ssrc);

    {
        std::lock_guard<std::mutex> lm(m_);
        for (auto &[index, track] : mapTracks_) {
            if (!track->isOpen()) {
                continue;
            }
            std::cout << len << std::endl;

            track->send(reinterpret_cast<const std::byte *>(buffer.get()), len);
        }
    }

    return true;
}

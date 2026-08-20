#include "mesh.h"

#include <stdexcept>
#include <utility>

namespace {
constexpr size_t PACKET_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint8_t);

uint32_t readUint32(const std::string& data) {
    return (static_cast<uint32_t>(static_cast<uint8_t>(data[0])) << 24) |
           (static_cast<uint32_t>(static_cast<uint8_t>(data[1])) << 16) |
           (static_cast<uint32_t>(static_cast<uint8_t>(data[2])) << 8) |
           static_cast<uint32_t>(static_cast<uint8_t>(data[3]));
}
}

std::string serialize(const MeshPacket& packet) {
    if (packet.ttl > MAX_TTL) {
        throw std::invalid_argument("mesh packet TTL must be at most 127");
    }

    std::string data(PACKET_HEADER_SIZE, '\0');
    data[0] = static_cast<char>((packet.msgId >> 24) & 0xFF);
    data[1] = static_cast<char>((packet.msgId >> 16) & 0xFF);
    data[2] = static_cast<char>((packet.msgId >> 8) & 0xFF);
    data[3] = static_cast<char>(packet.msgId & 0xFF);
    data[4] = static_cast<char>(packet.ttl);
    data += packet.payload;
    return data;
}

MeshPacket parse(const std::string& advertisement) {
    if (advertisement.size() < PACKET_HEADER_SIZE) {
        throw std::invalid_argument("mesh advertisement is too short");
    }

    const auto ttl = static_cast<uint8_t>(advertisement[4]);
    if (ttl > MAX_TTL) {
        throw std::invalid_argument("mesh packet TTL is invalid");
    }

    return MeshPacket{readUint32(advertisement), ttl,
                      advertisement.substr(PACKET_HEADER_SIZE)};
}

MeshNode::MeshNode(MeshTransport& transport, MessageHandler handler)
    : transport(transport), handler(std::move(handler)) {}

void MeshNode::start() {
    seenIds.clear();
    transport.start([this](const std::string& advertisement) {
        onScanResult(advertisement);
    });
}

void MeshNode::stop() {
    transport.stop();
}

void MeshNode::sendSos(uint32_t msgId, const std::string& message, uint8_t ttl) {
    MeshPacket packet{msgId, ttl, message};
    if (!seenIds.insert(msgId).second) {
        return;
    }
    deliver(packet);
    broadcast(packet);
}

void MeshNode::broadcast(const MeshPacket& packet) {
    transport.send(serialize(packet));
}

void MeshNode::relay(const MeshPacket& packet) {
    if (packet.ttl == 0) {
        return;
    }
    MeshPacket relayed = packet;
    --relayed.ttl;
    broadcast(relayed);
}

void MeshNode::deliver(const MeshPacket& packet) {
    if (handler) {
        handler(packet);
    }
}

void MeshNode::onScanResult(const std::string& advertisement) {
    MeshPacket packet;
    try {
        packet = parse(advertisement);
    } catch (const std::invalid_argument&) {
        return;
    }

    if (!seenIds.insert(packet.msgId).second) {
        return;
    }

    deliver(packet);
    relay(packet);
}


 

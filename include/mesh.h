#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_set>

class MeshTransport {
public:
    using ReceiveCallback = std::function<void(const std::string&)>;

    virtual ~MeshTransport() = default;
    virtual void start(ReceiveCallback callback) = 0;
    virtual void stop() = 0;
    virtual void send(const std::string& advertisement) = 0;
};

struct MeshPacket {
    uint32_t msgId;
    uint8_t ttl;
    std::string payload;
};

using MessageHandler = std::function<void(const MeshPacket&)>;

constexpr uint8_t MAX_TTL = 127;

std::string serialize(const MeshPacket& packet);
MeshPacket parse(const std::string& advertisement);

class MeshNode {
public:
    explicit MeshNode(MeshTransport& transport, MessageHandler handler = {});

    void start();
    void stop();
    void sendSos(uint32_t msgId, const std::string& message, uint8_t ttl = MAX_TTL);
    void onScanResult(const std::string& advertisement);

private:
    void broadcast(const MeshPacket& packet);
    void deliver(const MeshPacket& packet);
    void relay(const MeshPacket& packet);

    MeshTransport& transport;
    MessageHandler handler;
    std::unordered_set<uint32_t> seenIds;
};
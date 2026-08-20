#include "loopback_transport.h"
#include "mobile_ble_transport.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    LoopbackChannel channel;
    LoopbackTransport firstTransport(channel);
    LoopbackTransport secondTransport(channel);
    std::vector<MeshPacket> firstMessages;
    std::vector<MeshPacket> secondMessages;
    MeshNode first(firstTransport, [&](const MeshPacket& packet) {
        firstMessages.push_back(packet);
    });
    MeshNode second(secondTransport, [&](const MeshPacket& packet) {
        secondMessages.push_back(packet);
    });

    first.start();
    second.start();
    first.sendSos(42, "Need assistance", 2);

    assert(firstMessages.size() == 1);
    assert(secondMessages.size() == 1);
    assert(secondMessages[0].msgId == 42);
    assert(secondMessages[0].ttl == 2);
    assert(secondMessages[0].payload == "Need assistance");

    first.onScanResult(serialize(MeshPacket{42, 2, "Need assistance"}));
    assert(firstMessages.size() == 1);
    assert(secondMessages.size() == 1);

    bool rejected = false;
    try {
        parse("bad");
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    assert(parse(serialize(MeshPacket{7, 0, "ok"})).payload == "ok");

    std::string mobileAdvertisement;
    std::vector<MeshPacket> mobileMessages;
    MobileBleTransport mobileTransport([&](const std::string& advertisement) {
        mobileAdvertisement = advertisement;
    });
    MeshNode mobileNode(mobileTransport, [&](const MeshPacket& packet) {
        mobileMessages.push_back(packet);
    });
    mobileNode.start();
    mobileNode.sendSos(99, "Mobile SOS", 1);
    assert(!mobileAdvertisement.empty());
    assert(mobileMessages.size() == 1);
    mobileNode.onScanResult(mobileAdvertisement);
    assert(mobileMessages.size() == 1);
    mobileNode.stop();

    return 0;
}

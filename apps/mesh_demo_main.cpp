#include "loopback_transport.h"

#include <iostream>

int main() {
    LoopbackChannel channel;
    LoopbackTransport senderTransport(channel);
    LoopbackTransport receiverTransport(channel);

    MeshNode sender(senderTransport);
    MeshNode receiver(receiverTransport, [](const MeshPacket& packet) {
        std::cout << "Received SOS " << packet.msgId << ": " << packet.payload
                  << " (ttl=" << static_cast<unsigned>(packet.ttl) << ")\n";
    });

    sender.start();
    receiver.start();
    sender.sendSos(1, "Need assistance", 3);
    sender.stop();
    receiver.stop();
}

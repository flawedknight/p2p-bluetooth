#include "windows_ble_transport.h"
#include <winrt/base.h>

#include <iostream>

int main() {
    winrt::init_apartment();

    WindowsBleTransport transport;
    MeshNode node(transport, [](const MeshPacket& packet) {
        std::cout << "Received SOS " << packet.msgId << ": " << packet.payload
                  << " (ttl=" << static_cast<unsigned>(packet.ttl) << ")\n";
    });

    node.start();
    std::cout << "Windows Bluetooth receiver active. Press Enter to stop.\n";
    std::cin.get();
    node.stop();
}

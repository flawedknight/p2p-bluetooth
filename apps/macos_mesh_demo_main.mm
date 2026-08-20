#import <Foundation/Foundation.h>

#include "macos_ble_transport.h"

#include <iostream>

int main() {
    @autoreleasepool {
        MacBleTransport transport;
        MeshNode node(transport, [](const MeshPacket& packet) {
            std::cout << "Received SOS " << packet.msgId << ": " << packet.payload
                      << " (ttl=" << static_cast<unsigned>(packet.ttl) << ")\n";
        });

        node.start();
        std::cout << "Bluetooth mesh active. Press Enter to send a test SOS, then Ctrl-C to stop.\n";
        std::cin.get();
        node.sendSos(1, "Need help", 3);
        std::cout << "Test SOS advertised. Press Enter to exit.\n";
        std::cin.get();
        node.stop();
    }
}
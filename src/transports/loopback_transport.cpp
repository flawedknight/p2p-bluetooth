#include "loopback_transport.h"

#include <algorithm>
#include <utility>

LoopbackTransport::LoopbackTransport(LoopbackChannel& channel) : channel(channel) {
    channel.attach(this);
}

LoopbackTransport::~LoopbackTransport() {
    channel.detach(this);
}

void LoopbackTransport::start(ReceiveCallback receiveCallback) {
    callback = std::move(receiveCallback);
    active = true;
}

void LoopbackTransport::stop() {
    active = false;
    callback = {};
}

void LoopbackTransport::send(const std::string& advertisement) {
    if (active) {
        channel.broadcast(this, advertisement);
    }
}

void LoopbackChannel::attach(LoopbackTransport* transport) {
    transports.push_back(transport);
}

void LoopbackChannel::detach(LoopbackTransport* transport) {
    transports.erase(std::remove(transports.begin(), transports.end(), transport), transports.end());
}

void LoopbackChannel::broadcast(LoopbackTransport* sender, const std::string& advertisement) {
    for (LoopbackTransport* transport : transports) {
        if (transport != sender && transport->active && transport->callback) {
            transport->callback(advertisement);
        }
    }
}
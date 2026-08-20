#pragma once

#include "mesh.h"

#include <string>
#include <vector>

class LoopbackChannel;

class LoopbackTransport final : public MeshTransport {
public:
    explicit LoopbackTransport(LoopbackChannel& channel);
    ~LoopbackTransport() override;

    void start(ReceiveCallback callback) override;
    void stop() override;
    void send(const std::string& advertisement) override;

private:
    friend class LoopbackChannel;

    LoopbackChannel& channel;
    ReceiveCallback callback;
    bool active = false;
};

class LoopbackChannel {
public:
    void attach(LoopbackTransport* transport);
    void detach(LoopbackTransport* transport);
    void broadcast(LoopbackTransport* sender, const std::string& advertisement);

private:
    std::vector<LoopbackTransport*> transports;
};
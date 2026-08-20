#pragma once

#include "mesh.h"

#include <memory>

class MacBleTransport final : public MeshTransport {
public:
    class Impl;

    MacBleTransport();
    ~MacBleTransport() override;

    MacBleTransport(const MacBleTransport&) = delete;
    MacBleTransport& operator=(const MacBleTransport&) = delete;

    void start(ReceiveCallback callback) override;
    void stop() override;
    void send(const std::string& advertisement) override;

private:
    std::unique_ptr<Impl> impl;
};

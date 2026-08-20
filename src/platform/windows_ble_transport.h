#pragma once

#include "mesh.h"

#include <memory>

class WindowsBleTransport final : public MeshTransport {
public:
    WindowsBleTransport();
    ~WindowsBleTransport() override;

    WindowsBleTransport(const WindowsBleTransport&) = delete;
    WindowsBleTransport& operator=(const WindowsBleTransport&) = delete;

    void start(ReceiveCallback callback) override;
    void stop() override;
    void send(const std::string& advertisement) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

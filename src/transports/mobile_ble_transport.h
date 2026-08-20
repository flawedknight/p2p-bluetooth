#pragma once

#include "mesh.h"

#include <functional>
#include <string>

namespace mobile_ble {
constexpr const char* SERVICE_UUID = "7f7d0001-5c3a-4b2a-9b0d-534f534d4553";
constexpr const char* SOS_CHARACTERISTIC_UUID = "7f7d0002-5c3a-4b2a-9b0d-534f534d4553";
}

class MobileBleTransport final : public MeshTransport {
public:
    using AdvertiseCallback = std::function<void(const std::string&)>;

    explicit MobileBleTransport(AdvertiseCallback advertiseCallback = {});

    void setAdvertiseCallback(AdvertiseCallback callback);
    void start(ReceiveCallback callback) override;
    void stop() override;
    void send(const std::string& advertisement) override;

    // Native BLE code calls this from its characteristic/advertisement callback.
    void onBleAdvertisement(const std::string& advertisement);

private:
    AdvertiseCallback advertiseCallback;
    ReceiveCallback receiveCallback;
    bool active = false;
};
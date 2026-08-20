#include "mobile_ble_transport.h"

#include <utility>

MobileBleTransport::MobileBleTransport(AdvertiseCallback advertiseCallback)
    : advertiseCallback(std::move(advertiseCallback)) {}

void MobileBleTransport::setAdvertiseCallback(AdvertiseCallback callback) {
    advertiseCallback = std::move(callback);
}

void MobileBleTransport::start(ReceiveCallback callback) {
    receiveCallback = std::move(callback);
    active = true;
}

void MobileBleTransport::stop() {
    active = false;
    receiveCallback = {};
}

void MobileBleTransport::send(const std::string& advertisement) {
    if (active && advertiseCallback) {
        advertiseCallback(advertisement);
    }
}

void MobileBleTransport::onBleAdvertisement(const std::string& advertisement) {
    if (active && receiveCallback) {
        receiveCallback(advertisement);
    }
}
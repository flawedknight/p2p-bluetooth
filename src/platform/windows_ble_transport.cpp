#include "windows_ble_transport.h"

#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/base.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace winrt;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Storage::Streams;

namespace {
constexpr uint16_t TEST_COMPANY_ID = 0xFFFE;
constexpr uint8_t PACKET_MARKER[] = {0x53, 0x4F};
constexpr size_t MARKER_SIZE = sizeof(PACKET_MARKER);
constexpr size_t MIN_PACKET_SIZE = 5;
constexpr wchar_t SERVICE_UUID[] = L"7f7d0001-5c3a-4b2a-9b0d-534f534d4553";

std::vector<uint8_t> readBuffer(IBuffer buffer) {
    DataReader reader = DataReader::FromBuffer(buffer);
    std::vector<uint8_t> bytes(buffer.Length());
    if (!bytes.empty()) {
        reader.ReadBytes(bytes);
    }
    return bytes;
}

bool hasMarker(const std::vector<uint8_t>& bytes) {
    return bytes.size() >= MARKER_SIZE && bytes[0] == PACKET_MARKER[0] &&
           bytes[1] == PACKET_MARKER[1];
}
}

class WindowsBleTransport::Impl {
public:
    Impl() {
        watcher.ScanningMode(BluetoothLEScanningMode::Active);
        watcher.Received({this, &Impl::onReceived});
        publisher = BluetoothLEAdvertisementPublisher();
    }

    void start(MeshTransport::ReceiveCallback receiveCallback) {
        callback = std::move(receiveCallback);
        active = true;
        watcher.Start();
    }

    void stop() {
        active = false;
        watcher.Stop();
        publisher.Stop();
        callback = {};
    }

    void send(const std::string& packet) {
        if (packet.size() + MARKER_SIZE > 24) {
            return;
        }

        BluetoothLEAdvertisement advertisement;
        advertisement.ServiceUuids().Append(Guid(SERVICE_UUID));
        BluetoothLEManufacturerData manufacturer(TEST_COMPANY_ID);
        std::vector<uint8_t> bytes(PACKET_MARKER, PACKET_MARKER + MARKER_SIZE);
        bytes.insert(bytes.end(), packet.begin(), packet.end());
        DataWriter writer;
        writer.WriteBytes(bytes);
        manufacturer.Data(writer.DetachBuffer());
        advertisement.ManufacturerData().Append(manufacturer);
        publisher.Advertisement(advertisement);
        publisher.Start();
    }

private:
    void onReceived(BluetoothLEAdvertisementWatcher const&,
                    BluetoothLEAdvertisementReceivedEventArgs const& args) {
        if (!active || !callback) {
            return;
        }

        auto advertisement = args.Advertisement();
        bool matchingService = false;
        for (auto uuid : advertisement.ServiceUuids()) {
            if (uuid == Guid(SERVICE_UUID)) {
                matchingService = true;
                break;
            }
        }
        if (!matchingService) {
            return;
        }

        for (auto manufacturer : advertisement.ManufacturerData()) {
            auto bytes = readBuffer(manufacturer.Data());
            if (hasMarker(bytes)) {
                callback(std::string(reinterpret_cast<const char*>(bytes.data() + MARKER_SIZE),
                                     bytes.size() - MARKER_SIZE));
                continue;
            }
            // macOS may expose the marker as the manufacturer company ID and return
            // only the packet bytes in Data.
            if (bytes.size() >= MIN_PACKET_SIZE) {
                callback(std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
            }
        }
    }

    BluetoothLEAdvertisementWatcher watcher;
    BluetoothLEAdvertisementPublisher publisher;
    MeshTransport::ReceiveCallback callback;
    bool active = false;
};

WindowsBleTransport::WindowsBleTransport() : impl(std::make_unique<Impl>()) {}

WindowsBleTransport::~WindowsBleTransport() = default;

void WindowsBleTransport::start(ReceiveCallback callback) {
    impl->start(std::move(callback));
}

void WindowsBleTransport::stop() {
    impl->stop();
}

void WindowsBleTransport::send(const std::string& advertisement) {
    impl->send(advertisement);
}

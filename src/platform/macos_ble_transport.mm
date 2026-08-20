#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include "macos_ble_transport.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace {
NSString* const kServiceUuidString = @"7f7d0001-5c3a-4b2a-9b0d-534f534d4553";
const uint8_t kPacketMarker[] = {0x53, 0x4f};
constexpr size_t kMarkerSize = sizeof(kPacketMarker);
constexpr size_t kMaxManufacturerData = 24;
}

@interface MacBleDelegate : NSObject <CBCentralManagerDelegate, CBPeripheralManagerDelegate>
@property(nonatomic, assign) MacBleTransport::Impl* owner;
@end

class MacBleTransport::Impl {
public:
    Impl() {
        delegate = [[MacBleDelegate alloc] init];
        delegate.owner = this;
        central = [[CBCentralManager alloc] initWithDelegate:delegate queue:dispatch_get_main_queue() options:nil];
        peripheral = [[CBPeripheralManager alloc] initWithDelegate:delegate queue:dispatch_get_main_queue()];
    }

    ~Impl() {
        stop();
        central.delegate = nil;
        peripheral.delegate = nil;
        delegate.owner = nullptr;
    }

    void start(MeshTransport::ReceiveCallback receiveCallback) {
        callback = std::move(receiveCallback);
        active = true;
        scanWhenReady = true;
        advertiseWhenReady = true;
        startBluetoothWork();
    }

    void stop() {
        active = false;
        scanWhenReady = false;
        advertiseWhenReady = false;
        if (central.isScanning) {
            [central stopScan];
        }
        [peripheral stopAdvertising];
        callback = {};
    }

    void send(const std::string& packet) {
        if (packet.size() + kMarkerSize > kMaxManufacturerData) {
            NSLog(@"SOS packet was not advertised: payload is too large for BLE advertising");
            return;
        }
        pendingPacket = packet;
        advertiseWhenReady = true;
        startBluetoothWork();
    }

    void onCentralState(CBManagerState state) {
        if (state == CBManagerStatePoweredOn && active) {
            startBluetoothWork();
        }
    }

    void onPeripheralState(CBManagerState state) {
        if (state == CBManagerStatePoweredOn && active) {
            startBluetoothWork();
        }
    }

    void onAdvertisement(NSDictionary<NSString*, id>* advertisementData) {
        if (!active || !callback) {
            return;
        }
        NSData* data = advertisementData[CBAdvertisementDataManufacturerDataKey];
        if (data.length <= kMarkerSize) {
            return;
        }
        const auto* bytes = static_cast<const uint8_t*>(data.bytes);
        size_t offset = 0;
        if (bytes[0] == kPacketMarker[0] && bytes[1] == kPacketMarker[1]) {
            offset = kMarkerSize;
        } else if (data.length > 4 && bytes[0] == 0xFE && bytes[1] == 0xFF &&
                   bytes[2] == kPacketMarker[0] && bytes[3] == kPacketMarker[1]) {
            offset = 4;
        } else {
            return;
        }
        callback(std::string(reinterpret_cast<const char*>(bytes + offset), data.length - offset));
    }

private:
    void startBluetoothWork() {
        if (!active) {
            return;
        }
        if (central.state == CBManagerStatePoweredOn && !central.isScanning) {
            [central scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:kServiceUuidString]]
                                            options:@{CBCentralManagerScanOptionAllowDuplicatesKey: @NO}];
            scanWhenReady = false;
        }
        if (peripheral.state == CBManagerStatePoweredOn && advertiseWhenReady && !pendingPacket.empty()) {
            NSMutableData* data = [NSMutableData dataWithBytes:kPacketMarker length:kMarkerSize];
            [data appendBytes:pendingPacket.data() length:pendingPacket.size()];
            [peripheral stopAdvertising];
            [peripheral startAdvertising:@{
                CBAdvertisementDataServiceUUIDsKey: @[[CBUUID UUIDWithString:kServiceUuidString]],
                CBAdvertisementDataManufacturerDataKey: data
            }];
            advertiseWhenReady = false;
        }
    }

    MacBleDelegate* delegate = nil;
    CBCentralManager* central = nil;
    CBPeripheralManager* peripheral = nil;
    MeshTransport::ReceiveCallback callback;
    std::string pendingPacket;
    bool active = false;
    bool scanWhenReady = false;
    bool advertiseWhenReady = false;
};

@implementation MacBleDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager*)central {
    if (self.owner != nullptr) {
        self.owner->onCentralState(central.state);
    }
}

- (void)centralManager:(CBCentralManager*)central
 didDiscoverPeripheral:(CBPeripheral*)peripheral
     advertisementData:(NSDictionary<NSString*, id>*)advertisementData
                  RSSI:(NSNumber*)RSSI {
    (void)central;
    (void)peripheral;
    (void)RSSI;
    if (self.owner != nullptr) {
        self.owner->onAdvertisement(advertisementData);
    }
}

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager*)peripheral {
    if (self.owner != nullptr) {
        self.owner->onPeripheralState(peripheral.state);
    }
}

@end

MacBleTransport::MacBleTransport() : impl(std::make_unique<Impl>()) {}

MacBleTransport::~MacBleTransport() = default;

void MacBleTransport::start(ReceiveCallback callback) {
    impl->start(std::move(callback));
}

void MacBleTransport::stop() {
    impl->stop();
}

void MacBleTransport::send(const std::string& advertisement) {
    impl->send(advertisement);
}

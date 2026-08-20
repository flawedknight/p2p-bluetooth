# P2P Bluetooth SOS Mesh

This project contains the portable protocol core for forwarding SOS messages over a Bluetooth-style advertisement/scanning transport.

## Packet format

Each advertisement is binary and uses this big-endian layout:

| Bytes | Field |
| --- | --- |
| 0-3 | Message ID (`uint32_t`) |
| 4 | TTL (`0`-`127`) |
| 5+ | SOS payload bytes |

A node delivers a message once per message ID. A received message with a nonzero TTL is relayed with its TTL decremented. The transport does not send a packet back to the node that emitted it; duplicate suppression prevents loops across multiple hops.

## Build

Requirements: C++17 and CMake 3.16 or newer.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/mesh_demo
```

The repository is organized as follows:

- `include/`: public C++ headers shared by every platform.
- `src/`: protocol core and transport implementations.
- `apps/`: executable entry points for the demo, macOS, and Windows.
- `tests/`: automated protocol tests.
- `frontend/`: browser simulator.
- `docs/`: platform metadata and notes.

## Frontend simulator

Open `frontend/index.html` directly in a browser, or serve the project root with:

```sh
python3 -m http.server 8080
```

Then visit `http://localhost:8080/frontend/`. Use **Broadcast SOS** to create a mobile-originated packet. Use **Use sample** and **Inject scan** to simulate a packet received from a nearby phone and inspect its relay event.

## macOS Bluetooth

On macOS, the native CoreBluetooth adapter is available in `src/platform/macos_ble_transport.mm`. It scans for the project service UUID and advertises the packet as manufacturer data with the `SO` app marker. Packets are limited to 24 bytes in that BLE advertisement path, leaving approximately 17 bytes for the SOS payload after the marker and packet header. Oversized packets are rejected with a log message instead of crashing the app.

Build the native demo with CMake on a Mac that has CMake installed:

```sh
cmake -S . -B build
cmake --build build --target macos_mesh_demo
open build/macos_mesh_demo.app
```

If CMake is not installed, build the same app directly with the Apple toolchain:

```sh
mkdir -p build/macos_mesh_demo.app/Contents/MacOS
xcrun clang++ -std=c++17 -Wall -Wextra -Werror -pedantic -fobjc-arc \
	-isysroot "$(xcrun --show-sdk-path)" -Iinclude -Isrc/platform \
	src/mesh.cpp src/platform/macos_ble_transport.mm apps/macos_mesh_demo_main.mm \
	-framework Foundation -framework CoreBluetooth \
	-o build/macos_mesh_demo.app/Contents/MacOS/macos_mesh_demo
cp docs/Info.plist build/macos_mesh_demo.app/Contents/Info.plist
open build/macos_mesh_demo.app
```

The demo asks for Bluetooth permission through `docs/Info.plist`. On two Macs, start the demo on each machine. Press Enter in one terminal to advertise the test SOS; the other Mac should print the received message and relay it. For a real mobile app, use the same service UUID and manufacturer-data payload with CoreBluetooth on iOS or BluetoothLeScanner/BluetoothGatt on Android.

## Windows receiver

The Windows implementation in `src/platform/windows_ble_transport.cpp` uses C++/WinRT `BluetoothLEAdvertisementWatcher` to receive the Mac packet and `BluetoothLEAdvertisementPublisher` to relay it. It uses the same service UUID, `SO` marker, packet format, and short-payload limit.

On Windows 10/11, install Visual Studio with **Desktop development with C++** and the C++/WinRT tooling, then build from a Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --target windows_mesh_receiver --config Release
build\Release\windows_mesh_receiver.exe
```

Enable Bluetooth and allow the app through Windows Bluetooth/privacy settings. Start the Windows receiver first, then run the Mac sender. When the Mac prints `Test SOS advertised.`, the Windows terminal should print:

```text
Received SOS 1: Need help (ttl=3)
```

If the Windows device receives the packet but does not print it, confirm that the Mac and Windows adapter are powered on and that both devices are within a few meters. This is a BLE advertisement receiver, so it does not require pairing.

The current demo uses `LoopbackTransport`, which simulates nearby Bluetooth nodes in one process. `MobileBleTransport` is the native mobile integration boundary. Your iOS or Android BLE layer should:

1. Create `MobileBleTransport` and pass an advertise callback that writes bytes to the SOS characteristic.
2. Construct `MeshNode` with that transport and call `start()` when scanning begins.
3. Pass every received SOS characteristic value to `onBleAdvertisement()`.
4. Call `sendSos()` when the user presses the mobile SOS button.
5. Call `stop()` when the screen/service or BLE session closes.

Use `mobile_ble::SERVICE_UUID` and `mobile_ble::SOS_CHARACTERISTIC_UUID` on both phones. The C++ layer is deliberately platform-neutral; CoreBluetooth on iOS and Android BluetoothGatt/BluetoothLeScanner on Android remain native wrappers around these callbacks.

## Files

- `include/mesh.h` and `src/mesh.cpp`: packet encoding plus node delivery, deduplication, TTL, and relay logic.
- `src/transports/loopback_transport.*`: deterministic in-process transport for tests and demos.
- `src/transports/mobile_ble_transport.*`: callback bridge for native mobile BLE code.
- `src/platform/macos_ble_transport.*`: macOS CoreBluetooth scanner/publisher.
- `src/platform/windows_ble_transport.*`: Windows BLE scanner/publisher for Mac-to-Windows reception and relay.
- `apps/windows_mesh_receiver_main.cpp`: Windows receiver executable entry point.
- `include/mesh_types.h` and `include/network.h`: Bluetooth Mesh address, key, nonce, and network PDU types.
- `tests/mesh_test.cpp`: protocol and relay behavior tests.
- `apps/mesh_demo_main.cpp`: two-node loopback demonstration.

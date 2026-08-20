# Windows BLE receiver

This folder is reserved for Windows-specific packaging. The receiver source is kept at the project root so it can share the portable mesh library and CMake target.

The application needs Bluetooth permission and an enabled Bluetooth adapter. Pairing is not required because the SOS packet is sent as a BLE advertisement.
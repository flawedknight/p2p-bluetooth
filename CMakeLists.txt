cmake_minimum_required(VERSION 3.16)
project(p2p_bluetooth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mesh_core
    src/mesh.cpp
    src/transports/loopback_transport.cpp
    src/transports/mobile_ble_transport.cpp
)
target_include_directories(mesh_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transports
)
target_compile_options(mesh_core PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(mesh_test tests/mesh_test.cpp)
target_link_libraries(mesh_test PRIVATE mesh_core)
target_compile_options(mesh_test PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(mesh_demo apps/mesh_demo_main.cpp)
target_link_libraries(mesh_demo PRIVATE mesh_core)
target_compile_options(mesh_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(APPLE)
    add_library(macos_ble_transport src/platform/macos_ble_transport.mm)
    target_include_directories(macos_ble_transport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
    )
    target_link_libraries(macos_ble_transport PUBLIC mesh_core
        "-framework Foundation"
        "-framework CoreBluetooth"
    )
    target_compile_options(macos_ble_transport PRIVATE -Wall -Wextra -Wpedantic -Werror -fobjc-arc)

    add_executable(macos_mesh_demo MACOSX_BUNDLE apps/macos_mesh_demo_main.mm)
    target_link_libraries(macos_mesh_demo PRIVATE macos_ble_transport)
    target_compile_options(macos_mesh_demo PRIVATE -Wall -Wextra -Wpedantic -Werror -fobjc-arc)
    set_target_properties(macos_mesh_demo PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/docs/Info.plist
    )
endif()

if(WIN32)
    add_library(windows_ble_transport src/platform/windows_ble_transport.cpp)
    target_include_directories(windows_ble_transport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
    )
    target_link_libraries(windows_ble_transport PUBLIC mesh_core
        windowsapp
    )
    target_compile_options(windows_ble_transport PRIVATE /W4 /WX)

    add_executable(windows_mesh_receiver apps/windows_mesh_receiver_main.cpp)
    target_link_libraries(windows_mesh_receiver PRIVATE windows_ble_transport)
    target_compile_options(windows_mesh_receiver PRIVATE /W4 /WX)
endif()

enable_testing()
add_test(NAME mesh_relay_test COMMAND mesh_test)

#include<cstdint>
#include<array>

using MeshAddress = uint16_t;
    
//   0x0000            -> unassigned
//   0x0001 - 0x7FFF    -> unicast (one specific node/element)
//   0x8000 - 0xBFFF    -> virtual (hashed from a 128-bit UUID, rare — skip for v1)
//   0xC000 - 0xFEFF    -> group(multicast)
//   0xFF00 - 0xFFFF    -> reserved / fixed group (all-nodes, all-relays, etc.)
constexpr MeshAddress ADDR_UNASSIGNED = 0x0000;
constexpr MeshAddress ADDR_ALL_PROXIES  = 0xFFFC;
constexpr MeshAddress ADDR_ALL_FRIENDS  = 0xFFFD;
constexpr MeshAddress ADDR_ALL_RELAYS = 0xFFEE;
constexpr MeshAddress ADDR_ALL_NODES = 0xFFFF;

inline bool isUnicast(MeshAddress a) {
    return a >= 0x0001 && a <= 0x7FFF;
}

inline bool isGroup(MeshAddress a)   { 
    return a >= 0xC000 && a <= 0xFFFF; 
}


// NetKey: shared by all nodes in a subnet. 
// AppKey: shared by nodes that run the same application
// DevKey: unique per node

using NetKey = std::array<uint8_t, 16>;
using AppKey = std::array<uint8_t, 16>;
using DevKey = std::array<uint8_t, 16>;


using KeyIndex = uint16_t;

// IVIndex changes rarely replay.

struct NodeState {
    uint32_t ivIndex = 0;
    uint32_t seqNum = 0;
    MeshAddress primaryUnicastAddr = ADDR_UNASSIGNED;
    DevKey devkey{};
};



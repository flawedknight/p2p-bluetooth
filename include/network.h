#include "mesh_types.h"
#include <vector>
#include <cstring>

// ivi, nid, ctl, ttl, seq, src/dst, netmic

struct NetworkPDU{
    uint8_t ivi;
    uint16_t nid;
    bool ctl;
    uint8_t ttl;
    uint32_t seq;
    MeshAddress src;
    MeshAddress dst;
    std::vector<uint8_t> encryptedTransportPdu;
    std::array<uint8_t, 8> netMic{};
};

struct NetworkNonce {
    std::array<uint8_t, 13> bytes{};
    static NetworkNonce build(bool ctl, uint8_t ttl, uint32_t seq, MeshAddress src, uint32_t ivIndex) {
        NetworkNonce n;
        n.bytes[0] = 0x00;
        n.bytes[1] = (ctl ? 0x80: 0x00) | (ttl & 0x7F);
        n.bytes[2] = (seq >> 16) & 0xFF;
        n.bytes[3] = (seq >> 8) & 0xFF;
        n.bytes[4] = seq & 0xFF;
        n.bytes[5] = (src >> 8) & 0xFF;
        n.bytes[6] = src & 0xFF;
        n.bytes[7] = 0x00;
        n.bytes[8] = 0x00;
        n.bytes[9] = (ivIndex >> 24) & 0xFF;
        n.bytes[10] = (ivIndex >> 16) & 0xFF;
        n.bytes[11] = (ivIndex >> 8) & 0xFF;
        n.bytes[12] = ivIndex & 0xFF;
        
        return n;
    }



};
#include "Game/Player/GhostPacket.hpp"

#include <cassert>
#include <cstdint>

int main() {
    const u8 bytes[] = {
        0xFE,                         // signed byte -2
        0x12, 0x34,                   // signed short 0x1234
        0x89, 0xAB, 0xCD, 0xEF,       // unsigned word
        0x80, 0xFF, 0x7F,             // signed vector (-128, -1, 127)
        0xFF, 0xFE, 0x00, 0x01, 0x80, 0x00, // signed vector (-2, 1, -32768)
        'r', 'u', 'n', '\0', 'i', 'd', 'l', 'e', '\0'
    };

    GhostPacket packet(const_cast<u8*>(bytes), 0);
    s8 signedByte = 0;
    packet.read(&signedByte);
    assert(signedByte == -2 && packet.mCurOffs == 1);

    s16 signedShort = 0;
    packet.read(&signedShort);
    assert(signedShort == 0x1234 && packet.mCurOffs == 3);

    u32 word = 0;
    packet.read(&word);
    assert(word == 0x89ABCDEFu && packet.mCurOffs == 7);

    TVec3Sc byteVector;
    packet.read(&byteVector);
    assert(byteVector.x == -128 && byteVector.y == -1 && byteVector.z == 127);
    assert(packet.mCurOffs == 10);

    TVec3s shortVector;
    packet.read(&shortVector);
    assert(shortVector.x == -2 && shortVector.y == 1 && shortVector.z == -32768);
    assert(packet.mCurOffs == 16);

    char* text = nullptr;
    packet.read(&text);
    assert(text != nullptr && text[0] == 'r' && text[3] == '\0');
    assert(packet.mCurOffs == 20);
    packet.read(&text);
    assert(text != nullptr && text[0] == 'i' && text[4] == '\0');
    assert(packet.mCurOffs == sizeof(bytes));

    // The game passes zero for streaming records and one for a one-byte lookahead.
    GhostPacket streaming(const_cast<u8*>(bytes), 0);
    GhostPacket lookahead(const_cast<u8*>(bytes + 3), 1);
    lookahead.read(&signedByte);
    assert(signedByte == static_cast<s8>(0x89) && lookahead.mCurOffs == 1);
    streaming.read(&signedByte);
    assert(signedByte == -2 && streaming.mCurOffs == 1);
    return 0;
}

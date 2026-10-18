#include <Native/LayoutMaterialResource.hpp>
#include <Native/LayoutVertexResource.hpp>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>

void word(void* target, u32 value) {
    auto* bytes = static_cast<u8*>(target);
    for (unsigned i = 0; i < 4; ++i) bytes[i] = u8(value >> (24 - i * 8));
}
int main() {
    u32 packedColor{}; word(&packedColor, 0x12345678);
    const auto vertexColor = Native::readLayoutVertexColor(packedColor);
    assert(vertexColor.r == 0x12 && vertexColor.g == 0x34 && vertexColor.b == 0x56 && vertexColor.a == 0x78);
    assert(u32(vertexColor) == 0x12345678);
    const auto savedColor = packedColor;
    nw4r::math::VEC2 rawUv{};
    const std::array<float, 2> expectedUv{-0.125f, 0.875f};
    for (unsigned i=0;i<2;++i) {u32 bits; std::memcpy(&bits,&expectedUv[i],4); word(reinterpret_cast<u8*>(&rawUv)+i*4,bits);}
    const auto savedUv=rawUv;
    const auto uv=Native::readLayoutTexCoord(rawUv);
    assert(uv.x == expectedUv[0] && uv.y == expectedUv[1]);
    assert(packedColor == savedColor && std::memcmp(&rawUv,&savedUv,sizeof(rawUv))==0);
    using namespace nw4r::lyt;
    res::MaterialResourceNum raw{};
    const u32 bits = 3 | (2 << 4) | (1 << 8) | (1 << 12) | (1 << 13) |
                     (2 << 15) | (4 << 18) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 27);
    word(&raw.bits, bits);
    const auto counts = Native::readLayoutMaterialCounts(raw);
    assert(counts.GetTexMapNum() == 3 && counts.GetTexSRTNum() == 2 && counts.GetTexCoordGenNum() == 1);
    assert(counts.GetIndTexSRTNum() == 1 && counts.GetIndTexStageNum() == 2 && counts.GetTevStageNum() == 4);
    assert(counts.HasTevSwapTable() && counts.HasAlphaCompare() && counts.HasBlendMode());
    assert(counts.GetChanCtrlNum() == 1 && counts.GetMatColNum() == 1);
    const auto* original = reinterpret_cast<const u8*>(&raw.bits);
    assert(original[0] == u8(bits >> 24)); // Decoding must not mutate archive bytes.

    // A real material's channel control follows the texture map/SRT/generator
    // arrays. Wrong resource counts place the read at unrelated bytes.
    alignas(res::Material) std::array<u8, 96> material{};
    static_assert(sizeof(res::Material) == 64 && sizeof(TexSRT) == 20);
    auto* resource = reinterpret_cast<res::Material*>(material.data());
    word(&resource->resNum.bits, 1 | (1 << 4) | (1 << 8) | (1 << 25));
    material[92] = GX_SRC_VTX; material[93] = GX_SRC_REG;
    const auto one = Native::readLayoutMaterialCounts(resource->resNum);
    const auto channelOffset = sizeof(res::Material) + one.GetTexMapNum() * sizeof(res::TexMap) +
        one.GetTexSRTNum() * sizeof(TexSRT) + one.GetTexCoordGenNum() * sizeof(TexCoordGen);
    assert(channelOffset == 92 && material[channelOffset] == GX_SRC_VTX && material[channelOffset + 1] == GX_SRC_REG);

    GXColorS10 color{};
    const std::array<s16, 4> expected{-128, 255, 1023, -1024};
    auto* colorBytes = reinterpret_cast<u8*>(&color);
    for (unsigned i = 0; i < 4; ++i) { colorBytes[i * 2] = u8(u16(expected[i]) >> 8); colorBytes[i * 2 + 1] = u8(expected[i]); }
    const auto decodedColor = Native::readLayoutTevColor(color);
    assert(decodedColor.r == -128 && decodedColor.g == 255 && decodedColor.b == 1023 && decodedColor.a == -1024);

    TexSRT transform{};
    const std::array<float, 5> values{2.5f, -0.125f, 90.f, 1.f, -1.f};
    for (unsigned i = 0; i < values.size(); ++i) { u32 encoded; std::memcpy(&encoded, &values[i], 4); word(reinterpret_cast<u8*>(&transform) + i * 4, encoded); }
    const auto decoded = Native::readLayoutTextureSrt(transform);
    assert(decoded.translate.x == values[0] && decoded.translate.y == values[1] && decoded.rotate == values[2]);
    assert(decoded.scale.x == values[3] && decoded.scale.y == values[4]);
    std::puts("PASS Wii layout material counts, channel offset, signed TEV colors, texture SRT, RGBA vertex colors, UV floats, and immutable source bytes");
}

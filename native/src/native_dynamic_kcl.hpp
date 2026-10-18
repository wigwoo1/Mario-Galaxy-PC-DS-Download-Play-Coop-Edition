#pragma once
#include <Game/Map/KCollision.hpp>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stdexcept>

// Serialize the original DynamicCollisionObj single-leaf geometry into the
// bounded byte format consumed by NativeKclStorage. Counts describe live arrays;
// no Wii pointer overlay or packed native header is copied.
inline std::vector<std::uint8_t> encodeDynamicKcl(const KCLFile& file,
    std::size_t positionCount, std::size_t normalCount, std::size_t triangleCount) {
    if (!triangleCount || triangleCount > 16384 || positionCount != triangleCount ||
        normalCount != triangleCount * 4 || !file.mPos || !file.mNorms || !file.mPrisms)
        throw std::invalid_argument("Dynamic KCL array counts");
    if (file.mBlockXShift != -1 || file.mBlockXYShift != -1 ||
        file.mBlockWidthShift < 0 || file.mBlockWidthShift > 31 ||
        !std::isfinite(file.mThickness) || file.mThickness < 0)
        throw std::invalid_argument("Dynamic KCL header");
    const std::size_t pos = 56, norm = pos + positionCount * 12;
    const std::size_t prism = norm + normalCount * 12 - 16;
    const std::size_t tree = prism + (triangleCount + 1) * 16;
    std::vector<std::uint8_t> bytes(tree + 4 + (triangleCount + 2) * 2, 0);
    auto word = [&](std::size_t at, std::uint32_t value) {
        for (int i=0; i<4; ++i) bytes[at+i]=std::uint8_t(value >> (24-i*8));
    };
    auto half = [&](std::size_t at, std::uint16_t value) {
        bytes[at]=std::uint8_t(value>>8); bytes[at+1]=std::uint8_t(value);
    };
    auto number = [&](std::size_t at, float value) {
        if (!std::isfinite(value)) throw std::invalid_argument("Non-finite dynamic KCL geometry");
        std::uint32_t bits; std::memcpy(&bits,&value,4); word(at,bits);
    };
    word(0,std::uint32_t(pos)); word(4,std::uint32_t(norm));
    word(8,std::uint32_t(prism)); word(12,std::uint32_t(tree));
    number(16,file.mThickness); number(20,file.mMin.x);
    number(24,file.mMin.y); number(28,file.mMin.z);
    word(32,std::uint32_t(file.mXMask)); word(36,std::uint32_t(file.mYMask));
    word(40,std::uint32_t(file.mZMask)); word(44,std::uint32_t(file.mBlockWidthShift));
    word(48,0xffffffffu); word(52,0xffffffffu);
    auto vectors = [&](std::size_t at, const TVec3f* values, std::size_t count) {
        for (std::size_t i=0;i<count;++i) {
            number(at+i*12,values[i].x); number(at+i*12+4,values[i].y);
            number(at+i*12+8,values[i].z);
        }
    };
    vectors(pos,file.mPos,positionCount); vectors(norm,file.mNorms,normalCount);
    // The reserved prism overlaps the last normal in the original file layout.
    for (std::size_t i=1;i<=triangleCount;++i) {
        const auto& value=file.mPrisms[i]; const auto at=prism+i*16;
        if (value.mPositionIndex>=positionCount || value.mNormalIndex>=normalCount)
            throw std::out_of_range("Dynamic KCL prism index");
        for (auto edge:value.mEdgeIndices)
            if(edge>=normalCount) throw std::out_of_range("Dynamic KCL edge index");
        number(at,value.mHeight); half(at+4,value.mPositionIndex);
        half(at+6,value.mNormalIndex);
        for(int j=0;j<3;++j) half(at+8+j*2,value.mEdgeIndices[j]);
        half(at+14,value.mAttribute);
    }
    word(tree,0x80000002u); // Original reserved halfword lies inside root word.
    for(std::size_t i=0;i<triangleCount;++i)
        half(tree+4+i*2,std::uint16_t(triangleCount-i));
    return bytes;
}

#include "Game/System/BinaryDataContentAccessor.hpp"
#include <stdexcept>
namespace {
u16 nativeContentReadU16(const u8* data) { return u16((u16(data[0]) << 8) | data[1]); }
void nativeContentWriteU16(JSUMemoryOutputStream& stream, u16 value) {
    const u8 bytes[2] = {u8(value >> 8), u8(value)};
    stream.write(bytes, 2);
}
}
#include "Game/Util/HashUtil.hpp"
#include <JSystem/JSupport/JSUInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

BinaryDataContentHeaderSerializer::BinaryDataContentHeaderSerializer(u8* pData, u32 dataSize)
    : mStream(pData, dataSize), mAttributeNum(0), mDataSize(0) {
    if (!pData || dataSize < 4) throw std::invalid_argument("Save content header capacity");
    u16 numAttributes = 0;
    nativeContentWriteU16(mStream, numAttributes);
    u16 streamDataSize = 0;
    nativeContentWriteU16(mStream, streamDataSize);
}

void BinaryDataContentHeaderSerializer::addAttribute(const char* pName, u32 attributeSize) {
    if (!pName || mStream.getAvailable() < 4 || attributeSize > 65535 - mDataSize || mAttributeNum >= 65535) throw std::length_error("Save content attribute capacity");
    u16 hash = MR::getHashCode(pName);
    nativeContentWriteU16(mStream, hash);
    u16 offset = mDataSize;
    nativeContentWriteU16(mStream, offset);
    mDataSize += attributeSize;
    mAttributeNum++;
}

void BinaryDataContentHeaderSerializer::flush() {
    s32 restorePos = mStream.getPosition();
    mStream.seek(0, SEEK_FROM_START);
    u16 attributeNum = mAttributeNum;
    nativeContentWriteU16(mStream, attributeNum);
    u16 dataSize = mDataSize;
    nativeContentWriteU16(mStream, dataSize);
    mStream.seek(restorePos, SEEK_FROM_START);
}

u32 BinaryDataContentHeaderSerializer::getHeaderSize() const {
    return mStream.getPosition();
}

u32 BinaryDataContentHeaderSerializer::getDataSize() const {
    return mDataSize;
}

BinaryDataContentAccessor::BinaryDataContentAccessor(u8* pData) : mData(pData) {
}

s32 BinaryDataContentAccessor::getHeaderSize() const {
    return nativeContentReadU16(mData) * 4 + 4;
}

s32 BinaryDataContentAccessor::getDataSize() const {
    return nativeContentReadU16(mData + 2);
}

// stripped
s32 BinaryDataContentAccessor::getAttributeNum() const {
    return nativeContentReadU16(mData);
}

void* BinaryDataContentAccessor::getPointer(const char* name, u8* data) const {
    if (!name || !data) return nullptr;
    const u16 hash = u16(MR::getHashCode(name));
    for (s32 i = 0; i < getAttributeNum(); ++i) {
        const u8* entry = mData + 4 + i * 4;
        if (nativeContentReadU16(entry) == hash) {
            const u16 offset = nativeContentReadU16(entry + 2);
            return offset < getDataSize() ? data + offset : nullptr;
        }
    }
    return nullptr;
}

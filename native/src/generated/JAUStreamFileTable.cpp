#include "JSystem/JAudio2/JAUStreamFileTable.hpp"
#include <revolution/dvd.h>
namespace { u32 streamTableWord(const void* value){auto p=static_cast<const u8*>(value);return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];} }

JAUStreamFileTable::JAUStreamFileTable() {
    mData = nullptr;
}

void JAUStreamFileTable::init(void const* data) {
    const BinaryStreamFileTable* binaryTable = (const BinaryStreamFileTable*)data;
    if (binaryTable == nullptr) {
        mData = nullptr;
        return;
    }
    if (binaryTable->mIdentifier[0] == 'b' && binaryTable->mIdentifier[1] == 's' && binaryTable->mIdentifier[2] == 'f' &&
        binaryTable->mIdentifier[3] == 't') {
        mData = binaryTable;
    }
}

u32 JAUStreamFileTable::getNumFiles() const {
    return mData ? streamTableWord(reinterpret_cast<const u8*>(mData)+4) : 0;
}

const char* JAUStreamFileTable::getFilePath(int index) const {
    if (mData == nullptr || index < 0 || u32(index) >= getNumFiles()) {
        return 0;
    }

    char* data = (char*)mData;
    return (char*)(data + streamTableWord(data + 8 + (index * sizeof(s32))));
}

s32 JAUStreamDataMgr_StreamFileTable::getStreamFileEntry(JAISoundID soundID) {
    const char* filePath = getFilePath(soundID.getWaveID());
    if (filePath == nullptr) {
        return -1;
    }
    return DVDConvertPathToEntrynum(filePath);
}


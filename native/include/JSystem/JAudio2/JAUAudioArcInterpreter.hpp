#include <cstddef>
#include <stdexcept>
#pragma once

#include <revolution/types.h>

class JAUAudioArcInterpreter {
public:
    JAUAudioArcInterpreter();
    virtual ~JAUAudioArcInterpreter();
    virtual void readWS(u32, void const*, u32) = 0;
    virtual void readBNK(u32, void const*) = 0;
    virtual void readBSC(void const*, u32) = 0;
    virtual void readBST(void const*, u32) = 0;
    virtual void readBSTN(void const*, u32) = 0;
    virtual void readBMS(u32, void const*, u32) = 0;
    virtual void readBMS_fromArchive(u32) = 0;
    virtual void newVoiceBank(u32, u32) = 0;
    virtual void newDynamicSeqBlock(u32) = 0;
    virtual void readBSFT(void const*) = 0;
    virtual void readMaxSeCategory(int, int, int) = 0;
    virtual void beginBNKList(u32, u32) = 0;
    virtual void endBNKList() = 0;
    bool parse(void const*);
    std::size_t nativeArchiveSize=0; bool nativeBounded=false;
    bool parse(const void* data,std::size_t size){if(!data || size<8)return false;nativeArchiveSize=size;nativeBounded=true;try{bool result=parse(data);nativeBounded=false;return result;}catch(const std::out_of_range&){nativeBounded=false;return false;}}
    void nativeCheckRead(std::size_t count) const {if(nativeBounded){const auto offset=std::size_t(mReadPtr-mBase);if(offset>nativeArchiveSize || count>nativeArchiveSize-offset)throw std::out_of_range("Audio archive command extent");}}
    virtual bool readCommandMore(u32);
    bool readCommand_();

    u8 readU8_() {
        nativeCheckRead(1); return *mReadPtr++;
    }

    u32 readU32_() {
        nativeCheckRead(4); u32 temp = (u32(mReadPtr[0])<<24)|(u32(mReadPtr[1])<<16)|(u32(mReadPtr[2])<<8)|mReadPtr[3];
        mReadPtr += 4;
        return temp;
    }
    const void* getContent_(u32 offset) const {
        if(nativeBounded && offset>nativeArchiveSize)throw std::out_of_range("Audio archive resource offset");return mBase + offset;
    }

    /* 0x04 */ const u8* mReadPtr;
    /* 0x08 */ const u8* mBase;
};


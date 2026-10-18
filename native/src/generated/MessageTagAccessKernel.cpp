#include "Game/Screen/MessageTagSkipTagProcessor.hpp"

MessageEditorMessageTag::MessageEditorMessageTag(const nw4r::ut::PrintContext< wchar_t >* pContext) : mMessage(pContext->str) {
}

MessageEditorMessageTag::MessageEditorMessageTag(const wchar_t* pMessage) : mMessage(pMessage) {
}

u32 MessageEditorMessageTag::getTagLength() const {
    return nativeMessageTag(mMessage).tag.size();
}

u32 MessageEditorMessageTag::getSkipLength() const {
    return getTagLength() / 2;
}

s32 MessageEditorMessageTag::getParamLength() const {
    return nativeMessageTag(mMessage).parameterBytes();
}

u8 MessageEditorMessageTag::getParam8(int index) const {
    const u8* pParam = reinterpret_cast< const u8* >(mMessage) + index;
    return nativeMessageTag(mMessage).param8(static_cast<size_t>(index));
}

u16 MessageEditorMessageTag::getParam16(int index) const {
    const u16* pParam = reinterpret_cast< const u16* >(mMessage) + index;
    return nativeMessageTag(mMessage).param16(static_cast<size_t>(index));
}

u32 MessageEditorMessageTag::getParam32(int index) const {
    const u32* pParam = reinterpret_cast< const u32* >(mMessage) + index;
    return nativeMessageTag(mMessage).param32(static_cast<size_t>(index));
}

wchar_t* MessageEditorMessageTag::getParamPtr(int index) const {
    const u8* pParam = reinterpret_cast< const u8* >(mMessage) + index + 4;
    nativeMessageTag(mMessage); throw std::runtime_error("Typed tag string/pointer conversion is not implemented");
}


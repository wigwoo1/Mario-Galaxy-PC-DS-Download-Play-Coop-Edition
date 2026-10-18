#pragma once
#include "native_tagged_message.hpp"
#include "native_message_text_binding.hpp"
#include <tuple>
// Windows compatibility macros collide with original actor parameter names.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
inline thread_local const NativeTaggedMessage* nativeTagOwner=nullptr;
inline thread_local NativeBmgText::Token nativeRawMessageTag{};
inline thread_local std::map<std::tuple<std::uintptr_t,size_t,size_t>,std::wstring> nativeRawTagText{};
class NativeMessageTagScope {
    const NativeTaggedMessage* previous;
public:
    explicit NativeMessageTagScope(const NativeTaggedMessage& owner):previous(nativeTagOwner){nativeTagOwner=&owner;}
    ~NativeMessageTagScope(){nativeTagOwner=previous;}
    NativeMessageTagScope(const NativeMessageTagScope&)=delete;
    NativeMessageTagScope& operator=(const NativeMessageTagScope&)=delete;
};
inline const NativeBmgText::Token& nativeMessageTag(const wchar_t* payload){
    if(nativeTagOwner)return nativeTagOwner->tagAt(payload);
    NativeMessageTextAddress address;if(nativeMessageTextAddress(payload,&address))return address.owner->tagAt(address.offset);
    assetCheck(payload!=nullptr,"Message tag payload is null");
    const auto* bytes=reinterpret_cast<const uint8_t*>(payload);const size_t length=bytes[0];
    assetCheck(length>=6&&length<=254&&!(length&1),"Unowned inline message tag has an invalid length");
    nativeRawMessageTag.code=0x1a;nativeRawMessageTag.tag.assign(bytes,bytes+length-2);nativeRawMessageTag.validateTag();
    return nativeRawMessageTag;
}
inline const wchar_t* nativeMessageTagText(const wchar_t* payload,size_t byteOffset,size_t count){
    if(nativeTagOwner)return nativeTagOwner->tagText(payload,byteOffset,count);
    NativeMessageTextAddress address;if(nativeMessageTextAddress(payload,&address))return address.owner->tagText(address.owner->data()+address.offset,byteOffset,count);
    const auto& tag=nativeMessageTag(payload);const size_t size=tag.parameterBytes();
    assetCheck(byteOffset<=size&&count<=(size-byteOffset)/2,"Tag text exceeds inline payload extent");
    const auto key=std::make_tuple(reinterpret_cast<std::uintptr_t>(payload),byteOffset,count);auto found=nativeRawTagText.find(key);if(found!=nativeRawTagText.end())return found->second.c_str();
    std::wstring text;text.reserve(count);for(size_t i=0;i<count;++i)text.push_back(wchar_t((uint16_t(tag.param8(byteOffset+i*2))<<8)|tag.param8(byteOffset+i*2+1)));
    return nativeRawTagText.emplace(key,std::move(text)).first->second.c_str();
}
inline const wchar_t* nativeMessageTagBase(const wchar_t* payload,size_t count){
    if(nativeTagOwner)return nativeTagOwner->baseTextAfterTag(payload,count);
    NativeMessageTextAddress address;if(nativeMessageTextAddress(payload,&address))return address.owner->baseTextAfterTag(address.owner->data()+address.offset,count);
    const auto& tag=nativeMessageTag(payload);const size_t start=tag.tag.size()/2;
    for(size_t i=0;i<count;++i)assetCheck(payload[start+i]!=0x1a&&payload[start+i]!=0,"Ruby base contains a tag or terminator");
    return payload+start;
}


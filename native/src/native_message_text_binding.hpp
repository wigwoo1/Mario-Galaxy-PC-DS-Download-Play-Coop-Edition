#pragma once
#include "native_tagged_message.hpp"
#include <Game/System/MessageHolder.hpp>
#include <memory>
#include <cstdint>
#include <vector>
#include <algorithm>
using NativeMessageTextOwners=std::vector<std::unique_ptr<NativeTaggedMessage>>;
inline auto& nativeMessageTextBindings(){static std::map<const MessageData*,const NativeMessageTextOwners*> bindings;return bindings;}
struct NativeMessageTextAddress{const NativeTaggedMessage* owner=nullptr;size_t offset=0;};
struct NativeMessageTextCopy{const wchar_t* begin=nullptr;size_t length=0;const NativeTaggedMessage* owner=nullptr;size_t sourceOffset=0;};
inline auto& nativeMessageTextCopies(){static std::vector<NativeMessageTextCopy> copies;return copies;}
inline bool nativeMessageTextAddress(const wchar_t* text,NativeMessageTextAddress* result){
    if(!text||!result)return false;
    for(const auto& binding:nativeMessageTextBindings())for(const auto& owner:*binding.second){size_t offset=0;if(owner&&owner->offsetOf(text,&offset)){*result={owner.get(),offset};return true;}}
    const auto address=reinterpret_cast<std::uintptr_t>(text);
    for(const auto& copy:nativeMessageTextCopies()){
        const auto begin=reinterpret_cast<std::uintptr_t>(copy.begin),bytes=copy.length*sizeof(wchar_t);
        if(address>=begin&&address-begin<bytes&&(address-begin)%sizeof(wchar_t)==0){*result={copy.owner,copy.sourceOffset+(address-begin)/sizeof(wchar_t)};return true;}
    }
    return false;
}
inline void bindNativeMessageTextCopy(const wchar_t* destination,size_t length,const wchar_t* source){
    if(!destination||!length)return;
    NativeMessageTextAddress sourceAddress;const bool hasOwner=nativeMessageTextAddress(source,&sourceAddress);
    const auto begin=reinterpret_cast<std::uintptr_t>(destination),end=begin+length*sizeof(wchar_t);
    auto& copies=nativeMessageTextCopies();std::vector<NativeMessageTextCopy> retained;retained.reserve(copies.size()+1);
    for(const auto& copy:copies){
        const auto oldBegin=reinterpret_cast<std::uintptr_t>(copy.begin),oldEnd=oldBegin+copy.length*sizeof(wchar_t);
        if(end<=oldBegin||begin>=oldEnd){retained.push_back(copy);continue;}
        if(oldBegin<begin)retained.push_back({copy.begin,(begin-oldBegin)/sizeof(wchar_t),copy.owner,copy.sourceOffset});
        if(oldEnd>end)retained.push_back({reinterpret_cast<const wchar_t*>(end),(oldEnd-end)/sizeof(wchar_t),copy.owner,copy.sourceOffset+(end-oldBegin)/sizeof(wchar_t)});
    }
    if(hasOwner)retained.push_back({destination,length,sourceAddress.owner,sourceAddress.offset});
    copies=std::move(retained);
}
inline void unbindNativeMessageTextCopies(const wchar_t* buffer,size_t length){
    if(!buffer||!length)return;const auto begin=reinterpret_cast<std::uintptr_t>(buffer),end=begin+length*sizeof(wchar_t);auto& copies=nativeMessageTextCopies();
    copies.erase(std::remove_if(copies.begin(),copies.end(),[&](const auto& copy){const auto a=reinterpret_cast<std::uintptr_t>(copy.begin),b=a+copy.length*sizeof(wchar_t);return begin<b&&a<end;}),copies.end());
}
inline void unbindNativeMessageTextCopies(const NativeMessageTextOwners* owners){
    if(!owners)return;auto& copies=nativeMessageTextCopies();
    copies.erase(std::remove_if(copies.begin(),copies.end(),[&](const auto& copy){return std::find_if(owners->begin(),owners->end(),[&](const auto& owner){return owner.get()==copy.owner;})!=owners->end();}),copies.end());
}
inline const NativeTaggedMessage& nativeMessageTextOwner(const MessageData& message,size_t index){
    auto found=nativeMessageTextBindings().find(&message);
    assetCheck(found!=nativeMessageTextBindings().end(),"Message getter requires owned host text binding");
    assetCheck(index<found->second->size(),"Host message index outside text binding");
    return *found->second->at(index);
}
inline u8* nativeMessageText(const MessageData& message,size_t index){
    return reinterpret_cast<u8*>(const_cast<wchar_t*>(nativeMessageTextOwner(message,index).data()));
}

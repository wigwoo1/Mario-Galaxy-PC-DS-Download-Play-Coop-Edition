#pragma once
#include "message_text.hpp"
#include <map>
#include <cstring>
#include <tuple>
#include <cstdint>
// Host text units are UTF16. Tag payloads retain Wii bytes; access them through metadata.
class NativeTaggedMessage {
    std::vector<wchar_t> storage;
    std::map<size_t,NativeBmgText::Token> tags;
    mutable std::map<std::tuple<size_t,size_t,size_t>,std::wstring> parameterText;
public:
    explicit NativeTaggedMessage(std::vector<NativeBmgText::Token> tokens){
        static_assert(sizeof(wchar_t)==2,"Native messages require UTF16 wchar_t");
        size_t units=1;for(auto& token:tokens){if(token.code==0x1a){token.validateTag();units+=1+token.tag.size()/2;}else{assetCheck(token.code!=0,"Unexpected message terminator token");++units;}assetCheck(units<=1024*1024,"Host message exceeds budget");}
        storage.reserve(units);
        for(auto& token:tokens){storage.push_back(wchar_t(token.code));if(token.code==0x1a){auto offset=storage.size();storage.resize(offset+token.tag.size()/2);std::memcpy(storage.data()+offset,token.tag.data(),token.tag.size());tags.emplace(offset,std::move(token));}}
        storage.push_back(0);
    }
    NativeTaggedMessage(NativeTaggedMessage&&)=delete;
    NativeTaggedMessage& operator=(NativeTaggedMessage&&)=delete;
    NativeTaggedMessage(const NativeTaggedMessage&)=delete;
    NativeTaggedMessage& operator=(const NativeTaggedMessage&)=delete;
    const wchar_t* data()const{return storage.data();}
    size_t size()const{return storage.size()-1;}
    size_t tagCount()const{return tags.size();}
    bool offsetOf(const wchar_t* text,size_t* offset)const{
        if(!text||!offset)return false;
        const auto begin=reinterpret_cast<std::uintptr_t>(storage.data());
        const auto address=reinterpret_cast<std::uintptr_t>(text);
        const auto bytes=storage.size()*sizeof(wchar_t);
        if(address<begin||address-begin>=bytes||(address-begin)%sizeof(wchar_t))return false;
        *offset=(address-begin)/sizeof(wchar_t);return true;
    }
    // Original PrintContext points immediately after the 0x1A marker.
    // Resolve only exact owned payload starts, without comparing unrelated pointers.
    const NativeBmgText::Token& tagAt(const wchar_t* payload)const{
        for(const auto& entry:tags)if(storage.data()+entry.first==payload)return entry.second;
        assetCheck(false,"Tag pointer is not an owned payload start");
        throw std::runtime_error("Unreachable tag lookup");
    }
    const wchar_t* tagText(const wchar_t* payload,size_t byteOffset,size_t count)const{
        const auto& token=tagAt(payload);
        assetCheck(byteOffset<=token.parameterBytes() && count<=(token.parameterBytes()-byteOffset)/2,"Tag text exceeds owned parameter extent");
        auto key=std::make_tuple(size_t(payload-storage.data()),byteOffset,count);
        auto found=parameterText.find(key);
        if(found!=parameterText.end())return found->second.c_str();
        std::wstring text; text.reserve(count);
        for(size_t i=0;i<count;++i)text.push_back(wchar_t((uint16_t(token.param8(byteOffset+i*2))<<8)|token.param8(byteOffset+i*2+1)));
        return parameterText.emplace(key,std::move(text)).first->second.c_str();
    }
    const wchar_t* baseTextAfterTag(const wchar_t* payload,size_t count)const{
        const auto& token=tagAt(payload);
        size_t start=size_t(payload-storage.data())+token.tag.size()/2;
        assetCheck(start<=size() && count<=size()-start,"Ruby base exceeds owned message extent");
        for(size_t i=0;i<count;++i)assetCheck(storage[start+i]!=0x1a && storage[start+i]!=0,"Ruby base contains a tag or terminator");
        return storage.data()+start;
    }
    const wchar_t* afterTag(const wchar_t* payload)const{
        const auto& token=tagAt(payload);
        return payload+token.tag.size()/2;
    }
    const NativeBmgText::Token& tagAt(size_t offset)const{auto found=tags.find(offset);assetCheck(found!=tags.end(),"Host message position is not a tag payload");return found->second;}
};





#pragma once
#include "message_text.hpp"
#include <map>

// Owned per-message arguments. No native pointer is stored in encoded tag bytes.
class NativeMessageArguments {

    std::map<uint32_t,int32_t> numbers;
    std::map<uint32_t,std::wstring> strings;
public:
    void setNumber(uint32_t index,int32_t value){numbers.insert_or_assign(index,value);}
    void setString(uint32_t index,std::wstring value){strings.insert_or_assign(index,std::move(value));}
    void clear(){numbers.clear();strings.clear();}
    std::optional<int32_t> number(const NativeBmgText::Token& token)const{
        assetCheck(token.group()==6,"Number argument requires group 6");auto found=numbers.find(token.param32(1));
        if(found==numbers.end())return std::nullopt;return found->second;
    }
    const std::wstring* string(const NativeBmgText::Token& token)const{
        assetCheck(token.group()==7,"String argument requires group 7");auto found=strings.find(token.param32(1));
        return found==strings.end()?nullptr:&found->second;
    }
};


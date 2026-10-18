#pragma once
#include "graphics.hpp"
#include <JSystem/JUtility/JUTNameTab.hpp>
// Original JUTNameTab retains this resource: owner must outlive the name table.
struct NativeNameTable {
    std::vector<u16> storage;
    explicit NativeNameTable(BinaryView table){
        extent(0,4,table.size());size_t count=table.u16(0);extent(4,count*4,table.size());
        storage.resize((table.size()+1)/2);auto* bytes=reinterpret_cast<uint8_t*>(storage.data());
        for(size_t i=0;i<table.size();++i)bytes[i]=table.u8(i);
        storage[0]=u16(count);storage[1]=table.u16(2);
        for(size_t i=0;i<count;++i){size_t offset=table.u16(6+i*4);assetCheck(offset>=4+count*4 && offset<table.size(),"Invalid name offset");
            size_t end=offset;while(end<table.size() && table.u8(end))++end;assetCheck(end<table.size(),"Unterminated model name");
            storage[2+i*2]=table.u16(4+i*4);storage[3+i*2]=u16(offset);
        }
    }
    const ResNTAB* resource() const{return reinterpret_cast<const ResNTAB*>(storage.data());}
};

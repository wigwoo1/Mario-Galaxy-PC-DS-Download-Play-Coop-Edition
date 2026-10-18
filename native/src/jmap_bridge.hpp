#pragma once
#include "stage_data.hpp"
#include "Game/Util/JMapInfo.hpp"
class NativeJmapTable {
    std::vector<uint32_t> storage;
public:
    JMapInfo info;
    NativeJmapTable(const NativeJmapTable&)=delete;NativeJmapTable& operator=(const NativeJmapTable&)=delete;
    explicit NativeJmapTable(BinaryView source){
        StageTable table(source);table.validate();size_t originalData=source.u32(8),originalStride=source.u32(12),originalStrings=originalData+table.rows*originalStride;
        std::vector<size_t> offsets;size_t stride=0;
        for(auto f:table.fields){size_t alignment=f.type==1||f.type==5?1:f.type==4?2:4;stride=(stride+alignment-1)/alignment*alignment;assetCheck(stride<=65535,"Native JMap row offset overflow");offsets.push_back(stride);stride+=f.type==1?32:alignment;}
        stride=(stride+3)/4*4;size_t data=16+table.fields.size()*12,strings=data+table.rows*stride,total=strings+source.size()-originalStrings;
        assetCheck(total<=256*1024*1024,"Repacked native JMap exceeds budget");storage.resize((total+3)/4);auto bytes=reinterpret_cast<uint8_t*>(storage.data());
        for(size_t i=originalStrings;i<source.size();++i)bytes[strings+i-originalStrings]=source.u8(i);
        auto put32=[&](size_t p,uint32_t value){std::memcpy(bytes+p,&value,4);};auto put16=[&](size_t p,uint16_t value){std::memcpy(bytes+p,&value,2);};
        put32(0,uint32_t(table.rows));put32(4,uint32_t(table.fields.size()));put32(8,uint32_t(data));put32(12,uint32_t(stride));
        for(size_t i=0;i<table.fields.size();++i){auto f=table.fields[i];size_t p=16+i*12;put32(p,f.hash);put32(p+4,f.mask);put16(p+8,uint16_t(offsets[i]));bytes[p+10]=f.shift;bytes[p+11]=f.type;
            size_t width=f.type==1||f.type==5?1:f.type==4?2:4;
            for(size_t row=0;row<table.rows;++row){size_t value=data+row*stride+offsets[i],original=originalData+row*originalStride+f.offset;if(width==4)put32(value,source.u32(original));else if(width==2)put16(value,source.u16(original));else for(size_t k=0;k<(f.type==1?32u:1u);++k)bytes[value+k]=source.u8(original+k);}}
        info.attach(storage.data());
    }
};
inline size_t verifyNativeJmap(BinaryView source){
    StageTable reference(source);NativeJmapTable native(source);size_t cells=0;
    for(size_t row=0;row<reference.rows;++row)for(size_t field=0;field<reference.fields.size();++field){auto value=reference.read(row,field);auto type=reference.fields[field].type;
        if(type==1||type==6){const char* text=nullptr;assetCheck(native.info.getValueFast(int(row),int(field),&text)&&std::get<std::string>(value)==text,"Native JMap string bridge differs");}
        else if(type==2){float number=0;assetCheck(native.info.getValueFast(int(row),int(field),&number)&&number==std::get<float>(value),"Native JMap float bridge differs");}
        else{u32 number=0;assetCheck(native.info.getValueFast(int(row),int(field),&number)&&number==std::get<uint32_t>(value),"Native JMap integer bridge differs");}++cells;
    }return cells;
}

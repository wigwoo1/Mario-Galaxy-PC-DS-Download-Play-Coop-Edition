#pragma once
#include "graphics.hpp"
#include <variant>
inline uint32_t jmapHash(const std::string& name){uint32_t hash=0;for(unsigned char c:name)hash=hash*31+c;return hash;}
struct JmapField {uint32_t hash,mask;uint16_t offset;uint8_t shift,type;};
using JmapValue=std::variant<uint32_t,float,std::string>;
class StageTable {
    BinaryView view;size_t data,stride,stringBase;
public:
    size_t rows;std::vector<JmapField> fields;
    explicit StageTable(BinaryView input):view(input){
        rows=view.u32(0);size_t count=view.u32(4);data=view.u32(8);stride=view.u32(12);
        assetCheck(rows<=1000000&&count<=1024,"Oversized stage table");extent(16,count*12,view.size());
        assetCheck(data>=16+count*12,"Stage data overlaps field descriptors");extent(data,rows*stride,view.size());stringBase=data+rows*stride;
        for(size_t i=0;i<count;++i){size_t p=16+i*12;JmapField f{view.u32(p),view.u32(p+4),view.u16(p+8),view.u8(p+10),view.u8(p+11)};
            assetCheck(f.type<=6&&f.shift<32,"Unsupported stage field");size_t width=f.type==1?32:f.type==4?2:f.type==5?1:4;extent(f.offset,width,stride);
            fields.push_back(f);}
    }
    int find(const std::string& name)const{uint32_t hash=jmapHash(name);for(size_t i=0;i<fields.size();++i)if(fields[i].hash==hash)return int(i);return -1;}
    JmapValue read(size_t row,size_t field)const{
        assetCheck(row<rows&&field<fields.size(),"Stage cell outside table");auto f=fields[field];size_t p=data+row*stride+f.offset;
        if(f.type==2)return view.f32(p);
        if(f.type==1||f.type==6){size_t end;
            if(f.type==6){p=stringBase+view.u32(p);assetCheck(p<view.size(),"Stage string pointer outside table");end=view.size();}else end=p+32;
            std::string text;for(;p<end;++p){uint8_t c=view.u8(p);if(!c)return text;text+=char(c);}throw std::runtime_error("Unterminated stage string");}
        uint32_t raw=f.type==4?view.u16(p):f.type==5?view.u8(p):view.u32(p);return (raw&f.mask)>>f.shift;
    }
    void validate()const{for(size_t r=0;r<rows;++r)for(size_t f=0;f<fields.size();++f)read(r,f);}
};
inline void stageDataTests(){
    std::vector<uint8_t> b(40,0);auto put=[&](size_t p,uint32_t value){for(int k=0;k<4;++k)b.at(p+k)=uint8_t(value>>(24-8*k));};
    put(0,2);put(4,1);put(8,28);put(12,4);put(16,jmapHash("Obj_arg0"));put(20,0x00ff0000);b[26]=16;b[27]=0;put(28,0x12560000);put(32,0x00ff0000);
    StageTable table{BinaryView(b)};assetCheck(table.find("Obj_arg0")==0&&table.find("missing")==-1&&std::get<uint32_t>(table.read(0,0))==0x56,"Stage hash/masked integer test");
    b[27]=6;put(28,0);b[36]='A';b[37]=0;StageTable strings{BinaryView(b)};assetCheck(std::get<std::string>(strings.read(0,0))=="A","Stage string offset test");
    bool rejected=false;put(28,100);try{StageTable invalid{BinaryView(b)};invalid.read(0,0);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid stage string pointer accepted");
}

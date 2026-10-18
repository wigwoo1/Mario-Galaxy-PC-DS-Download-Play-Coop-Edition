#pragma once
#include "rig.hpp"
struct MeshVertex {
    uint32_t position=UINT32_MAX;uint16_t drawMatrix=0;
    std::array<uint32_t,21> attributeIndices;
    std::array<uint32_t,3> normalIndices{UINT32_MAX,UINT32_MAX,UINT32_MAX};
    std::array<uint8_t,9> matrixSelectors{};
    MeshVertex(){attributeIndices.fill(UINT32_MAX);}
};
struct MeshPacket { uint16_t shape;std::vector<MeshVertex> vertices;std::vector<std::array<uint32_t,3>> triangles; };
struct ModelMesh {std::vector<MeshPacket> packets;size_t triangles=0,vertices=0;};
inline void triangulate(uint8_t primitive,uint32_t first,uint32_t count,std::vector<std::array<uint32_t,3>>& out){
    auto add=[&](uint32_t a,uint32_t b,uint32_t c){out.push_back({first+a,first+b,first+c});};
    switch(primitive){
    case 0x90:assetCheck(count%3==0,"Incomplete triangle primitive");for(uint32_t i=0;i<count;i+=3)add(i,i+1,i+2);break;
    case 0x80:assetCheck(count%4==0,"Incomplete quad primitive");for(uint32_t i=0;i<count;i+=4){add(i,i+1,i+2);add(i,i+2,i+3);}break;
    case 0x98:assetCheck(count>=3,"Incomplete triangle strip");for(uint32_t i=2;i<count;++i){if(i%2)add(i-1,i-2,i);else add(i-2,i-1,i);}break;
    case 0xa0:assetCheck(count>=3,"Incomplete triangle fan");for(uint32_t i=2;i<count;++i)add(0,i-1,i);break;
    case 0xa8:assetCheck(count%2==0,"Incomplete line primitive");break;
    case 0xb0:assetCheck(count>=2,"Incomplete line strip");break;
    case 0xb8:break;
    default:throw std::runtime_error("Unsupported shape primitive");
    }
}
inline ModelMesh decodeMesh(BinaryView file,const ModelInfo& model,const ModelRig& rig){
    std::map<std::string,BinaryView> sections;size_t pos=32;
    for(uint32_t i=0;i<file.u32(12);++i){size_t size=file.u32(pos+4);assetCheck(size>=8,"Invalid mesh section");assetCheck(sections.emplace(file.tag(pos),file.sub(pos,size)).second,"Duplicate mesh section");pos+=size;}
    assetCheck(sections.count("SHP1")&&sections.count("VTX1"),"Missing mesh sections");
    auto shape=sections.at("SHP1"),vtx=sections.at("VTX1");
    size_t init=shape.u32(12),remap=shape.u32(16),descriptors=shape.u32(24),matrices=shape.u32(28),lists=shape.u32(32),matrixInit=shape.u32(36),drawInit=shape.u32(40);
    uint16_t shapes=shape.u16(8);extent(remap,size_t(shapes)*2,shape.size());
    std::map<uint32_t,std::pair<uint32_t,uint32_t>> formats;
    bool formatEnd=false;for(size_t p=vtx.u32(8),i=0;i<32;++i,p+=16){uint32_t attr=vtx.u32(p);if(attr==255){formatEnd=true;break;}formats.emplace(attr,std::make_pair(vtx.u32(p+4),vtx.u32(p+8)));}
    assetCheck(formatEnd,"Unterminated vertex formats");
    auto directSize=[&](uint32_t attr)->size_t{
        if(attr<=8)return 1;
        assetCheck(formats.count(attr),"Missing direct attribute format");auto [components,type]=formats.at(attr);
        if(attr==11||attr==12){const size_t sizes[]={2,3,4,2,3,4};assetCheck(type<6,"Invalid direct color type");return sizes[type];}
        assetCheck(type<=4,"Invalid direct scalar type");size_t scalar=type<=1?1:(type<=3?2:4);
        size_t count=attr==9?(components?3:2):attr==10?(components?9:3):(components?2:1);return count*scalar;
    };
    ModelMesh mesh;
    for(uint16_t s=0;s<shapes;++s){
        uint16_t idx=shape.u16(remap+s*2);assetCheck(idx<shapes,"Invalid shape remap");size_t p=init+size_t(idx)*40;extent(p,40,shape.size());
        uint8_t matrixType=shape.u8(p);assetCheck(matrixType<=3,"Unsupported shape matrix type");
        uint16_t groups=shape.u16(p+2),dsc=shape.u16(p+4),midx=shape.u16(p+6),didx=shape.u16(p+8);
        std::vector<std::pair<uint32_t,uint32_t>> attrs;bool terminated=false;std::set<uint32_t> seen;
        for(size_t q=descriptors+dsc,i=0;i<32;++i,q+=8){uint32_t attr=shape.u32(q),kind=shape.u32(q+4);if(attr==255){terminated=true;break;}
            assetCheck(attr<=20&&kind<=3&&seen.insert(attr).second,"Invalid vertex descriptor");if(kind)attrs.push_back({attr,kind});}
        assetCheck(terminated&&seen.count(9),"Missing position descriptor");
        std::array<uint16_t,10> palette;palette.fill(0xffff);
        for(uint16_t g=0;g<groups;++g){
            size_t mi=matrixInit+(size_t(midx)+g)*8;uint16_t rigid=shape.u16(mi),used=shape.u16(mi+2);uint32_t firstMatrix=shape.u32(mi+4);
            if(matrixType==3){assetCheck(used<=10,"Excessive packet matrix palette");for(size_t k=0;k<used;++k){uint16_t m=shape.u16(matrices+(size_t(firstMatrix)+k)*2);if(m!=0xffff)palette[k]=m;}}
            else palette[0]=rigid;
            size_t di=drawInit+(size_t(didx)+g)*8;size_t size=shape.u32(di),offset=shape.u32(di+4);auto data=shape.sub(lists+offset,size);
            MeshPacket packet{s,{},{}};size_t q=0;
            while(q<data.size()){
                uint8_t opcode=data.u8(q++);if(opcode==0 || opcode==0x48)continue;
                assetCheck((opcode&7)==0,"Unsupported vertex attribute table selector");uint8_t primitive=opcode&0xf8;
                assetCheck(primitive==0x80||primitive==0x90||primitive==0x98||primitive==0xa0||primitive==0xa8||primitive==0xb0||primitive==0xb8,"Unknown display-list command");
                uint16_t count=data.u16(q);q+=2;assetCheck(packet.vertices.size()+count<=1000000,"Excessive packet vertices");uint32_t first=uint32_t(packet.vertices.size());
                for(uint16_t v=0;v<count;++v){uint32_t position=UINT32_MAX;uint16_t slot=0;MeshVertex output;
                    for(auto [attr,kind]:attrs){
                        if(kind==1){size_t n=directSize(attr);extent(q,n,data.size());if(attr<=8)output.matrixSelectors[attr]=data.u8(q);if(attr==0){uint8_t m=data.u8(q);assetCheck(m%3==0&&m/3<10,"Invalid position matrix selector");slot=m/3;}
                            assetCheck(attr!=9,"Direct position vertices are not implemented");q+=n;
                        }else{uint32_t index=kind==2?data.u8(q):data.u16(q);q+=kind==2?1:2;output.attributeIndices[attr]=index;if(attr==9)position=index;
                            if(attr==10){output.normalIndices[0]=index;if(formats.count(10)&&formats.at(10).first==2){for(size_t k=1;k<3;++k){output.normalIndices[k]=kind==2?data.u8(q):data.u16(q);q+=kind==2?1:2;}}}}
                    }
                    assetCheck(position<model.positions.size(),"Position index outside vertex array");assetCheck(palette[slot]<rig.drawMatrices.size(),"Undefined or invalid packet draw matrix");
                    output.position=position;output.drawMatrix=palette[slot];packet.vertices.push_back(output);
                }
                triangulate(primitive,first,count,packet.triangles);
            }
            mesh.vertices+=packet.vertices.size();mesh.triangles+=packet.triangles.size();mesh.packets.push_back(std::move(packet));
        }
    }
    return mesh;
}
inline void meshTests(){
    std::vector<std::array<uint32_t,3>> out;triangulate(0x98,10,4,out);
    assetCheck(out==std::vector<std::array<uint32_t,3>>{{10,11,12},{12,11,13}},"Strip winding test failed");
    out.clear();triangulate(0x80,0,4,out);assetCheck(out==std::vector<std::array<uint32_t,3>>{{0,1,2},{0,2,3}},"Quad triangulation failed");
    out.clear();triangulate(0xa0,0,4,out);assetCheck(out==std::vector<std::array<uint32_t,3>>{{0,1,2},{0,2,3}},"Fan triangulation failed");
    bool rejected=false;try{triangulate(0x90,0,4,out);}catch(const std::runtime_error&){rejected=true;}assetCheck(rejected,"Incomplete primitive accepted");
}



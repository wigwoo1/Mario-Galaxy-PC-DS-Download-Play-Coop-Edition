#pragma once
#include "mesh.hpp"
inline std::map<std::string,BinaryView> modelSections(BinaryView file){
    assetCheck(file.size()>=32&&file.tag(0)=="J3D2"&&file.u32(8)==file.size(),"Invalid material container");
    uint32_t count=file.u32(12);assetCheck(count<=64,"Invalid material section count");std::map<std::string,BinaryView> sections;size_t p=32;
    for(uint32_t i=0;i<count;++i){size_t n=file.u32(p+4);assetCheck(n>=8,"Invalid material section size");assetCheck(sections.emplace(file.tag(p),file.sub(p,n)).second,"Duplicate material section");p+=n;}
    assetCheck(p==file.size(),"Material section length mismatch");return sections;
}
struct AttributeFormat {uint32_t components,type;uint8_t fraction;};
struct VertexAppearance {std::array<float,3> normal{0,0,0};std::array<std::array<float,2>,8> uv{};std::array<Pixel,2> colors{{{255,255,255,255},{255,255,255,255}}};};
class VertexArrays {
    BinaryView view;
    std::map<uint32_t,AttributeFormat> formats;
    std::array<size_t,13> starts{};
    BinaryView array(uint32_t attr,uint32_t index,size_t stride)const{
        size_t slot=attr==9?0:attr==10?(formats.at(10).components?2:1):attr<=12?attr-8:attr-8;
        assetCheck(slot<starts.size()&&starts[slot]!=0,"Missing referenced vertex array");size_t start=starts[slot],end=view.size();
        for(size_t offset:starts)if(offset>start)end=std::min(end,offset);
        uint64_t relative=uint64_t(index)*stride;extent(relative,stride,end-start);return view.sub(start+size_t(relative),stride);
    }
    std::vector<float> scalarValues(uint32_t attr,uint32_t index)const{
        assetCheck(formats.count(attr),"Missing referenced attribute format");auto f=formats.at(attr);assetCheck(f.type<=4&&f.fraction<=31,"Invalid scalar attribute format");
        size_t count=attr==9?(f.components?3:2):attr==10?(f.components==1?9:3):(f.components?2:1);
        size_t size=f.type<=1?1:(f.type<=3?2:4);auto bytes=array(attr,index,count*size);std::vector<float> result;
        unsigned shift=attr==10?(f.type==1?6:(f.type==3?14:0)):f.fraction;
        for(size_t k=0;k<count;++k){size_t p=k*size;float v=f.type==4?bytes.f32(p):float(std::ldexp(double(f.type==0?bytes.u8(p):f.type==1?int8_t(bytes.u8(p)):f.type==2?bytes.u16(p):int16_t(bytes.u16(p))),-int(shift)));result.push_back(v);}return result;
    }
    Pixel color(uint32_t attr,uint32_t index)const{
        assetCheck(formats.count(attr),"Missing color format");auto type=formats.at(attr).type;const size_t sizes[]={2,3,4,2,3,4};assetCheck(type<6,"Invalid vertex color format");auto b=array(attr,index,sizes[type]);
        if(type==0)return rgb565(b.u16(0));if(type==1||type==2)return {b.u8(0),b.u8(1),b.u8(2),255};
        if(type==3){uint16_t v=b.u16(0);return {uint8_t((v>>12)*17),uint8_t(((v>>8)&15)*17),uint8_t(((v>>4)&15)*17),uint8_t((v&15)*17)};}
        if(type==4){uint32_t v=uint32_t(b.u8(0))<<16|uint32_t(b.u8(1))<<8|b.u8(2);return {expand6(v>>18),expand6((v>>12)&63),expand6((v>>6)&63),expand6(v&63)};}
        return {b.u8(0),b.u8(1),b.u8(2),b.u8(3)};
    }
public:
    explicit VertexArrays(BinaryView v):view(v){
        for(size_t i=0;i<13;++i){starts[i]=v.u32(12+i*4);if(starts[i])extent(starts[i],0,v.size());}
        bool end=false;size_t offset=v.u32(8);for(size_t i=0;i<32;++i,offset+=16){uint32_t attr=v.u32(offset);if(attr==255){end=true;break;}
            assetCheck(formats.emplace(attr,AttributeFormat{v.u32(offset+4),v.u32(offset+8),v.u8(offset+12)}).second,"Duplicate attribute format");}assetCheck(end,"Unterminated attribute formats");
    }
    VertexAppearance read(const MeshVertex& v)const{
        VertexAppearance result;if(v.normalIndices[0]!=UINT32_MAX){auto values=scalarValues(10,v.normalIndices[0]);for(size_t k=0;k<3;++k)result.normal[k]=values[k];}
        for(uint32_t i=0;i<2;++i)if(v.attributeIndices[11+i]!=UINT32_MAX)result.colors[i]=color(11+i,v.attributeIndices[11+i]);
        for(uint32_t i=0;i<8;++i)if(v.attributeIndices[13+i]!=UINT32_MAX){auto values=scalarValues(13+i,v.attributeIndices[13+i]);result.uv[i]={values[0],values.size()>1?values[1]:0};}
        return result;
    }
};
struct BasicMaterial {Pixel color{255,255,255,255};std::array<uint16_t,8> textures;BasicMaterial(){textures.fill(0xffff);}};
struct ModelMaterials {std::vector<BasicMaterial> materials;std::vector<uint16_t> shapeMaterial;};
inline ModelMaterials decodeMaterials(BinaryView file,const ModelInfo& model){
    auto sections=modelSections(file);assetCheck(sections.count("MAT3")&&sections.count("INF1"),"Missing material sections");auto mat=sections.at("MAT3");
    uint16_t count=mat.u16(8);assetCheck(count==model.materials,"Material count mismatch");size_t init=mat.u32(12),ids=mat.u32(16),colors=mat.u32(32),textureNos=mat.u32(72);
    ModelMaterials result;extent(ids,size_t(count)*2,mat.size());
    for(size_t i=0;i<count;++i){uint16_t id=mat.u16(ids+i*2);size_t base=init+size_t(id)*332;extent(base,332,mat.size());BasicMaterial material;uint16_t color=mat.u16(base+8);
        if(color!=0xffff){assetCheck(colors!=0,"Missing material color table");size_t p=colors+size_t(color)*4;material.color={mat.u8(p),mat.u8(p+1),mat.u8(p+2),mat.u8(p+3)};}
        for(size_t slot=0;slot<8;++slot){uint16_t index=mat.u16(base+132+slot*2);if(index==0xffff)continue;assetCheck(textureNos!=0,"Missing material texture table");uint16_t texture=mat.u16(textureNos+size_t(index)*2);assetCheck(texture==0xffff||texture<model.textures.size(),"Invalid material texture reference");material.textures[slot]=texture;}
        result.materials.push_back(material);
    }
    auto info=sections.at("INF1");uint16_t current=0xffff;std::vector<uint16_t> stack;result.shapeMaterial.assign(model.shapes,0xffff);bool ended=false;
    for(size_t p=info.u32(20);p+4<=info.size();p+=4){uint16_t type=info.u16(p),index=info.u16(p+2);
        if(type==0){assetCheck(stack.empty(),"Unbalanced material hierarchy");ended=true;break;}
        if(type==1)stack.push_back(current);else if(type==2){assetCheck(!stack.empty(),"Material hierarchy underflow");current=stack.back();stack.pop_back();}
        else if(type==17){assetCheck(index<count,"Invalid hierarchy material");current=index;}
        else if(type==18){assetCheck(index<model.shapes&&current<count,"Invalid shape/material association");assetCheck(result.shapeMaterial[index]==0xffff,"Duplicate shape material assignment");result.shapeMaterial[index]=current;}
        else assetCheck(type==16,"Unknown material hierarchy entry");
    }
    assetCheck(ended,"Missing material hierarchy terminator");for(uint16_t id:result.shapeMaterial)assetCheck(id<count,"Shape missing material association");return result;
}

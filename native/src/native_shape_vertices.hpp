#pragma once
#include "graphics.hpp"
#include <Native/GxCommandState.hpp>
struct NativeShapeVertex {std::array<float,3> position;u16 positionIndex;u8 matrix;std::array<float,3> normal{},texCoord0{},transformedNormal{};u16 normalIndex=0,texCoordIndex=0;bool hasNormal=false,hasTexCoord0=false;std::array<float,3> objectPosition{};std::array<u8,8> textureMatrices{};std::array<std::array<unsigned,4>,2> colors{};std::array<bool,2> hasColor{};std::array<std::array<float,3>,8> texCoords{};std::array<bool,8> hasTexCoords{};};
// Decode indexed geometry using the original shape's configured GX state.
inline std::vector<NativeShapeVertex> decodeShapePositions(const Native::GxCommandState& state,const u8* bytes,size_t size,const Native::GpuResources& resources,std::vector<std::array<u32,3>>* triangles=nullptr){
    std::vector<NativeShapeVertex> vertices;size_t p=0;auto byte=[&](){assetCheck(p<size,"Truncated geometry vertex");return bytes[p++];};
    auto index=[&](unsigned type)->u16{assetCheck(type==2 || type==3,"Unsupported direct geometry attribute");u16 value=byte();if(type==3)value=u16(value<<8)|byte();return value;};
    while(p<size){u8 command=byte();if(!command)continue;unsigned primitive=command&0xf8,vat=command&7;assetCheck(primitive==0x90 || primitive==0x98 || primitive==0xa0 || primitive==0x80,"Unsupported geometry primitive");u16 count=u16(byte())<<8;count|=byte();
        u32 first=u32(vertices.size());
        for(unsigned i=0;i<count;++i){u8 matrix=0;u32 low=state.cp(0x50),high=state.cp(0x60);
            std::array<u8,8> textureMatrices{};
            for(unsigned slot=0;slot<8;++slot)textureMatrices[slot]=u8((state.cp(slot<4?0x30:0x40)>>((slot<4?slot+1:slot-4)*6))&63);
            for(unsigned attr=0;attr<9;++attr)if(low&(1u<<attr)){u8 value=byte();if(attr==0)matrix=value;else textureMatrices[attr-1]=value;}
            u16 position=0;std::array<float,3> value{};
            if(((low>>9)&3)==1){
                auto format=state.positionFormat(vat);unsigned width=format.type<2?1:format.type<4?2:4;size_t length=format.components*width;
                assetCheck(length<=size-p,"Truncated direct geometry position");
                value=state.directPosition(vat,bytes+p,length);p+=length;
            }else{position=index((low>>9)&3);value=state.indexedPosition(vat,position,resources);}
            NativeShapeVertex vertex{state.transformPosition(matrix,value),position,matrix};vertex.objectPosition=value;vertex.textureMatrices=textureMatrices;
            for(unsigned attr=10;attr<=20;++attr){unsigned type=attr<=12?(low>>(11+(attr-10)*2))&3:(high>>((attr-13)*2))&3;if(type){
                if(attr>=13){
                    unsigned channel=attr-13;
                    if(type==1){size_t length=state.directTexCoordSize(vat,channel);assetCheck(length<=size-p,"Truncated direct geometry texture coordinate");vertex.texCoords[channel]=state.directTexCoord(vat,channel,bytes+p,length);p+=length;}
                    else{u16 idx=index(type);vertex.texCoords[channel]=state.indexedTexCoord(vat,channel,idx,resources);if(channel==0)vertex.texCoordIndex=idx;}
                    vertex.hasTexCoords[channel]=true;
                    if(channel==0){vertex.texCoord0=vertex.texCoords[0];vertex.hasTexCoord0=true;}continue;
                }
                if(type==1&&attr==10){
                    size_t length=state.directNormalSize(vat);assetCheck(length<=size-p,"Truncated direct geometry normal");
                    vertex.normal=state.directNormal(vat,bytes+p,length);p+=length;
                    vertex.transformedNormal=state.transformNormal(matrix,vertex.normal);vertex.hasNormal=true;continue;
                }
                if(type==1&&(attr==11||attr==12)){
                    unsigned channel=attr-11;size_t length=state.directColorSize(vat,channel);
                    assetCheck(length<=size-p,"Truncated direct geometry color");
                    vertex.colors[channel]=state.directColor(vat,channel,bytes+p,length);p+=length;vertex.hasColor[channel]=true;continue;
                }
                u16 idx=index(type);
                if(attr==10){vertex.normal=state.indexedNormal(vat,idx,resources);vertex.transformedNormal=state.transformNormal(matrix,vertex.normal);vertex.normalIndex=idx;vertex.hasNormal=true;}
                if(attr==11||attr==12){unsigned channel=attr-11;vertex.colors[channel]=state.indexedColor(vat,channel,idx,resources);vertex.hasColor[channel]=true;}
                if(attr==13){vertex.texCoord0=state.indexedTexCoord0(vat,idx,resources);vertex.texCoordIndex=idx;vertex.hasTexCoord0=true;}}}
            vertices.push_back(vertex);
        }
        auto triangle=[&](u32 a,u32 b,u32 c){if(triangles)triangles->push_back({first+a,first+b,first+c});};
        if(primitive==0x90){assetCheck(count%3==0,"Incomplete geometry triangles");for(u32 i=0;i<count;i+=3)triangle(i,i+1,i+2);}
        else if(primitive==0x80){assetCheck(count%4==0,"Incomplete geometry quads");for(u32 i=0;i<count;i+=4){triangle(i,i+1,i+2);triangle(i,i+2,i+3);}}
        else{assetCheck(count>=3,"Incomplete geometry strip/fan");for(u32 i=2;i<count;++i){if(primitive==0xa0)triangle(0,i-1,i);else if(i&1)triangle(i-1,i-2,i);else triangle(i-2,i-1,i);}}
    }
    return vertices;
}




inline std::vector<NativeShapeVertex> decodeCalledShapePositions(const Native::GxCommandState& state,const u8* command,size_t size,const Native::GpuResources& resources,std::vector<std::array<u32,3>>* triangles=nullptr){
    assetCheck(command && size==9 && command[0]==0x40,"Expected geometry display-list call");u32 address=be32(command+1),length=be32(command+5);
    return decodeShapePositions(state,resources.resolve(address,length),length,resources,triangles);
}







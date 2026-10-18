#pragma once
#include "materials.hpp"
#include "asset_tests.hpp"
inline void materialTests(){
    std::vector<uint8_t> v(256,0);put32(v,8,64);put32(v,16,128);put32(v,24,160);put32(v,32,192);
    put32(v,64,10);put32(v,68,0);put32(v,72,3);v[76]=14;
    put32(v,80,11);put32(v,84,1);put32(v,88,5);
    put32(v,96,13);put32(v,100,1);put32(v,104,3);v[108]=14;put32(v,112,255);
    v[128]=0x20;v[160]=10;v[161]=20;v[162]=30;v[163]=40;v[192]=0x10;v[194]=0x30;
    MeshVertex vertex;vertex.normalIndices[0]=0;vertex.attributeIndices[11]=0;vertex.attributeIndices[13]=0;
    VertexArrays arrays{BinaryView(v)};auto a=arrays.read(vertex);
    assetCheck(a.normal[0]==0.5f&&a.uv[0][0]==0.25f&&a.uv[0][1]==0.75f&&a.colors[0].a==40&&a.colors[0].b==30,"Vertex appearance decoding failed");
    bool rejected=false;vertex.attributeIndices[13]=100;try{arrays.read(vertex);}catch(const std::runtime_error&){rejected=true;}assetCheck(rejected,"Out-of-bounds UV accepted");
    std::vector<uint8_t> file(516,0);std::memcpy(file.data(),"J3D2bdl4",8);put32(file,8,516);put32(file,12,2);
    size_t mat=32;std::memcpy(file.data()+mat,"MAT3",4);put32(file,mat+4,448);file[mat+9]=1;put32(file,mat+12,84);put32(file,mat+16,80);put32(file,mat+32,416);put32(file,mat+72,420);
    for(size_t k=0;k<16;++k)file[mat+84+132+k]=255;file[mat+84+132]=0;file[mat+84+133]=0;
    file[mat+416]=201;file[mat+417]=202;file[mat+418]=203;file[mat+419]=204;
    size_t info=480;std::memcpy(file.data()+info,"INF1",4);put32(file,info+4,36);put32(file,info+20,24);file[info+25]=17;file[info+29]=18;
    ModelInfo model;model.materials=1;model.shapes=1;model.textures.resize(1);
    auto materials=decodeMaterials(BinaryView(file),model);assetCheck(materials.shapeMaterial[0]==0&&materials.materials[0].textures[0]==0&&materials.materials[0].color.r==201,"Material mapping test failed");
    file[mat+421]=2;rejected=false;try{decodeMaterials(BinaryView(file),model);}catch(const std::runtime_error&){rejected=true;}assetCheck(rejected,"Invalid texture mapping accepted");
}

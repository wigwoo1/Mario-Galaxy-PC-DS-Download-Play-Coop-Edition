#pragma once
#include "graphics.hpp"
#include "asset_tests.hpp"
inline bool same(Pixel p,Pixel q) {return p.r==q.r&&p.g==q.g&&p.b==q.b&&p.a==q.a;}
inline void graphicsTests() {
    auto rejects=[](const std::function<void()>& f) {bool caught=false;try{f();}catch(const std::runtime_error&){caught=true;}assetCheck(caught,"Malformed graphics accepted");};
    std::vector<uint8_t> raw(128,0),palette(4,0);palette[0]=0xf8;palette[2]=0x07;palette[3]=0xe0;
    auto decode=[&](uint32_t fmt,uint32_t w=1,uint32_t h=1) {return decodeGxTexture(BinaryView(raw),w,h,fmt,0,BinaryView(palette),2,1);};
    raw[0]=0xf0;assetCheck(same(decode(0).pixels[0],{255,255,255,255}),"I4 decoding failed");
    raw[0]=123;assetCheck(same(decode(1).pixels[0],{123,123,123,123}),"I8 decoding failed");
    raw[0]=0xa3;assetCheck(same(decode(2).pixels[0],{51,51,51,170}),"IA4 decoding failed");
    raw[0]=17;raw[1]=211;assetCheck(same(decode(3).pixels[0],{211,211,211,17}),"IA8 decoding failed");
    raw[0]=0xf8;raw[1]=0;assetCheck(same(decode(4).pixels[0],{255,0,0,255}),"RGB565 decoding failed");
    raw[0]=0xfc;raw[1]=0;assetCheck(same(decode(5).pixels[0],{255,0,0,255}),"RGB5A3 opaque decoding failed");
    raw[0]=0x3f;raw[1]=0;assetCheck(same(decode(5).pixels[0],{255,0,0,109}),"RGB5A3 alpha decoding failed");
    raw[0]=99;raw[1]=11;raw[32]=22;raw[33]=33;assetCheck(same(decode(6).pixels[0],{11,22,33,99}),"RGBA8 planes failed");
    raw[0]=0x10;assetCheck(same(decode(8).pixels[0],{0,255,0,255}),"C4 decoding failed");
    raw[0]=1;assetCheck(same(decode(9).pixels[0],{0,255,0,255}),"C8 decoding failed");
    raw[0]=0xc0;raw[1]=1;assetCheck(same(decode(10).pixels[0],{0,255,0,255}),"C14X2 masking failed");
    raw.assign(128,0);raw[0]=0xf8;raw[2]=0;raw[3]=0x1f;raw[4]=0xb0;
    auto cmpr=decode(14,2,1);assetCheck(same(cmpr.pixels[0],{159,0,95,255})&&same(cmpr.pixels[1],{95,0,159,255}),"CMPR Wii interpolation failed");
    raw[0]=0;raw[1]=0;raw[2]=0xff;raw[3]=0xff;raw[4]=0xc0;
    assetCheck(same(decode(14).pixels[0],{127,127,127,0}),"CMPR transparent color failed");
    raw.assign(128,0);raw[32]=200;raw[64]=100;
    auto edge=decode(1,9,5);assetCheck(edge.pixels[8].r==200&&edge.pixels[36].r==100&&edge.pixels.size()==45,"Clipped tile addressing failed");
    rejects([&]{decode(7);});rejects([&]{decode(1,0,1);});rejects([&]{decode(6,100,100);});
    raw[0]=5;rejects([&]{decode(9);});
    std::vector<uint8_t> tpl(96,0);put32(tpl,0,0x0020af30);put32(tpl,4,1);put32(tpl,8,12);put32(tpl,12,20);
    tpl[21]=1;tpl[23]=1;put32(tpl,24,4);put32(tpl,28,64);tpl[64]=0xf8;
    assetCheck(same(decodeTpl(BinaryView(tpl))[0].pixels[0],{255,0,0,255}),"TPL offsets failed");
    put32(tpl,28,95);rejects([&]{decodeTpl(BinaryView(tpl));});
    std::vector<uint8_t> model(32,0);std::memcpy(model.data(),"J3D2bdl4",8);put32(model,12,6);
    auto section=[&](const char* tag,size_t size) {size_t base=model.size();model.resize(base+size);std::memcpy(model.data()+base,tag,4);put32(model,base+4,uint32_t(size));return base;};
    size_t inf=section("INF1",24);put32(model,inf+16,1);
    size_t vtx=section("VTX1",128);put32(model,vtx+8,64);put32(model,vtx+12,96);put32(model,vtx+64,9);put32(model,vtx+68,1);put32(model,vtx+72,4);put32(model,vtx+80,255);
    put32(model,vtx+96,0x3f800000);put32(model,vtx+100,0x40000000);put32(model,vtx+104,0x40400000);
    section("JNT1",24);section("SHP1",44);section("MAT3",12);
    size_t tex=section("TEX1",96);model[tex+9]=1;put32(model,tex+12,32);model[tex+32]=4;model[tex+35]=1;model[tex+37]=1;put32(model,tex+60,32);model[tex+64]=0xf8;
    put32(model,8,uint32_t(model.size()));auto parsed=decodeModel(BinaryView(model));
    assetCheck(parsed.positions.size()==1&&parsed.positions[0]==std::array<float,3>{1,2,3}&&same(parsed.textures[0].pixels[0],{255,0,0,255}),"J3D geometry/embedded texture failed");
    put32(model,vtx+96,0x7f800000);rejects([&]{decodeModel(BinaryView(model));});put32(model,vtx+96,0x3f800000);
    put32(model,36,0);rejects([&]{decodeModel(BinaryView(model));});
}

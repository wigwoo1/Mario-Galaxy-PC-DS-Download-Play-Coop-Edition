#pragma once
#include "gx_command_texture.hpp"
#include <Native/GxUiVertices.hpp>
#include <memory>
#include <map>
// Explicit original CharWriter format. CP/VAT-driven general draw dispatch remains separate.
struct NativeFontDraw {std::array<Native::UiVertex,4> vertices;std::shared_ptr<const TextureImage> texture;bool alphaBlend=false;bool intensityFont=false;};
inline std::vector<NativeFontDraw> decodeNativeFontDraws(Native::GxCommandState& state,const uint8_t* bytes,size_t size,const Native::GpuResources& resources){
 assetCheck(size<=64*1024*1024,"Font command stream exceeds budget");assetCheck(bytes || !size,"Null font command stream");auto pending=state;std::vector<NativeFontDraw> draws;
 std::map<std::array<uint32_t,4>,std::shared_ptr<const TextureImage>> textures;size_t pixels=0;
 size_t cursor=0;
 while(cursor<size){
  if(bytes[cursor]==0){++cursor;continue;}
  if(bytes[cursor]==0x48){pending.consume(bytes+cursor,1);++cursor;continue;}
  if(bytes[cursor]==0x08){extent(cursor,6,size);pending.consume(bytes+cursor,6);cursor+=6;continue;}
  if(bytes[cursor]==0x10){extent(cursor,5,size);size_t count=(size_t(bytes[cursor+1])<<8|bytes[cursor+2])+1;size_t length=5+count*4;extent(cursor,length,size);pending.consume(bytes+cursor,length);cursor+=length;continue;}
  if(bytes[cursor]==0x61){extent(cursor,5,size);pending.consume(bytes+cursor,5);cursor+=5;continue;}
  if(bytes[cursor]==0x80){extent(cursor,83,size);if(pending.cpConfigured()){auto vat=pending.cp(0x70);assetCheck(pending.cp(0x50)==((1u<<9)|(1u<<13)) && pending.cp(0x60)==1 && (vat&511u)==9 && ((vat>>13)&15u)==11 && ((vat>>21)&511u)==245,"Font draw requires original direct XYZ/RGBA8/ST-U16 format");}auto vertices=Native::decodeFontQuad(bytes+cursor,83);assetCheck(draws.size()<4096,"Font draw budget exceeded");std::array<uint32_t,4> key{pending.bp(0x80),pending.bp(0x84),pending.bp(0x88),pending.bp(0x94)};auto found=textures.find(key);if(found==textures.end()){auto info=pending.texture(0);auto count=size_t(info.width)*info.height;assetCheck(count<=16*1024*1024-pixels,"Font texture budget exceeded");auto texture=std::make_shared<TextureImage>(decodeCommandTexture(pending,0,resources));pixels+=count;found=textures.emplace(key,std::move(texture)).first;}auto blend=pending.bp(0x41);bool alphaBlend=bool(blend&1);assetCheck(!(blend&2) && !(blend&0x800) && (!alphaBlend || (((blend>>8)&7)==4 && ((blend>>5)&7)==5)),"Unsupported font blend equation");draws.push_back({std::move(vertices),found->second,alphaBlend,pending.texture(0).format<=1});cursor+=83;continue;}
  throw std::invalid_argument("Unsupported font command stream opcode");
 }
 state=std::move(pending);return draws;
}






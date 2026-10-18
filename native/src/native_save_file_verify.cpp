#include "native_save_envelope.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc,char** argv){
 try{
  if(argc<2||argc>3)throw std::runtime_error("Supply GameData.bin path and optional --require-created");
  std::ifstream file(std::filesystem::u8path(argv[1]),std::ios::binary);if(!file)throw std::runtime_error("Game-written save file could not be opened");
  std::vector<u8> disk((std::istreambuf_iterator<char>(file)),{}),memory,roundTrip;
  if(disk.empty()||disk.size()>0x10000||!Native::decodeSaveEnvelope(disk.data(),u32(disk.size()),memory))throw std::runtime_error("Save envelope checksum, bounds or layout validation failed");
  if(!Native::encodeSaveEnvelope(memory.data(),u32(memory.size()),roundTrip)||roundTrip!=disk)throw std::runtime_error("Save read/re-encode did not preserve the exact on-disk bytes");
  u32 count=0,size=0;if(!Native::envelopeLayout(memory.data(),u32(memory.size()),false,count,size))throw std::runtime_error("Decoded save layout rejected");
  const auto* entries=reinterpret_cast<const SaveDataFileInfo*>(memory.data()+16);
  std::cout<<"PASS actual game-written save: "<<disk.size()<<" bytes, "<<count<<" entries, valid checksum/endian offsets/bounds, exact decode/re-encode roundtrip\n";
  bool anyCreated=false;
  for(u32 i=0;i<count;++i){
   std::cout<<"entry="<<entries[i].mName<<" offset="<<entries[i].mOffset<<"\n";
   if(std::strncmp(entries[i].mName,"config",6))continue;
   const u32 start=entries[i].mOffset,end=i+1<count?entries[i+1].mOffset:size;
   if(end-start<4||memory[start]!=1)throw std::runtime_error("Invalid profile config chunk header");
   u32 at=start+4;bool found=false,created=false;
   for(unsigned chunk=0;chunk<memory[start+1];++chunk){
    if(at>end||end-at<12)throw std::runtime_error("Truncated profile config chunk");
    const u32 signature=Native::envelopeRead(memory.data()+at),hash=Native::envelopeRead(memory.data()+at+4),length=Native::envelopeRead(memory.data()+at+8);
    if(length<12||length>end-at)throw std::runtime_error("Invalid profile config chunk extent");
    if(signature==0x434f4e46){if(found||hash!=0x2432da||length!=13)throw std::runtime_error("Invalid CONF created-flag schema");const auto value=memory[at+12];if(value!=0&&value!=0xff)throw std::runtime_error("Invalid CONF created flag");found=true;created=value!=0;}
    at+=length;
   }
   if(!found)throw std::runtime_error("Profile config lacks CONF created flag");
   anyCreated|=created;std::cout<<"profile="<<entries[i].mName<<" created="<<created<<"\n";
  }
  if(argc==3&&(std::strcmp(argv[2],"--require-created")||!anyCreated))throw std::runtime_error("Expected at least one persisted created profile");
  if(anyCreated)std::cout<<"PASS at least one actual on-disk profile has its original CONF created flag set\n";
  return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL actual save verification: "<<error.what()<<"\n";return 1;}
}
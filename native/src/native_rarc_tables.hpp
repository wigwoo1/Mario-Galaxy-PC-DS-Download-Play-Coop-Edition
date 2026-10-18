#pragma once
#include "assets.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
// Disk records are 20 bytes; native records include a 64-bit pointer.
// Keep decoded tables separate from immutable archive bytes.
struct NativeRarcTables {
 std::vector<uint8_t> bytes;
 std::vector<JKRArchive::SDIDirEntry> directories;
 std::vector<JKRArchive::SDIFileEntry> files;
 JKRArchive::RarcHeader header{}; JKRArchive::RarcInfoBlock info{};
 size_t strings=0,payload=0;
 explicit NativeRarcTables(std::vector<uint8_t> input):bytes(std::move(input)){
  if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4))bytes=decodeYaz0(bytes);
  else if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4))bytes=decodeYay0(bytes);
  Rarc validated(bytes);assetCheck(bytes.size()>=64 && !std::memcmp(bytes.data(),"RARC",4),"Original JKR tables require RARC");
  auto word=[&](size_t at){extent(at,4,bytes.size());return be32(bytes.data()+at);};
  auto half=[&](size_t at){extent(at,2,bytes.size());return u16(u16(bytes[at])<<8|bytes[at+1]);};
  header={word(0),word(4),word(8),word(12),word(16),word(20),word(24),word(28)};info={word(32),word(36),word(40),word(44),word(48),word(52),half(56),half(58),word(60)};
  assetCheck(uint64_t(header.mMRamDataSize)+header.mARamDataSize<=header.mTotalDataSize,"Invalid RARC memory partition sizes");
  size_t nodes=32+word(36),entries=32+word(44);strings=32+word(52);payload=32+word(12);
  directories.resize(word(32));files.resize(word(40));
  for(size_t i=0;i<directories.size();++i){size_t at=nodes+i*16;directories[i]={word(at),word(at+4),half(at+8),half(at+10),word(at+12)};}
  for(size_t i=0;i<files.size();++i){size_t at=entries+i*20;auto& file=files[i];file.mFileID=half(at);file.mHash=half(at+2);u32 flags=word(at+4);file.mFlag=flags>>24;file.mNameOffset=flags&0xffffff;file.mDataOffset=word(at+8);file.mDataSize=word(at+12);file.mFileData=nullptr;auto name=tableString(bytes,strings+file.mNameOffset,strings+word(48));if(file.mFlag & JKRArchive::FILE_FLAG_FOLDER){if(name!="." && name!="..")assetCheck(file.mDirIndex<directories.size(),"Invalid RARC directory index");}else extent(file.mDataOffset,file.mDataSize,word(16));}for(const auto& directory:directories){extent(directory.mFirstFileIndex,directory.mNrFiles,files.size());tableString(bytes,strings+directory.mNameOffset,strings+word(48));}
 }
 struct Partition {size_t offset,size;};
 Partition mainRamPartition()const{return {payload,header.mMRamDataSize};}
 Partition aramPartition()const{return {payload+header.mMRamDataSize,header.mARamDataSize};}
 Partition dvdPartition()const{return {payload+header.mMRamDataSize+header.mARamDataSize,size_t(header.mTotalDataSize)-header.mMRamDataSize-header.mARamDataSize};}
 // Combined archive offsets are relative to the complete payload, including ARAM entries.
 Partition filePartition(const JKRArchive::SDIFileEntry& file)const{
  assetCheck((file.mFlag & JKRArchive::FILE_FLAG_FILE)!=0,"Partition requested for directory");
  unsigned mode=file.mFlag & 0x70;
  assetCheck(mode==0x10 || mode==0x20 || mode==0x40,"Invalid combined archive storage flags");
  Partition region=mode==0x10?mainRamPartition():mode==0x20?aramPartition():dvdPartition();
  size_t start=payload+size_t(file.mDataOffset);
  assetCheck(start>=region.offset,"Resource precedes its archive partition");
  extent(start-region.offset,file.mDataSize,region.size);
  extent(start,file.mDataSize,bytes.size());
  return {start,file.mDataSize};
 }
 NativeRarcTables(const NativeRarcTables&)=delete;
 NativeRarcTables& operator=(const NativeRarcTables&)=delete;
};






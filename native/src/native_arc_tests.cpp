#include <thread>
#include <atomic>
#include "native_jkr_archive_port.hpp"
#include "native_rarc_tables.hpp"
#include "assets.hpp"
#include "nw4r/lyt/arcResourceAccessor.h"
#include "native_arc_port.hpp"
#include <revolution/arc.h>
#include <vector>
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){std::vector<unsigned char> bytes(68);auto put=[&](size_t p,unsigned value){for(unsigned i=0;i<4;++i)bytes[p+i]=value>>(24-i*8);};put(0,0x55aa382d);put(4,32);put(8,27);put(12,64);put(32,0x01000000);put(40,2);put(44,1);put(48,64);put(52,4);bytes[57]='a';bytes[64]=1;bytes[65]=2;bytes[66]=3;bytes[67]=4;assert(nativeRegisterArc(bytes.data(),bytes.size()));ARCHandle handle{};assert(ARCInitHandle(bytes.data(),&handle));nativeUnregisterArc(bytes.data());bytes.clear();bytes.shrink_to_fit();assert(ARCConvertPathToEntrynum(&handle,"/a")==1);assert(ARCConvertPathToEntrynum(&handle,"./a")==1);assert(ARCConvertPathToEntrynum(&handle,"missing")==-1);assert(!ARCChangeDir(&handle,"a"));assert(ARCChangeDir(&handle,"/.."));char directory[2]{};assert(ARCGetCurrentDir(&handle,directory,2) && directory[0]=='/');assert(!ARCGetCurrentDir(&handle,directory,1));ARCFileInfo file{};assert(ARCFastOpen(&handle,1,&file));assert(ARCGetLength(&file)==4 && static_cast<unsigned char*>(ARCGetStartAddrInMem(&file))[3]==4);assert(!ARCFastOpen(&handle,0,&file));assert(!ARCFastOpen(&handle,2,&file));assert(ARCClose(&file));nativeDetachArc(&handle);assert(!ARCGetStartAddrInMem(&file));bytes.assign(100,0);put(0,0x55aa382d);put(4,32);put(8,55);put(12,96);put(32,0x01000000);put(40,4);put(44,0x01000001);put(48,0);put(52,3);put(56,3);put(60,96);put(64,1);put(68,5);put(72,97);put(76,1);bytes[81]='d';bytes[83]='a';bytes[85]='b';assert(nativeRegisterArc(bytes.data(),bytes.size()));assert(ARCInitHandle(bytes.data(),&handle));assert(ARCConvertPathToEntrynum(&handle,"/d/a")==2);assert(ARCConvertPathToEntrynum(&handle,"/D/A")==2);assert(ARCConvertPathToEntrynum(&handle,"/d/a/")==-1);assert(ARCConvertPathToEntrynum(&handle,"/b")==3);assert(ARCChangeDir(&handle,"d"));assert(ARCConvertPathToEntrynum(&handle,"a")==2);assert(ARCConvertPathToEntrynum(&handle,"../b")==3);assert(ARCConvertPathToEntrynum(&handle,"a/..")==-1);ARCDir enumeration{};ARCDirEntry entry{};assert(ARCOpenDir(&handle,"/",&enumeration));assert(ARCReadDir(&enumeration,&entry) && entry.entryNum==1 && entry.isDir && entry.name[0]=='d');assert(ARCReadDir(&enumeration,&entry) && entry.entryNum==3 && !entry.isDir && entry.name[0]=='b');assert(!ARCReadDir(&enumeration,&entry));assert(ARCCloseDir(&enumeration));assert(ARCOpenDir(&handle,".",&enumeration));assert(ARCReadDir(&enumeration,&entry) && entry.entryNum==2 && entry.name[0]=='a');assert(!ARCReadDir(&enumeration,&entry));assert(!ARCOpenDir(&handle,"a",&enumeration));char nested[4]{};assert(ARCGetCurrentDir(&handle,nested,4) && nested[1]=='d' && nested[2]=='/');nativeDetachArc(&handle);assert(!ARCReadDir(&enumeration,&entry));ARCFileInfo retained{};{nw4r::lyt::ArcResourceAccessor accessor;assert(accessor.Attach(bytes.data(),"/"));nativeUnregisterArc(bytes.data());u32 size=0;assert(accessor.GetResource(0,"A",&size) && size==1);assert(accessor.GetResource(0,"b",&size) && size==1);assert(!accessor.GetResource(0,"missing",&size));assert(!accessor.GetFont("missing"));assert(ARCFastOpen(&accessor.mArcHandle,2,&retained));}assert(!ARCGetStartAddrInMem(&retained));if(argc==2){WiiAssets disc(std::filesystem::u8path(argv[1]));for(const char* archivePath:{"/ObjectData/Mario.arc","/LayoutData/Font.arc"}){
 auto input=disc.readFile(archivePath);Rarc expectedArchive(input);NativeRarcTables host(std::move(input));size_t matched=0,total=0;
 for(const auto& expected:expectedArchive.files){
  auto name=expected.path.substr(expected.path.find_last_of('/')+1);const JKRArchive::SDIFileEntry* match=nullptr;
  for(const auto& file:host.files){if(!(file.mFlag & JKRArchive::FILE_FLAG_FILE))continue;auto fileName=reinterpret_cast<const char*>(host.bytes.data()+host.strings+file.mNameOffset);if(name==fileName && host.payload+file.mDataOffset==expected.offset){assert(!match);match=&file;}}
  assert(match && match->mDataSize==expected.size && !match->mFileData); auto stored=host.filePartition(*match); assert(stored.offset==expected.offset && stored.size==expected.size); auto malformed=*match; malformed.mFlag=(malformed.mFlag & ~0x70)|0x70; bool badFlags=false; try{host.filePartition(malformed);}catch(const std::runtime_error&){badFlags=true;} assert(badFlags); malformed=*match; malformed.mDataOffset=host.header.mTotalDataSize; malformed.mDataSize=1; bool badExtent=false; try{host.filePartition(malformed);}catch(const std::runtime_error&){badExtent=true;} assert(badExtent);auto reference=expectedArchive.readFile(expected.path);assert(reference.empty() || !std::memcmp(host.bytes.data()+host.payload+match->mDataOffset,reference.data(),reference.size()));++matched;total+=reference.size();
 }
 size_t fileCount=0;for(const auto& file:host.files)if(file.mFlag & JKRArchive::FILE_FLAG_FILE)++fileCount;assert(fileCount==matched);
 auto partitionMain=host.mainRamPartition(),partitionAram=host.aramPartition(),partitionDvd=host.dvdPartition();assert(partitionMain.offset==host.payload && partitionAram.offset==partitionMain.offset+partitionMain.size && partitionDvd.offset==partitionAram.offset+partitionAram.size);assert(partitionMain.size+partitionAram.size+partitionDvd.size==host.header.mTotalDataSize);auto partitionBroken=host.bytes;for(unsigned k=20;k<24;++k)partitionBroken[k]=0xff;assert(!nativeRegisterRarc(partitionBroken.data(),partitionBroken.size()));auto broken=host.bytes;broken[4]=0xff;assert(!nativeRegisterRarc(broken.data(),broken.size()));broken=host.bytes;size_t firstNode=32+be32(broken.data()+36);broken[firstNode+10]=0xff;broken[firstNode+11]=0xff;assert(!nativeRegisterRarc(broken.data(),broken.size()));std::printf("PASS native JKR tables %s: %zu resources, %zu exact bytes\n",archivePath,matched,total);
}auto packed=disc.readFile("/LayoutData/Font.arc");if(packed.size()>=4 && !std::memcmp(packed.data(),"Yaz0",4))packed=decodeYaz0(packed);else if(packed.size()>=4 && !std::memcmp(packed.data(),"Yay0",4))packed=decodeYay0(packed);assert(nativeRegisterRarc(packed.data(),packed.size()));assert(!nativeRegisterRarc(packed.data(),packed.size()));assert(!nativeAttachRarc(nullptr,packed.data()));nativeUnregisterRarc(packed.data());std::atomic<unsigned> registrations{0};std::vector<std::thread> workers;for(unsigned i=0;i<8;++i)workers.emplace_back([&]{if(nativeRegisterRarc(packed.data(),packed.size()))++registrations;});for(auto& worker:workers)worker.join();assert(registrations==1);nativeUnregisterRarc(packed.data());NativeRarcTables tables(packed);assert(!tables.directories.empty());for(const auto& file:tables.files){assert(!file.mFileData);if(file.mFlag & JKRArchive::FILE_FLAG_FILE)assert(file.mDataOffset+size_t(file.mDataSize)<=tables.bytes.size()-tables.payload);}Rarc reference(packed);assert(!nativeRegisterArc(packed.data(),packed.size()));assert(packed.size()>=4 && !std::memcmp(packed.data(),"RARC",4));std::printf("PASS retail Font.arc format check: RARC, %zu resources; correctly rejected by U8 adapter\n",reference.files.size());}std::puts("PASS bounded ARC initialization, owned-source lifetime, file extent and detachment");}

















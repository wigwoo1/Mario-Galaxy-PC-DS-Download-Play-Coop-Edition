#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <revolution/dvd.h>
#include <cassert>
#include <cstdio>
#include <set>
#include <string>
#include <vector>
struct Expected {u32 id;bool directory;std::string name;};
int main(int argc,char** argv) {
 assert(argc==2);
 DVDDir untouched{91,92,93};DVDDirEntry output{94,0,nullptr};
 assert(!DVDOpenDir("/",&untouched)&&untouched.entryNum==91);
 assert(!DVDReadDir(&untouched,&output)&&output.entryNum==94);
 assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
 WiiAssets reference(std::filesystem::u8path(argv[1]));
 const auto boot=reference.read(0,0x440);
 const auto fst=reference.read(uint64_t(be32(boot.data()+0x424))*4,size_t(be32(boot.data()+0x428))*4);
 const u32 count=be32(fst.data()+8);
 std::vector<std::vector<Expected>> children(count);
 std::vector<std::string> paths(count);paths[0]="/";
 std::vector<u32> parents{0};
 for(u32 i=1;i<count;++i) {
  while(i>=be32(fst.data()+size_t(parents.back())*12+8))parents.pop_back();
  auto entry=fst.data()+size_t(i)*12;
  auto name=tableString(fst,size_t(count)*12+(be32(entry)&0xffffff),fst.size());
  const bool isDir=entry[0]!=0;
  children[parents.back()].push_back({i,isDir,name});
  paths[i]=paths[parents.back()]+(parents.back()?"/":"")+name;
  if(isDir)parents.push_back(i);
 }
 unsigned directories=0,entries=0,galaxies=0;
 const int stage=DVDConvertPathToEntrynum("/StageData");assert(stage>0);
 for(u32 id=0;id<count;++id) {
  if(fst[size_t(id)*12]==0)continue;
  DVDDir dir{};assert(DVDOpenDir(paths[id].c_str(),&dir));
  assert(dir.entryNum==id&&dir.location==id+1&&dir.next==be32(fst.data()+size_t(id)*12+8));
  // Independent copied cursors behave like SDK value handles.
  DVDDir copy=dir;std::vector<const char*> pointers;
  for(const auto& expected:children[id]) {
   DVDDirEntry found{},again{};
   assert(DVDReadDir(&dir,&found)&&DVDReadDir(&copy,&again));
   assert(found.entryNum==expected.id&&found.isDir==expected.directory);
   assert(found.name&&std::string(found.name)==expected.name);
   assert(found.entryNum==again.entryNum&&std::string(again.name)==expected.name);
   pointers.push_back(found.name);++entries;
   if(int(id)==stage&&found.isDir)++galaxies;
  }
  const auto exhausted=dir.location;
  DVDDirEntry sentinel{123,1,reinterpret_cast<char*>(uintptr_t(1))};
  assert(!DVDReadDir(&dir,&sentinel)&&sentinel.entryNum==123&&dir.location==exhausted);
  for(size_t i=0;i<pointers.size();++i)assert(std::string(pointers[i])==children[id][i].name);
  assert(DVDCloseDir(&dir)&&DVDCloseDir(&dir));++directories;
 }
 assert(galaxies>30);
 assert(!DVDOpenDir("/LayoutData/Font.arc",&untouched)&&untouched.entryNum==91);
 assert(!DVDOpenDir("/missing",&untouched));
 assert(!DVDOpenDir("/",nullptr)&&!DVDOpenDir(nullptr,&untouched));
 assert(!DVDReadDir(nullptr,&output)&&!DVDReadDir(&untouched,nullptr));
 DVDDir malformed{0,count+1,count};assert(!DVDReadDir(&malformed,&output));
 DVDDir stale{};assert(DVDOpenDir("/StageData",&stale));
 nativeReleaseDvd();assert(!DVDReadDir(&stale,&output));
 assert(DVDCloseDir(&stale)&&!DVDCloseDir(nullptr));
 std::printf("PASS DVD directory enumeration: %u directories, %u direct children, %u StageData galaxy directories; original FST IDs/names/order and subtree skipping\n",directories,entries,galaxies);
}

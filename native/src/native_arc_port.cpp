#include "assets.hpp"
#include "native_arc_port.hpp"
#include <revolution/arc.h>
#include <map>
#include <memory>
namespace {
struct ArcOwner {std::vector<uint8_t> bytes;u32 root,count,payload,fstSize;explicit ArcOwner(std::vector<uint8_t> input):bytes(std::move(input)){assetCheck(bytes.size()>=32 && be32(bytes.data())==0x55aa382d,"ARC requires U8 data");Rarc validated(bytes);root=be32(bytes.data()+4);fstSize=be32(bytes.data()+8);payload=be32(bytes.data()+12);count=be32(bytes.data()+root+8);}};
std::map<const void*,std::shared_ptr<ArcOwner>> registered;
std::map<ARCHandle*,std::shared_ptr<ArcOwner>> handles;
}
bool nativeRegisterArc(const void* source,size_t size){if(!source || size>256*1024*1024)return false;try{auto p=static_cast<const uint8_t*>(source);auto owner=std::make_shared<ArcOwner>(std::vector<uint8_t>(p,p+size));return registered.emplace(source,std::move(owner)).second;}catch(const std::exception&){return false;}}
void nativeUnregisterArc(const void* source){registered.erase(source);}
extern "C" BOOL ARCInitHandle(void* source,ARCHandle* handle){auto found=registered.find(source);if(!handle || found==registered.end())return FALSE;auto owner=found->second;ARCHandle next{};next.archiveStartAddr=owner->bytes.data();next.FSTStart=owner->bytes.data()+owner->root;next.fileStart=owner->bytes.data()+owner->payload;next.entryNum=owner->count;next.FSTStringStart=reinterpret_cast<char*>(owner->bytes.data()+owner->root+owner->count*12);next.FSTLength=owner->fstSize;handles[handle]=owner;*handle=next;return TRUE;}
extern "C" BOOL ARCFastOpen(ARCHandle* handle,s32 entry,ARCFileInfo* info){auto found=handles.find(handle);if(!info || found==handles.end() || entry<0 || u32(entry)>=found->second->count)return FALSE;auto& owner=*found->second;auto p=owner.bytes.data()+owner.root+size_t(entry)*12;if(be32(p)>>24)return FALSE;ARCFileInfo next{handle,be32(p+4),be32(p+8)};*info=next;return TRUE;}
extern "C" void* ARCGetStartAddrInMem(ARCFileInfo* info){if(!info)return nullptr;auto found=handles.find(info->handle);if(found==handles.end())return nullptr;auto& owner=*found->second;if(info->startOffset<owner.payload || info->startOffset>owner.bytes.size() || info->length>owner.bytes.size()-info->startOffset)return nullptr;return owner.bytes.data()+info->startOffset;}
extern "C" u32 ARCGetLength(ARCFileInfo* info){return ARCGetStartAddrInMem(info)?info->length:0;}
extern "C" BOOL ARCClose(ARCFileInfo* info){return ARCGetStartAddrInMem(info)!=nullptr;}
void nativeDetachArc(void* handle){handles.erase(static_cast<ARCHandle*>(handle));}

namespace {
s32 resolveArcPath(ARCHandle* handle,const char* path){
 auto found=handles.find(handle);if(found==handles.end() || !path)return -1;
 auto& owner=*found->second;u32 current=path[0]=='/'?0:handle->currDir;
 if(current>=owner.count)return -1;
 size_t cursor=0;
 while(path[cursor]){
  if(cursor>=4096)return -1;
  if(path[cursor]=='/'){++cursor;continue;}
  size_t start=cursor;while(path[cursor] && path[cursor]!='/'){if(cursor>=4096)return -1;++cursor;}
  std::string part(path+start,cursor-start);
  auto node=owner.bytes.data()+owner.root+size_t(current)*12;
  if((be32(node)>>24)!=1)return -1;
  if(part==".")continue;
  if(part==".."){current=be32(node+4);continue;}
  u32 end=be32(node+8),child=current+1;bool matched=false;
  while(child<end){auto p=owner.bytes.data()+owner.root+size_t(child)*12;
   auto name=reinterpret_cast<const char*>(owner.bytes.data()+owner.root+owner.count*12+(be32(p)&0xffffff));
   auto fold=[](unsigned char c){return c>='A' && c<='Z'?c+32:c;};bool same=part.size()==std::strlen(name);for(size_t k=0;same && k<part.size();++k)same=fold(part[k])==fold(name[k]);if(same){current=child;matched=true;break;}
   child=(be32(p)>>24)==1?be32(p+8):child+1;
  }
  if(!matched)return -1;
 }
 if(cursor && path[cursor-1]=='/' && (be32(owner.bytes.data()+owner.root+size_t(current)*12)>>24)!=1)return -1;return s32(current);
}
}
extern "C" s32 ARCConvertPathToEntrynum(ARCHandle* handle,const char* path){return resolveArcPath(handle,path);}
extern "C" BOOL ARCChangeDir(ARCHandle* handle,const char* path){auto entry=resolveArcPath(handle,path);if(entry<0)return FALSE;auto& owner=*handles.find(handle)->second;if((be32(owner.bytes.data()+owner.root+size_t(entry)*12)>>24)!=1)return FALSE;handle->currDir=u32(entry);return TRUE;}
extern "C" BOOL ARCGetCurrentDir(ARCHandle* handle,char* output,u32 capacity){auto found=handles.find(handle);if(found==handles.end() || !output)return FALSE;auto& owner=*found->second;u32 entry=handle->currDir;if(entry>=owner.count)return FALSE;std::string path="/";std::vector<std::string> parts;while(entry){auto p=owner.bytes.data()+owner.root+size_t(entry)*12;if((be32(p)>>24)!=1)return FALSE;parts.emplace_back(reinterpret_cast<const char*>(owner.bytes.data()+owner.root+owner.count*12+(be32(p)&0xffffff)));entry=be32(p+4);if(parts.size()>128)return FALSE;}for(auto i=parts.rbegin();i!=parts.rend();++i){path+=*i;path+='/';}if(path.size()+1>capacity)return FALSE;std::memcpy(output,path.c_str(),path.size()+1);return TRUE;}
extern "C" BOOL ARCOpenDir(ARCHandle* handle,const char* path,ARCDir* directory){if(!directory)return FALSE;auto entry=resolveArcPath(handle,path);if(entry<0)return FALSE;auto& owner=*handles.find(handle)->second;auto node=owner.bytes.data()+owner.root+size_t(entry)*12;if((be32(node)>>24)!=1)return FALSE;*directory=ARCDir{handle,u32(entry),u32(entry)+1,be32(node+8)};return TRUE;}
extern "C" BOOL ARCReadDir(ARCDir* directory,ARCDirEntry* output){
 if(!directory || !output)return FALSE;
 auto found=handles.find(directory->handle);if(found==handles.end())return FALSE;
 auto& owner=*found->second;if(directory->entryNum>=owner.count)return FALSE;
 auto parent=owner.bytes.data()+owner.root+size_t(directory->entryNum)*12;
 if((be32(parent)>>24)!=1 || directory->next!=be32(parent+8))return FALSE;
 u32 location=directory->location;
 while(location>directory->entryNum && location<directory->next){
  auto node=owner.bytes.data()+owner.root+size_t(location)*12;
  char* name=reinterpret_cast<char*>(owner.bytes.data()+owner.root+owner.count*12+(be32(node)&0xffffff));
  if(name[0]=='.' && name[1]==0){++location;continue;}
  BOOL isDirectory=(be32(node)>>24)==1;
  *output=ARCDirEntry{directory->handle,location,isDirectory,name};
  directory->location=isDirectory?be32(node+8):location+1;
  return TRUE;
 }
 return FALSE;
}
extern "C" BOOL ARCCloseDir(ARCDir* directory){return directory && handles.find(directory->handle)!=handles.end();}


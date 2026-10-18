#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <mutex>
#include <revolution/dvd.h>
#include <unordered_map>
#include <thread>
#include <condition_variable>
namespace {
struct DiscCatalog{WiiAssets disc;std::vector<std::string> paths;std::vector<std::array<uint32_t,3>> entries;explicit DiscCatalog(const std::filesystem::path& path):disc(path){auto boot=disc.read(0,0x440);auto fst=disc.read(uint64_t(be32(boot.data()+0x424))*4,size_t(be32(boot.data()+0x428))*4);unsigned count=be32(fst.data()+8);paths.resize(count);entries.resize(count);entries[0]={1,0,count};paths[0]="/";std::vector<std::pair<unsigned,unsigned>> parents{{0,count}};for(unsigned i=1;i<count;++i){while(i>=parents.back().second)parents.pop_back();auto entry=fst.data()+size_t(i)*12;entries[i]={entry[0],be32(entry+4),be32(entry+8)};auto name=tableString(fst,size_t(count)*12+(be32(entry)&0xffffff),fst.size());paths[i]=paths[parents.back().first]+(parents.back().first?"/":"")+name;if(entry[0])parents.push_back({i,be32(entry+8)});}}};
struct DvdJob {bool done=false;bool cancelRequested=false;bool callbackStarted=false;};std::unordered_map<DVDFileInfo*,std::shared_ptr<DvdJob>> jobs;std::condition_variable dvdDone;thread_local DvdJob* currentDvdJob=nullptr;bool releasing=false;
std::mutex dvdMutex;std::unique_ptr<DiscCatalog> catalog;std::unordered_map<DVDFileInfo*,unsigned> handles;
}
bool nativeBindDvd(const std::filesystem::path& path){try{auto next=std::make_unique<DiscCatalog>(path);std::lock_guard<std::mutex> lock(dvdMutex);if(catalog)return false;catalog=std::move(next);return true;}catch(const std::exception&){return false;}}
void nativeReleaseDvd(){std::unique_lock<std::mutex> lock(dvdMutex);releasing=true;dvdDone.wait(lock,[]{for(auto& item:jobs)if(item.second.get()!=currentDvdJob)return false;return true;});handles.clear();catalog.reset();releasing=false;}
bool nativeDvdIsBound(){std::lock_guard<std::mutex> lock(dvdMutex);return catalog!=nullptr;}
extern "C" void DVDInit(void){
    // The host WBFS catalog is bound by the native launcher before game startup.
    // No Broadway DI interrupt or optical-drive initialization is used here.
}
extern "C" s32 DVDGetDriveStatus(void){
    std::lock_guard<std::mutex> lock(dvdMutex);
    return jobs.empty()?DVD_STATE_END:DVD_STATE_BUSY;
}
extern "C" BOOL DVDCheckDiskAsync(DVDCommandBlock* block,DVDCBCallback callback){
    if(!block)return false;
    const BOOL mounted=nativeDvdIsBound()?TRUE:FALSE;
    block->state=0;
    if(callback)callback(mounted,block);
    return true;
}
extern "C" int DVDConvertPathToEntrynum(const char* path){if(!path)return -1;std::string requested(path);if(requested.empty() || requested[0]!='/')requested="/"+requested;auto fold=[](std::string value){for(auto& c:value)if(c>='A' && c<='Z')c+=32;return value;};requested=fold(requested);std::lock_guard<std::mutex> lock(dvdMutex);if(!catalog)return -1;for(size_t i=0;i<catalog->paths.size();++i)if(fold(catalog->paths[i])==requested)return int(i);return -1;}

extern "C" BOOL DVDFastOpen(s32 entry,DVDFileInfo* info){
 std::lock_guard<std::mutex> lock(dvdMutex);
 if(releasing || !catalog || !info || entry<0 || size_t(entry)>=catalog->entries.size() || catalog->entries[entry][0] || handles.count(info))return false;
 handles.emplace(info,unsigned(entry));
 *info={};info->startAddr=catalog->entries[entry][1];info->length=catalog->entries[entry][2];return true;
}
extern "C" BOOL DVDOpen(const char* path,DVDFileInfo* info){return DVDFastOpen(DVDConvertPathToEntrynum(path),info);}
extern "C" BOOL DVDClose(DVDFileInfo* info){std::unique_lock<std::mutex> lock(dvdMutex);dvdDone.wait(lock,[&]{auto job=jobs.find(info);return job==jobs.end() || job->second.get()==currentDvdJob;});return info && handles.erase(info)!=0;}
extern "C" s32 DVDReadPrio(DVDFileInfo* info,void* destination,s32 length,s32 offset,s32 priority){
 (void)priority;
 std::lock_guard<std::mutex> lock(dvdMutex);
 auto handle=handles.find(info);
 auto pending=jobs.find(info);if(pending!=jobs.end() && pending->second.get()!=currentDvdJob)return -1;
 if(!catalog || handle==handles.end() || length<0 || offset<0 || (!destination && length))return -1;
 const auto& entry=catalog->entries[handle->second];
 // Original dvdfs.c permits up to 31 bytes beyond EOF for aligned transfers.
 const uint64_t end=uint64_t(offset)+uint64_t(length);
 if(uint64_t(offset)>entry[2] || end>=uint64_t(entry[2])+32)return -1;
 try{
  auto bytes=catalog->disc.read(uint64_t(entry[1])*4+(uint64_t(offset)>>2)*4,size_t(length));
  if(length)std::memcpy(destination,bytes.data(),bytes.size());
  info->cb.command=1;info->cb.state=0;info->cb.offset=entry[1]+(u32(offset)>>2);
  info->cb.length=u32(length);info->cb.addr=destination;info->cb.currTransferSize=u32(length);info->cb.transferredSize=u32(length);
  return length;
 }catch(const std::exception&){info->cb.state=-1;return -1;}
}
extern "C" s32 DVDGetCommandBlockStatus(const DVDCommandBlock* block){
 std::lock_guard<std::mutex> lock(dvdMutex);
 if(!block)return -1;
 // SDK normalizes cover-check and motor-stopped states for callers.
 return block->state==3?1:block->state==5?4:block->state;
}
extern "C" s32 DVDCancel(DVDCommandBlock* block){
 if(!block)return -1;
 std::unique_lock<std::mutex> lock(dvdMutex);
 DVDFileInfo* owner=nullptr;
 for(const auto& handle:handles)if(&handle.first->cb==block){owner=handle.first;break;}
 auto pending=owner?jobs.find(owner):jobs.end();
 if(pending==jobs.end())return block->state==0||block->state==-1||block->state==DVD_STATE_CANCELED?0:-1;
 const auto job=pending->second;
 // A callback may close/cancel its own file. The worker is about to remove
 // this job, so waiting here would deadlock its completion path.
 if(job.get()==currentDvdJob)return 0;
 if(!job->callbackStarted)job->cancelRequested=true;
 dvdDone.wait(lock,[&]{auto current=jobs.find(owner);return current==jobs.end()||current->second!=job;});
 return 0;
}
bool nativeReadDvdEntry(int id,std::vector<uint8_t>& destination){
 std::lock_guard<std::mutex> lock(dvdMutex);
 if(!catalog || id<0 || size_t(id)>=catalog->entries.size() || catalog->entries[id][0])return false;
 const auto& entry=catalog->entries[id];
 try{auto bytes=catalog->disc.read(uint64_t(entry[1])*4,entry[2]);destination=std::move(bytes);return true;}catch(const std::exception&){return false;}
}

extern "C" BOOL DVDReadAsyncPrio(DVDFileInfo* info,void* destination,s32 length,s32 offset,DVDCallback callback,s32 priority){
 std::unique_lock<std::mutex> lock(dvdMutex);
 auto previous=jobs.find(info);
 if(previous!=jobs.end()){if(previous->second.get()==currentDvdJob || info->cb.state!=0)return false;dvdDone.wait(lock,[&]{return jobs.find(info)==jobs.end();});}
 auto handle=handles.find(info);
 if(releasing || !catalog || handle==handles.end() || jobs.count(info) || length<0 || offset<0 || (!destination && length))return false;
 const auto entry=catalog->entries[handle->second];
 if(uint64_t(offset)>entry[2] || uint64_t(offset)+uint64_t(length)>=uint64_t(entry[2])+32)return false;
 auto job=std::make_shared<DvdJob>();jobs.emplace(info,job);info->callback=callback;info->cb.state=1;
 try{std::thread([=]{
  currentDvdJob=job.get();
  s32 result=-1;std::vector<uint8_t> staging;
  try{staging.resize(size_t(length));result=DVDReadPrio(info,staging.empty()?nullptr:staging.data(),length,offset,priority);}catch(const std::exception&){result=-1;}
  {
   std::lock_guard<std::mutex> finish(dvdMutex);
   if(job->cancelRequested){info->cb.state=DVD_STATE_CANCELED;info->cb.transferredSize=0;result=-3;}
   else if(result>0)std::memcpy(destination,staging.data(),size_t(result));
   info->cb.addr=destination;
   job->callbackStarted=true;
  }
  if(callback)callback(result,info);
  {std::lock_guard<std::mutex> finished(dvdMutex);auto found=jobs.find(info);if(found!=jobs.end() && found->second==job)jobs.erase(found);job->done=true;}
  currentDvdJob=nullptr;dvdDone.notify_all();
 }).detach();return true;}catch(const std::exception&){jobs.erase(info);info->cb.state=-1;return false;}
}



extern "C" BOOL DVDOpenDir(const char* path,DVDDir* dir) {
 if(!dir)return false;
 const int entry=DVDConvertPathToEntrynum(path);
 std::lock_guard<std::mutex> lock(dvdMutex);
 if(releasing||!catalog||entry<0||size_t(entry)>=catalog->entries.size()||
    !catalog->entries[entry][0])return false;
 const auto next=catalog->entries[entry][2];
 if(next<=unsigned(entry)||next>catalog->entries.size())return false;
 *dir={u32(entry),u32(entry)+1,next};return true;
}
extern "C" BOOL DVDReadDir(DVDDir* dir,DVDDirEntry* result) {
 std::lock_guard<std::mutex> lock(dvdMutex);
 if(releasing||!catalog||!dir||!result||dir->entryNum>=catalog->entries.size())return false;
 const auto& parent=catalog->entries[dir->entryNum];
 const u32 location=dir->location;
 if(!parent[0]||parent[2]!=dir->next||location<=dir->entryNum||
    location>=dir->next||location>=catalog->entries.size())return false;
 const auto& entry=catalog->entries[location];
 const u32 following=entry[0]?entry[2]:location+1;
 if(following<=location||following>dir->next)return false;
 // Original SDK points into disc-owned FST storage, valid while mounted.
 auto& fullPath=catalog->paths[location];
 const auto slash=fullPath.find_last_of('/');
 result->entryNum=location;result->isDir=entry[0]!=0;
 result->name=fullPath.data()+(slash==std::string::npos?0:slash+1);
 dir->location=following;return true;
}
extern "C" BOOL DVDCloseDir(DVDDir* dir) {
 // SDK directory handles are value cursors: closing has no side effects.
 return dir!=nullptr;
}

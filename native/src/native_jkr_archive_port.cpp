#include "native_rarc_tables.hpp"
#include "native_jkr_archive_port.hpp"
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <Native/GpuResources.hpp>
#include <map>
#include <memory>
#include <mutex>
namespace {
std::mutex registryMutex;
std::map<const void*,std::shared_ptr<NativeRarcTables>> sources;
std::map<JKRArchive*,std::shared_ptr<NativeRarcTables>> owners;
}
bool nativeRegisterRarc(const void* source,std::size_t size){
 if(!source || size<4 || size>256*1024*1024)return false;
 auto bytes=static_cast<const uint8_t*>(source);
 if(std::memcmp(bytes,"RARC",4) && std::memcmp(bytes,"Yaz0",4) && std::memcmp(bytes,"Yay0",4))return false;
 try{
  auto owner=std::make_shared<NativeRarcTables>(std::vector<uint8_t>(bytes,bytes+size));
  std::lock_guard<std::mutex> lock(registryMutex);
  return sources.emplace(source,std::move(owner)).second;
 }catch(const std::exception&){return false;}
}
void nativeUnregisterRarc(const void* source){std::lock_guard<std::mutex> lock(registryMutex);sources.erase(source);}
bool nativeAttachRarc(JKRMemArchive* archive,const void* source){
 if(!archive)return false;
 std::shared_ptr<NativeRarcTables> snapshot;
 {
  std::lock_guard<std::mutex> lock(registryMutex);
  auto found=sources.find(source);
  if(found!=sources.end())snapshot=found->second;
 }
 // FileRipper registers disc-backed buffers with their exact extent. A few
 // original game archives (notably ErrorMessageArchive) are static RARC
 // arrays and reach JKRMemArchive directly, so no FileRipper registration
 // exists for them. RARC stores its complete byte length in the header.
 if(!snapshot){
  auto bytes=static_cast<const uint8_t*>(source);
  if(!bytes || std::memcmp(bytes,"RARC",4))return false;
  const size_t size=be32(bytes+4);
  if(size<64 || size>256*1024*1024 || !nativeRegisterRarc(source,size)){
   // Another thread may have registered this same source while we parsed it.
   std::lock_guard<std::mutex> lock(registryMutex);
   auto found=sources.find(source);
   if(found==sources.end())return false;
   snapshot=found->second;
  }else{
   std::lock_guard<std::mutex> lock(registryMutex);
   auto found=sources.find(source);
   if(found==sources.end())return false;
   snapshot=found->second;
  }
 }
 std::shared_ptr<NativeRarcTables> owner;
 bool gpuRegistered=false;
 try{
  owner=std::make_shared<NativeRarcTables>(snapshot->bytes);
  Native::gpuResources().addBorrowed(owner->bytes.data(),owner->bytes.size(),owner);
  gpuRegistered=true;
  std::lock_guard<std::mutex> lock(registryMutex);
  if(owners.count(archive)){
   Native::gpuResources().remove(owner->bytes.data());
   return false; // Do not invalidate live resource pointers by rebinding.
  }
  owners.emplace(archive,owner);
  archive->mHeader=&owner->header;archive->mInfoBlock=&owner->info;
  archive->mDirs=owner->directories.data();archive->mFiles=owner->files.data();
  archive->mStringTable=reinterpret_cast<char*>(owner->bytes.data()+owner->strings);
  archive->mFileDataStart=owner->bytes.data()+owner->payload;archive->_6C=false;
  return true;
 }catch(const std::exception&){
  if(gpuRegistered)try{Native::gpuResources().remove(owner->bytes.data());}catch(const std::exception&){}
  return false;
 }
}
void nativeDetachRarc(JKRMemArchive* archive){
 std::lock_guard<std::mutex> lock(registryMutex);
 auto found=owners.find(archive);
 if(found!=owners.end()){
  Native::gpuResources().remove(found->second->bytes.data());
  owners.erase(found);
  archive->mHeader=nullptr;archive->mInfoBlock=nullptr;archive->mDirs=nullptr;archive->mFiles=nullptr;archive->mStringTable=nullptr;archive->mFileDataStart=nullptr;
 }
}
// US reference DOL: archive vtable 0x805d2f00 +0x3c points to 0x802755d8.
// Instructions: lwz r12,0(r3); lwz r12,0x30(r12); mtctr r12; bctr.
// The inherited +0x30 slot is getResSize; preserve virtual dispatch and return bits.
u32 JKRArchive::getExpandedResSize(const void* resource) const{return u32(getResSize(resource));}

// Common metadata ownership for lazy DVD archive fetching; no payload pointer binding.
bool nativeAttachRarcMetadata(JKRArchive* archive,const void* source,std::size_t* payloadOffset){
 if(!archive)return false;
 std::shared_ptr<NativeRarcTables> snapshot;
 {std::lock_guard<std::mutex> lock(registryMutex);auto found=sources.find(source);if(found==sources.end())return false;snapshot=found->second;}
 std::shared_ptr<NativeRarcTables> owner;
 bool gpuRegistered=false;
 try{
  owner=std::make_shared<NativeRarcTables>(snapshot->bytes);
  Native::gpuResources().addBorrowed(owner->bytes.data(),owner->bytes.size(),owner);
  gpuRegistered=true;
  std::lock_guard<std::mutex> lock(registryMutex);
  if(owners.count(archive)){
   Native::gpuResources().remove(owner->bytes.data());
   return false;
  }
  owners.emplace(archive,owner);
  archive->mInfoBlock=&owner->info;archive->mDirs=owner->directories.data();archive->mFiles=owner->files.data();
  archive->mStringTable=reinterpret_cast<char*>(owner->bytes.data()+owner->strings);
  if(payloadOffset)*payloadOffset=owner->payload;
  return true;
 }catch(const std::exception&){
  if(gpuRegistered)try{Native::gpuResources().remove(owner->bytes.data());}catch(const std::exception&){}
  return false;
 }
}
void nativeDetachRarcMetadata(JKRArchive* archive){
 std::lock_guard<std::mutex> lock(registryMutex);
 auto found=owners.find(archive);
 if(found!=owners.end()){
  Native::gpuResources().remove(found->second->bytes.data());
  owners.erase(found);
  archive->mInfoBlock=nullptr;archive->mDirs=nullptr;archive->mFiles=nullptr;archive->mStringTable=nullptr;
 }
}
std::shared_ptr<const NativeRarcTables> nativeOwnedRarcTables(JKRArchive* archive){
 std::lock_guard<std::mutex> lock(registryMutex);
 auto found=owners.find(archive);
 return found==owners.end()?nullptr:found->second;
}

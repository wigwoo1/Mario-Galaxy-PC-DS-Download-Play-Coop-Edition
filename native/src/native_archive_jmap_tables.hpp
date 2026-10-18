#pragma once
#include "owned_archive_resources.hpp"
#include <Game/Util/JMapInfo.hpp>
#include <memory>
#include <Game/Util/Array.hpp>
#include <limits>
#include <functional>
#include <Native/StageArchiveTables.hpp>
class NativeArchiveJMapTables : public NativeStageArchiveTables {
 std::shared_ptr<OwnedArchiveResources> archive;
 std::map<std::string,std::unique_ptr<JMapInfo>> tables;
 static std::string archiveNameKey(std::string value){for(char& c:value)if(c>='A' && c<='Z')c=char(c-'A'+'a');return value;}
public:
 std::function<const JMapInfo*()> objectNameResolver;
 const JMapInfo* objectNameTable() override {assetCheck(bool(objectNameResolver),"Native object-name resolver not bound");auto result=objectNameResolver();assetCheck(result!=nullptr,"Missing native object-name table");return result;}
 std::function<NativeStageArchiveTables*(const char*)> stageResolver;
 NativeStageArchiveTables* resolveStage(const char* name) override {assetCheck(bool(stageResolver),"Native stage resolver not bound");auto resolved=stageResolver(name);assetCheck(resolved!=nullptr,"Native stage resolver returned null");return resolved;}
 explicit NativeArchiveJMapTables(std::shared_ptr<OwnedArchiveResources> source):archive(std::move(source)){assetCheck(bool(archive),"Null table archive");}
  JMapInfo* get(const std::string& requestedPath){
  std::string path;auto wanted=archiveNameKey(requestedPath);
  for(const auto& file:archive->files())if(archiveNameKey(file.path)==wanted){path=file.path;break;}
  if(path.empty())return nullptr;
  auto it=tables.find(path);if(it!=tables.end())return it->second.get();
  auto ptr=archive->get(path);if(!ptr)return nullptr;
  size_t size=archive->sizeOf(ptr);std::vector<uint8_t> bytes(ptr,ptr+size);
  auto table=std::make_unique<JMapInfo>();assetCheck(table->attachNative(bytes),"Invalid native archive JMap");
  auto result=table.get();auto inserted=tables.emplace(path,std::move(table));
  const auto& storedPath=inserted.first->first;result->setName(storedPath.c_str()+storedPath.find_last_of('/')+1);return result;
 }
 void* resource(const char* name) override {
  assetCheck(name!=nullptr,"Null stage resource name");
  for(const auto& file:archive->files())if(archiveNameKey(file.path.substr(file.path.find_last_of('/')+1))==archiveNameKey(name))return const_cast<uint8_t*>(archive->get(file.path));
  return nullptr;
 }
 s32 resourceSize(const void* data) override {
  size_t size=archive->sizeOf(data);assetCheck(size<=size_t(std::numeric_limits<s32>::max()),"Stage resource too large");return s32(size);
 }
 s32 countTables(const char* path) override {
  assetCheck(path!=nullptr,"Null stage table directory");auto children=directory(path);
  assetCheck(children.size()<=size_t(std::numeric_limits<s32>::max()-2),"Too many stage tables");return s32(children.size());
 }
 JMapInfo* appendTables(JMapInfo* destination,const char* path) override {
  assetCheck(path!=nullptr,"Null stage table directory");auto children=directory(path);
  assetCheck(destination || children.empty(),"Null stage table destination");
  for(auto child:children)*destination++=*child;
  return destination;
 }
 // Transfer a contiguous table array to the original stage container. This bridge
 // must outlive that container because original names are borrowed pointers.
 void attachDirectory(MR::AssignableArray<JMapInfo>& target,const std::string& path){
  assetCheck(target.mArr==nullptr && target.mMaxSize==0,"Stage table array already populated");
  auto children=directory(path);
  assetCheck(children.size()<=size_t(std::numeric_limits<s32>::max()),"Too many stage tables");
  if(children.empty())return;
  auto pending=std::make_unique<JMapInfo[]>(children.size());
  for(size_t i=0;i<children.size();++i)pending[i]=*children[i];
  target.mArr=pending.release();target.mMaxSize=s32(children.size());
 }
 // Preserve immediate resource order for original placement table consumers.
 std::vector<JMapInfo*> directory(const std::string& path){
  assetCheck(!path.empty() && path.front()=='/',"Archive table directory must be absolute");
  std::string prefix=archiveNameKey(path);while(prefix.size()>1 && prefix.back()=='/')prefix.pop_back();
  if(prefix.back()!='/')prefix+='/';
  std::vector<JMapInfo*> result;
  for(const auto& file:archive->files())if(archiveNameKey(file.path).compare(0,prefix.size(),prefix)==0 && file.path.find('/',prefix.size())==std::string::npos)result.push_back(get(file.path));
  return result;
 }
};









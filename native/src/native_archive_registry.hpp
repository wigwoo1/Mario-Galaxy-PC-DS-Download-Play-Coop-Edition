#pragma once
#include "owned_archive_resources.hpp"
#include <memory>
class NativeArchiveRegistry {
 std::map<std::string,std::shared_ptr<OwnedArchiveResources>> mounted;
public:
 std::shared_ptr<OwnedArchiveResources> find(const std::string& path)const{
  auto it=mounted.find(path);return it==mounted.end()?nullptr:it->second;
 }
 std::shared_ptr<OwnedArchiveResources> mount(const std::string& path,std::vector<uint8_t> bytes){
  assetCheck(!path.empty(),"Empty archive mount path");
  if(auto existing=find(path))return existing;
  auto archive=std::make_shared<OwnedArchiveResources>(std::move(bytes));
  mounted.emplace(path,archive);return archive;
 }
 bool unmount(const std::string& path){return mounted.erase(path)!=0;}
 size_t count()const{return mounted.size();}
};

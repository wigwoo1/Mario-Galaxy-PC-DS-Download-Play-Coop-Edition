#pragma once
#include "native_archive_registry.hpp"
// Serial disc-backed archive backend; original loader adapter is still required.
class NativeDiscArchiveLoader {
 WiiAssets disc;
 NativeArchiveRegistry archives;
public:
 explicit NativeDiscArchiveLoader(const std::filesystem::path& path):disc(path){}
 std::shared_ptr<OwnedArchiveResources> mount(const std::string& path){
  if(auto existing=archives.find(path))return existing;
  return archives.mount(path,disc.readFile(path));
 }
 std::shared_ptr<OwnedArchiveResources> find(const std::string& path)const{return archives.find(path);}
 bool unmount(const std::string& path){return archives.unmount(path);}
 const std::vector<AssetFile>& files() const{return disc.files;}
 size_t mountedCount()const{return archives.count();}
};


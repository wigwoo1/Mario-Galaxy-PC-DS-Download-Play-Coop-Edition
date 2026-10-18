#include "native_disc_archive_loader.hpp"
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){
 assert(argc==2); WiiAssets disc(std::filesystem::u8path(argv[1]));
 auto bytes=disc.readFile("/ObjectData/Mario.arc"); Rarc reference(bytes);
 NativeArchiveRegistry registry; auto handle=registry.mount("/ObjectData/Mario.arc",bytes);
 auto& resources=*handle;assert(registry.count()==1 && registry.find("/ObjectData/Mario.arc")==handle);
 assert(registry.mount("/ObjectData/Mario.arc",{})==handle);
 bool failed=false;try{registry.mount("/broken.arc",{});}catch(const std::runtime_error&){failed=true;}assert(failed && registry.count()==1);
 std::vector<std::pair<std::string,const uint8_t*>> pointers;
 size_t total=0;
 for(const auto& file:reference.files){
  auto expected=reference.readFile(file.path);
  if(expected.size()>=4 && !std::memcmp(expected.data(),"Yaz0",4))expected=decodeYaz0(expected);
  else if(expected.size()>=4 && !std::memcmp(expected.data(),"Yay0",4))expected=decodeYay0(expected);
  auto p=resources.get(file.path);assert(p);assert(resources.sizeOf(p)==expected.size());
  assert(expected.empty() || std::memcmp(p,expected.data(),expected.size())==0);
  pointers.emplace_back(file.path,p);total+=expected.size();
 }
 for(const auto& pair:pointers)assert(resources.get(pair.first)==pair.second);
 const auto* model=resources.get("/mario.bdl");assert(model && resources.sizeOf(model)>32);
 assert(std::memcmp(model,"J3D2",4)==0);
 assert(resources.get("/not-a-resource")==nullptr);
 assert(registry.unmount("/ObjectData/Mario.arc") && !registry.find("/ObjectData/Mario.arc"));
 assert(std::memcmp(model,"J3D2",4)==0); // An active handle keeps bytes alive after unmount.
 auto replacement=registry.mount("/ObjectData/Mario.arc",std::move(bytes));assert(replacement!=handle);
 assert(!registry.unmount("/missing.arc"));
 NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
 auto mounted=loader.mount("/ObjectData/Mario.arc");assert(loader.mountedCount()==1);
 assert(loader.find("/ObjectData/Mario.arc")==mounted && loader.mount("/ObjectData/Mario.arc")==mounted);
 assert(mounted->sizeOf(mounted->get("/mario.bdl"))==resources.sizeOf(model));
 failed=false;try{loader.mount("/ObjectData/nonexistent-test.arc");}catch(const std::runtime_error&){failed=true;}
 assert(failed && loader.mountedCount()==1 && loader.find("/ObjectData/Mario.arc")==mounted);
 assert(loader.unmount("/ObjectData/Mario.arc") && loader.mountedCount()==0);
 assert(std::memcmp(mounted->get("/mario.bdl"),"J3D2",4)==0);
 size_t stageFiles=0,stageBytes=0;
 for(const char* path:{"/StageData/EggStarGalaxy.arc","/StageData/EggStarGalaxy/EggStarGalaxyScenario.arc"}){
  auto stage=loader.mount(path);Rarc stageReference(disc.readFile(path));
  std::vector<std::pair<std::string,const uint8_t*>> stagePointers;
  for(const auto& file:stageReference.files){
   auto expected=stageReference.readFile(file.path);
   if(expected.size()>=4 && !std::memcmp(expected.data(),"Yaz0",4))expected=decodeYaz0(expected);
   else if(expected.size()>=4 && !std::memcmp(expected.data(),"Yay0",4))expected=decodeYay0(expected);
   auto ptr=stage->get(file.path);assert(ptr && stage->sizeOf(ptr)==expected.size());
   assert(expected.empty() || !std::memcmp(ptr,expected.data(),expected.size()));
   stagePointers.emplace_back(file.path,ptr);++stageFiles;stageBytes+=expected.size();
  }
  for(const auto& pair:stagePointers)assert(stage->get(pair.first)==pair.second);
  assert(loader.unmount(path));
  for(const auto& pair:stagePointers)assert(stage->get(pair.first)==pair.second);
 }
 std::printf("PASS owned EggStar stage/scenario archives: %zu resources, %zu expanded bytes\n",stageFiles,stageBytes);
 std::printf("PASS owned Mario archive: %zu resources, %zu expanded bytes, stable pointers after all lookups\n",pointers.size(),total);
}




#include "native_disc_archive_loader.hpp"
#include "native_archive_jmap_tables.hpp"
#include "stage_data.hpp"
#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightFunction.hpp>
#include <cassert>
#include <cstdio>
namespace {JMapInfo* fixtureLightTable=nullptr;}
s32 LightFunction::createLightDataParser(JMapInfo** output){*output=fixtureLightTable;return fixtureLightTable->getNumEntries();}
int main(int argc,char** argv){
 assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));std::string path;
 for(auto& file:loader.files())if(file.path.substr(file.path.find_last_of('/')+1)=="LightData.arc"){path=file.path;break;}
 assert(!path.empty());auto archive=loader.mount(path);NativeArchiveJMapTables tables(archive);std::string tablePath;
 for(auto& file:archive->files()){auto name=file.path.substr(file.path.find_last_of('/')+1);for(char& c:name)c=char(std::tolower((unsigned char)c));if(name=="lightdata.bcsv")tablePath=file.path;}
 assert(!tablePath.empty());auto info=tables.get(tablePath);assert(info&&info->getNumEntries()>0);
 auto raw=archive->get(tablePath);std::vector<uint8_t> image(raw,raw+archive->sizeOf(raw));StageTable reference{BinaryView(image)};reference.validate();assert(reference.rows==size_t(info->getNumEntries()));unsigned checks=0;
 fixtureLightTable=info;LightDataHolder holder;holder.initLightData();assert(holder.mLightCount==info->getNumEntries()&&holder.getDefaultStepInterpolate()==30);
 for(int row=0;row<info->getNumEntries();++row){
  const auto& area=holder.mLights[row];assert(holder.findAreaLight(area.mAreaLightName)==&holder.mLights[row]);
  auto read=[&](const std::string& key){auto field=reference.find(key);assert(field>=0);++checks;return reference.read(row,field);};
  auto integer=[&](const std::string& key){return std::get<uint32_t>(read(key));};
  assert(std::get<std::string>(read("AreaLightName"))==area.mAreaLightName);assert(integer("Interpolate")==u32(area.mInterpolate));assert(bool(integer("Fix"))==area.mFix);
  std::array<const ActorLightInfo*,4> actors={&area.mPlayerLight,&area.mStrongLight,&area.mWeakLight,&area.mPlanetLight};unsigned group=0;
  for(const char* name:{"Player","Strong","Weak","Planet"}){
   auto actor=actors[group++];assert(integer(std::string(name)+"Alpha2")==actor->mAlpha2);
   auto color=[&](const std::string& key,GXColor c){assert(integer(key+"R")==c.r);assert(integer(key+"G")==c.g);assert(integer(key+"B")==c.b);assert(integer(key+"A")==c.a);};color(std::string(name)+"Ambient",actor->mColor);
   for(unsigned i=0;i<2;++i){auto& light=i?actor->mInfo1:actor->mInfo0;auto key=std::string(name)+"Light"+std::to_string(i);color(key+"Color",light.mColor);assert(std::get<float>(read(key+"PosX"))==light.mPos.x);assert(std::get<float>(read(key+"PosY"))==light.mPos.y);assert(std::get<float>(read(key+"PosZ"))==light.mPos.z);assert(bool(integer(key+"FollowCamera"))==light.mIsFollowCamera);}
  }
 }
 assert(holder.findAreaLight("fixture-unknown-area-light")==holder.mLights);delete[] holder.mLights;
 std::printf("PASS retail original area-light parsing: %zu rows, %u independently checked authored fields (%s)\n",reference.rows,checks,path.c_str());
}



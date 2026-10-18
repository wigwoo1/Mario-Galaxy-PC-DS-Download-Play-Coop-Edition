#include "native_disc_archive_loader.hpp"
#include "native_archive_jmap_tables.hpp"
#include "stage_data.hpp"
#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightFunction.hpp>
#include <Game/Map/LightZoneDataHolder.hpp>
#include <cassert>
#include <cstdio>
#include <deque>
// Fixture provides zone enumeration and archive lookup; original holder owns parsing and selection.
namespace {std::vector<std::string> zones;std::vector<JMapInfo*> zoneTables;constexpr const char* missing="fixture-missing-light";}
namespace MR {s32 getZoneNum(){return s32(zones.size());}const char* getZoneNameFromZoneId(s32 i){return zones.at(i).c_str();}}
s32 LightFunction::createZoneDataParser(const char* name,JMapInfo** output){for(size_t i=0;i<zones.size();++i)if(zones[i]==name){*output=zoneTables[i];return zoneTables[i]->getNumEntries();}throw std::runtime_error("Unknown fixture zone");}
const char* LightFunction::getDefaultAreaLightName(){return missing;}
int main(int argc,char** argv){
 assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));auto archive=loader.mount("/ObjectData/LightData.arc");NativeArchiveJMapTables tables(archive);std::vector<StageTable> references;std::deque<std::vector<uint8_t>> images;
 // StageTable views borrow stable image objects held in the deque.
 for(auto& file:archive->files())if(file.path.size()>5&&file.path.substr(file.path.size()-5)==".bcsv"){
  auto raw=archive->get(file.path);images.emplace_back(raw,raw+archive->sizeOf(raw));StageTable table{BinaryView(images.back())};
  if(table.find("LightID")<0||table.find("AreaLightName")<0)continue;
  references.push_back(table);auto base=file.path.substr(file.path.find_last_of('/')+1);zones.push_back(base);zoneTables.push_back(tables.get(file.path));
 }
 assert(!zones.empty());LightZoneDataHolder holder;holder.initZoneData();assert(holder.mCount==int(zones.size()));unsigned checked=0;
 for(int zone=0;zone<holder.mCount;++zone){auto& original=holder.mZoneInfo[zone];auto& reference=references[zone];assert(size_t(original.mAreaCount)==reference.rows);
  for(size_t row=0;row<reference.rows;++row){auto id=s32(std::get<uint32_t>(reference.read(row,reference.find("LightID"))));auto name=std::get<std::string>(reference.read(row,reference.find("AreaLightName")));assert(original.mAreaInfo[row].mID==id&&name==original.mAreaInfo[row].mAreaLightName);
   ZoneLightID selection;selection._0=zone;selection.mLightID=id;assert(name==holder.getAreaLightNameInZoneData(selection));++checked;
  }
 }
 ZoneLightID fallback;fallback._0=-1;fallback.mLightID=holder.mZoneInfo[0].mAreaInfo[0].mID;assert(std::strcmp(holder.getAreaLightNameInZoneData(fallback),holder.mZoneInfo[0].mAreaInfo[0].mAreaLightName)==0);
 assert(std::strcmp(holder.getDefaultStageAreaLightName(),holder.mZoneInfo[0].getAreaLightNameInZoneData(-1))==0);
 assert(holder.mZoneInfo[0].getAreaLightNameInZoneData(123456)==missing);fallback.clear();assert(fallback._0==-1&&fallback.isOutOfArea());
 for(int zone=0;zone<holder.mCount;++zone)delete[] holder.mZoneInfo[zone].mAreaInfo;delete[] holder.mZoneInfo;
 std::printf("PASS original zone-light initialization/selection: %zu retail zone tables, %u entries, first-zone and missing-ID fallback\n",zones.size(),checked);
}



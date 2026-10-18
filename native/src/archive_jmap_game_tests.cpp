#include "native_disc_archive_loader.hpp"
#include "native_archive_jmap_tables.hpp"
#include "stage_data.hpp"
#include <Game/System/ScenarioDataParser.hpp>
#include <Game/System/GalaxyNameSortTable.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Native/ScenarioBinding.hpp>
#include "native_stage_archive_session.hpp"
#include <cassert>
#include <cstdio>
static size_t fixtureMovementNotifications=0;namespace MR {void notifyRequestNameObjMovementOnOff(){++fixtureMovementNotifications;}}
std::string uppercaseArchiveName(std::string name){for(char& c:name)if(c>='a' && c<='z')c=char(c-'a'+'A');return name;}
template<> MR::AssignableArray<NameObj*>::~AssignableArray(){delete[] mArr;}
template<> MR::AssignableArray<JMapInfo>::~AssignableArray(){delete[] mArr;}
int main(int argc,char** argv){assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));size_t count=0,rows=0,cells=0;
NativeStageArchiveSession session(loader);auto root=session.stage("EggStarGalaxy");assert(root==session.stage("EggStarGalaxy"));
 auto objectNames=root->objectNameTable();assert(objectNames && objectNames->getNumEntries()>0 && objectNames==session.objectNameTable());
  auto nameArchive=loader.mount("/StageData/ObjNameTable.arc");const uint8_t* nameBytes=nullptr;for(const auto& item:nameArchive->files())if(item.path.find("objnametable.tbl")!=std::string::npos)nameBytes=nameArchive->get(item.path);assert(nameBytes);
 std::vector<uint8_t> nameImage(nameBytes,nameBytes+nameArchive->sizeOf(nameBytes));StageTable nameReference{BinaryView(nameImage)};nameReference.validate();assert(nameReference.rows==size_t(objectNames->getNumEntries()));auto englishField=nameReference.find("en_name"),japaneseField=nameReference.find("jp_name");assert(englishField>=0 && japaneseField>=0);
 for(int row=0;row<objectNames->getNumEntries();++row){const char* english=nullptr;const char* japanese=nullptr;assert(objectNames->getValue(row,"en_name",&english));assert(objectNames->getValue(row,"jp_name",&japanese));assert(english && japanese);assert(std::get<std::string>(nameReference.read(row,englishField))==english);assert(std::get<std::string>(nameReference.read(row,japaneseField))==japanese);}
 std::printf("PASS native object names: %d rows\n",objectNames->getNumEntries());
 auto scenario=loader.mount("/StageData/EggStarGalaxy/EggStarGalaxyScenario.arc");NativeArchiveJMapTables scenarioTables(scenario);JMapInfo* zones=nullptr;for(const auto& entry:scenario->files()){if(entry.path.find("zonelist")!=std::string::npos)zones=scenarioTables.get(entry.path);}assert(zones);
 auto scenarios=scenarioTables.get("/ScenarioData.bcsv");assert(scenarios);ScenarioData originalScenario("EggStarGalaxy",*scenarios,*zones);assert(originalScenario.getZoneNum()==zones->getNumEntries()); int visibleScenarios=0,powerStars=0;
 for(int row=0;row<scenarios->getNumEntries();++row){s32 number=0;u32 star=0;bool hidden=false;assert(scenarios->getValue(row,"ScenarioNo",&number));assert(scenarios->getValue(row,"PowerStarId",&star));assert(scenarios->getValue(row,"IsHidden",&hidden));
  u32 originalStar=0;bool originalHidden=false;assert(originalScenario.getValueU32("PowerStarId",number,&originalStar));assert(originalScenario.getValueBool("IsHidden",number,&originalHidden));assert(star==originalStar && hidden==originalHidden);if(star)++powerStars;if(!hidden)++visibleScenarios;
 }
  const ScenarioData* sessionScenario=session.scenario("EggStarGalaxy");assert(sessionScenario==session.scenario("EggStarGalaxy"));assert(std::strcmp(sessionScenario->mGalaxyName,"EggStarGalaxy")==0);
 assert(sessionScenario->getScenarioNum()==visibleScenarios && sessionScenario->getPowerStarNum()==powerStars);assert(sessionScenario->getZoneNum()==zones->getNumEntries());
 for(int zone=0;zone<sessionScenario->getZoneNum();++zone)assert(std::strcmp(sessionScenario->getZoneName(zone),originalScenario.getZoneName(zone))==0);
 assert(originalScenario.getScenarioNum()==visibleScenarios);assert(originalScenario.getPowerStarNum()==powerStars);
 std::printf("PASS original scenario logic: %d visible scenarios, %d Power Stars\n",visibleScenarios,powerStars);
 for(int i=0;i<zones->getNumEntries();++i){const char* zone=nullptr;assert(zones->getValue(i,"ZoneName",&zone));assert(root->resolveStage(zone)==session.stage(zone));assert(std::strcmp(originalScenario.getZoneName(i),zone)==0);assert(originalScenario.getZoneId(zone)==i);}
bool invalidName=false;try{session.stage("../EggStarGalaxy");}catch(const std::runtime_error&){invalidName=true;}assert(invalidName);
std::printf("PASS native zone session: %d scenario zones\n",zones->getNumEntries());
for(const char* name:{"/StageData/EggStarGalaxy.arc","/StageData/EggStarGalaxy/EggStarGalaxyScenario.arc"}){
 auto archive=loader.mount(name);NativeArchiveJMapTables tables(archive);

  for(const auto& resourceFile:archive->files()){
  auto basename=resourceFile.path.substr(resourceFile.path.find_last_of('/')+1);auto resolved=tables.resource(basename.c_str());assert(resolved);assert(tables.resource(uppercaseArchiveName(basename).c_str())==resolved);
  const AssetFile* first=nullptr;for(const auto& candidate:archive->files())if(candidate.path.substr(candidate.path.find_last_of('/')+1)==basename){first=&candidate;break;}
  assert(first);assert(resolved==archive->get(first->path));assert(size_t(tables.resourceSize(resolved))==archive->sizeOf(resolved));
 }
 assert(tables.resource("missing.resource")==nullptr);
 for(const auto& file:archive->files())if(file.path.rfind("/jmp/",0)==0 || (file.path.size()>=5 && file.path.substr(file.path.size()-5)==".bcsv")){
  auto info=tables.get(file.path);assert(info && info==tables.get(file.path));assert(info==tables.get(uppercaseArchiveName(file.path)));
  auto slash=file.path.find_last_of('/');assert(std::strcmp(info->getName(),file.path.c_str()+slash+1)==0);
  auto directory=slash?file.path.substr(0,slash):std::string("/");auto siblings=tables.directory(directory);size_t expectedSiblings=0,index=0;
  for(const auto& sibling:archive->files())if(sibling.path.substr(0,sibling.path.find_last_of('/'))==file.path.substr(0,slash)){assert(index<siblings.size());assert(siblings[index++]==tables.get(sibling.path));++expectedSiblings;}
      assert(siblings.size()==expectedSiblings);assert(siblings==tables.directory(uppercaseArchiveName(directory)));
  NativeStageArchiveTables& binding=tables;assert(size_t(binding.countTables(directory.c_str()))==siblings.size());
  auto nativeArray=std::make_unique<JMapInfo[]>(siblings.size());auto arrayEnd=binding.appendTables(nativeArray.get(),directory.c_str());assert(arrayEnd==nativeArray.get()+siblings.size());
  for(size_t i=0;i<siblings.size();++i){assert(nativeArray[i].mData==siblings[i]->mData);assert(nativeArray[i].mNativeStorage==siblings[i]->mNativeStorage);assert(std::strcmp(nativeArray[i].getName(),siblings[i]->getName())==0);}
  MR::AssignableArray<JMapInfo> attached;tables.attachDirectory(attached,directory);assert(size_t(attached.size())==siblings.size());
  for(size_t i=0;i<siblings.size();++i){assert(attached[int(i)].mData==siblings[i]->mData);assert(attached[int(i)].mNativeStorage==siblings[i]->mNativeStorage);assert(std::strcmp(attached[int(i)].getName(),siblings[i]->getName())==0);}
    for(size_t i=0;i<siblings.size();++i){
   auto table=siblings[i];for(int row=0;row<table->getNumEntries();++row)assert(nativeStageArrayOwns(attached,JMapInfoIter(table,row)));
   assert(!nativeStageArrayOwns(attached,JMapInfoIter(table,-1)));assert(!nativeStageArrayOwns(attached,JMapInfoIter(table,table->getNumEntries())));
  }
  assert(!nativeStageArrayOwns(attached,JMapInfoIter()));
  JMapInfo foreign;auto resource=archive->get(file.path);std::vector<uint8_t> foreignBytes(resource,resource+archive->sizeOf(resource));assert(foreign.attachNative(foreignBytes));
  assert(!nativeStageArrayOwns(attached,JMapInfoIter(&foreign,0)));
  bool duplicateRejected=false;try{tables.attachDirectory(attached,directory);}catch(const std::runtime_error&){duplicateRejected=true;}assert(duplicateRejected);
  delete[] attached.mArr;attached.mArr=nullptr;attached.mMaxSize=0;assert(siblings==tables.directory(directory+"/"));assert(info->getNumEntries()>=0); ++count;rows+=info->getNumEntries();
 auto bytes=archive->get(file.path);std::vector<uint8_t> image(bytes,bytes+archive->sizeOf(bytes));
 StageTable reference{BinaryView(image)};reference.validate();
 assert(size_t(info->getNumEntries())==reference.rows && size_t(info->getNumFields())==reference.fields.size());
 for(size_t r=0;r<reference.rows;++r)for(size_t c=0;c<reference.fields.size();++c){
  auto expected=reference.read(r,c);auto type=reference.fields[c].type;
  if(type==1 || type==6){const char* value=nullptr;assert(info->getValueFast(int(r),int(c),&value));assert(value && std::get<std::string>(expected)==value);}
  else if(type==2){float value;assert(info->getValueFast(int(r),int(c),&value));float wanted=std::get<float>(expected);assert(std::memcmp(&value,&wanted,4)==0);}
  else{u32 value;assert(info->getValueFast(int(r),int(c),&value));assert(value==std::get<uint32_t>(expected));}
  ++cells;
 }
 }
 assert(loader.unmount(name));assert(tables.get("/missing.bcsv")==nullptr);assert(tables.directory("/missing").empty());
 bool rejected=false;try{tables.directory("jmp/Placement");}catch(const std::runtime_error&){rejected=true;}assert(rejected);
}
  bool badScenario=false;try{session.scenario("../EggStarGalaxy");}catch(const std::runtime_error&){badScenario=true;}assert(badScenario);
 bool missingScenario=false;try{session.scenario("NativeMissingGalaxy");}catch(const std::runtime_error&){missingScenario=true;}assert(missingScenario);assert(session.scenario("EggStarGalaxy")==sessionScenario);
 assert(sessionScenario->getPowerStarNum()==powerStars);assert(std::strcmp(sessionScenario->mScenarioData->getName(),"scenariodata.bcsv")==0);
   std::ifstream galaxyFile(std::filesystem::u8path("D:/Desktop/Games/Mario galaxy/Petari/src/Game/System/GalaxyID.bcsv"),std::ios::binary);assert(galaxyFile);std::vector<uint8_t> galaxyImage((std::istreambuf_iterator<char>(galaxyFile)),std::istreambuf_iterator<char>());StageTable galaxyReference{BinaryView(galaxyImage)};galaxyReference.validate();int nameField=galaxyReference.find("name");assert(nameField>=0);
 for(size_t row=0;row<galaxyReference.rows;++row){auto name=std::get<std::string>(galaxyReference.read(row,nameField));size_t first=0;while(std::get<std::string>(galaxyReference.read(first,nameField))!=name)++first;assert(GalaxyNameSortTable::getGalaxySortIndex(name.c_str())==s32(first));}
 assert(GalaxyNameSortTable::getGalaxySortIndex("NativeMissingGalaxy")==-1);
 std::printf("PASS original galaxy ordering: %zu IDs\n",galaxyReference.rows);
 auto allScenarios=session.scenarioList();assert(!allScenarios.empty() && allScenarios.size()<=64);size_t totalScenarioRows=0;
 for(auto data:allScenarios){assert(data && data==session.scenario(data->mGalaxyName));totalScenarioRows+=data->mScenarioData->getNumEntries();}
  SingletonHolder<NameObjRegister>::init();NameObjHolder registeredObjects(8);SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&registeredObjects);
 {
 NativeScenarioSourceBinding startupSource(session);
 assert(nativeScenarioStartupSource==&session);
 ScenarioDataParser originalStartup("OriginalScenarioStartup");
 assert(originalStartup.mScenarioData.size()==allScenarios.size());
 for(int i=0;i<originalStartup.mScenarioData.size();++i) {
  const auto data=originalStartup.getScenarioData(i);
  assert(data&&data==originalStartup.getScenarioData(data->mGalaxyName));
  const auto expected=session.scenario(data->mGalaxyName);
  assert(data->getZoneNum()==expected->getZoneNum());
  assert(data->getScenarioNum()==expected->getScenarioNum());
  assert(data->getPowerStarNum()==expected->getPowerStarNum());
  delete data;
 }
 originalStartup.mScenarioData.mCount=0;
 {NativeScenarioSourceBinding nested(session);assert(nativeScenarioStartupSource==&session);}
 assert(nativeScenarioStartupSource==&session);
}
assert(nativeScenarioStartupSource==nullptr); registeredObjects.clearArray();
std::printf("PASS original one-argument scenario startup with scoped disc source\n");
ScenarioDataParser parser("NativeScenarioParser",&session);assert(parser.mScenarioData.size()==allScenarios.size());assert(registeredObjects.find("NativeScenarioParser")==&parser);assert(fixtureMovementNotifications==0);u32 previous=0;
 for(int i=0;i<parser.mScenarioData.size();++i){auto data=parser.getScenarioData(i);assert(data && data==parser.getScenarioData(data->mGalaxyName));u32 order=GalaxyNameSortTable::getGalaxySortIndex(data->mGalaxyName);assert(order>=previous);previous=order;assert(data->getZoneNum()==session.scenario(data->mGalaxyName)->getZoneNum());}
  assert(parser.getScenarioData("NativeMissingGalaxy")==nullptr);
 {NativeScenarioBinding active(parser,"EggStarGalaxy");assert(ScenarioDataFunction::getScenarioDataParser()==&parser);
  auto current=parser.getScenarioData("EggStarGalaxy");
  for(int row=0;row<current->mScenarioData->getNumEntries();++row){s32 number;assert(current->mScenarioData->getValue(row,"ScenarioNo",&number));for(int zone=0;zone<current->getZoneNum();++zone){auto name=current->getZoneName(zone);u32 mask=0;bool has=current->getValueU32(name,number,&mask);assert(ScenarioDataFunction::getCurrentScenarioLayers(name,number)==(has?mask*2:0));assert(ScenarioDataFunction::getCurrentCommonLayers(name)==1);}}
  assert(ScenarioDataFunction::getCurrentScenarioLayers("NativeMissingZone",1)==0);
  bool rejected=false;try{NativeScenarioBinding invalid(parser,"NativeMissingGalaxy");}catch(const std::runtime_error&){rejected=true;}assert(rejected && nativeScenarioBinding==&active);
  {NativeScenarioBinding nested(parser,parser.getScenarioData(0)->mGalaxyName);assert(nativeScenarioBinding==&nested);}assert(nativeScenarioBinding==&active);
 }assert(nativeScenarioBinding==nullptr);
 std::printf("PASS original scenario layer services\n");for(int i=0;i<parser.mScenarioData.size();++i)delete parser.mScenarioData[i];parser.mScenarioData.mCount=0;
 registeredObjects.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
 std::printf("PASS original scenario parser startup\n");
 std::printf("PASS native scenario source: %zu galaxies, %zu rows\n",allScenarios.size(),totalScenarioRows);
 assert(count>0);std::printf("PASS original JMap archive bridge: %zu tables, %zu rows, %zu cells\n",count,rows,cells);}






























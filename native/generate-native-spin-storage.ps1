$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Replace-Body($source,$signature,$body){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing $signature"}
 $open=$source.IndexOf('{',$start);$end=$open+1;$depth=1
 while($depth -gt 0){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 return $source.Substring(0,$open)+$body+$source.Substring($end)
}
$s='#include <vector>'+[Environment]::NewLine+'#include <cstring>'+[Environment]::NewLine+(Get-Content "$p\Petari\src\Game\System\SpinDriverPathStorage.cpp" -Raw)
$write=@"
{
 std::vector<u8> bytes(2);
 for(int zone=0;zone<=calcMaxZoneId();++zone){
  bool header=false;
  for(s32 i=0;i<mOneStorage.size();++i){
   const auto& one=mOneStorage[i];if(one.mZoneId!=zone)continue;
   if(zone>62 || one.mSpinDriverId<0 || one.mSpinDriverId>63)return -1;
   if(!header){bytes.push_back(u8(0xc0|zone));header=true;}
   u8 record[2]={};const s32 count=one.serialize(record);
   if(count<0||count>2)return -1;
   bytes.insert(bytes.end(),record,record+count);
  }
 }
 bytes.push_back(0xff);
 if(bytes.size()>65535 || bytes.size()>dataSize || !pData)return -1;
 bytes[0]=u8(bytes.size()>>8);bytes[1]=u8(bytes.size());
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}
"@
$read=@"
{
 if(!pData || dataSize<3)return -1;
 const u32 block=(u32(pData[0])<<8)|pData[1];
 if(block<3||block>dataSize)return -1;
 std::vector<SpinDriverPathStorageOne> pending;int zone=-1;
 for(u32 offset=2;offset<block;){
  const u8 token=pData[offset];
  if(token==0xff){
   if(offset+1!=block)return -1;
   resetAllData();for(const auto& one:pending)mOneStorage.push_back(one);
   return s32(block);
  }
  if((token&0xc0)==0xc0){zone=token&0x3f;++offset;continue;}
  if(zone<0 || pending.size()>=16)return -1;
  const u32 count=(token&0x80)?2:1;
  if(count>block-offset)return -1;
  SpinDriverPathStorageOne one;one.deserialize(pData+offset,zone);
  pending.push_back(one);offset+=count;
 }
 return -1;
}
"@
$s=Replace-Body $s 's32 SpinDriverPathStorageScenario::serialize(' $write
$s=Replace-Body $s 's32 SpinDriverPathStorageScenario::deserialize(' $read
$galaxyWrite=@"
{
 if(mNumScenarios<0||mNumScenarios>255||!pData)return -1;
 std::vector<u8> bytes(6);
 for(s32 i=0;i<mNumScenarios;++i){
  u8 scenario[64];const s32 count=mScenarioStorage[i].serialize(scenario,sizeof(scenario));
  if(count<0)return -1;
  bytes.insert(bytes.end(),scenario,scenario+count);
 }
 if(bytes.size()>65535||bytes.size()>maxBufferSize)return -1;
 const u16 hash=MR::getHashCode(mGalaxyName);
 bytes[0]=u8(hash>>8);bytes[1]=u8(hash);bytes[2]=u8(bytes.size()>>8);bytes[3]=u8(bytes.size());bytes[4]=u8(mNumScenarios);
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}
"@
$galaxyRead=@"
{
 if(!pData||maxBufferSize<6)return -1;
 const u16 hash=(u16(pData[0])<<8)|pData[1];
 const u32 block=(u32(pData[2])<<8)|pData[3];
 if(block<6||block>maxBufferSize)return -1;
 if(hash!=u16(MR::getHashCode(mGalaxyName)))return s32(block);
 const u32 count=pData[4];
 std::vector<std::vector<SpinDriverPathStorageOne>> pending(count);
 u32 offset=6;
 for(u32 i=0;i<count;++i){
  SpinDriverPathStorageScenario scenario;
  const s32 consumed=scenario.deserialize(pData+offset,block-offset);
  if(consumed<0)return -1;
  for(s32 j=0;j<scenario.mOneStorage.size();++j)pending[i].push_back(scenario.mOneStorage[j]);
  offset+=u32(consumed);
 }
 if(offset!=block)return -1;
 if(count!=u32(mNumScenarios)){resetAllData();return s32(block);}
 for(s32 i=0;i<mNumScenarios;++i){
  mScenarioStorage[i].resetAllData();
  for(const auto& one:pending[i])mScenarioStorage[i].mOneStorage.push_back(one);
 }
 return s32(block);
}
"@
$s=Replace-Body $s 's32 SpinDriverPathStorageGalaxy::serialize(' $galaxyWrite
$s=Replace-Body $s 's32 SpinDriverPathStorageGalaxy::deserialize(' $galaxyRead
$galaxyCut=$s.IndexOf('SpinDriverPathStorage::SpinDriverPathStorage()')
Set-Content "$n\src\generated\SpinDriverGalaxyCodecFixture.cpp" $s.Substring(0,$galaxyCut)
$topWrite=@"
{
 if(mGalaxyStorage.size()>255||!pData)return -1;
 std::vector<u8> bytes(1,u8(mGalaxyStorage.size()));
 for(s32 i=0;i<mGalaxyStorage.size();++i){
  std::vector<u8> block(65535);
  const s32 count=mGalaxyStorage[i]->serialize(block.data(),u32(block.size()));
  if(count<0)return -1;
  bytes.insert(bytes.end(),block.begin(),block.begin()+count);
 }
 if(bytes.size()>maxBufferSize||bytes.size()>0x7fffffff)return -1;
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}
"@
$topRead=@"
{
 if(!pData||maxBufferSize<1)return -1;
 struct Update {SpinDriverPathStorageGalaxy* galaxy;std::vector<std::vector<SpinDriverPathStorageOne>> scenarios;};
 std::vector<Update> updates;u32 offset=1;bool unknown=false;
 for(u32 i=0;i<pData[0];++i){
  if(maxBufferSize-offset<6)return -1;
  const u8* block=pData+offset;
  const u16 hash=(u16(block[0])<<8)|block[1];
  const u32 size=(u32(block[2])<<8)|block[3];
  if(size<6||size>maxBufferSize-offset)return -1;
  auto* galaxy=findFromHashCode(hash);
  if(!galaxy){unknown=true;offset+=size;continue;}
  Update update{galaxy,{}};update.scenarios.resize(block[4]);
  u32 pos=6;
  for(u32 j=0;j<block[4];++j){
   SpinDriverPathStorageScenario scenario;
   const s32 consumed=scenario.deserialize(block+pos,size-pos);
   if(consumed<0)return -1;
   for(s32 k=0;k<scenario.mOneStorage.size();++k)update.scenarios[j].push_back(scenario.mOneStorage[k]);
   pos+=u32(consumed);
  }
  if(pos!=size)return -1;
  updates.push_back(std::move(update));offset+=size;
 }
 for(const auto& update:updates){
  update.galaxy->resetAllData();
  if(update.scenarios.size()!=u32(update.galaxy->mNumScenarios))continue;
  for(s32 i=0;i<update.galaxy->mNumScenarios;++i)for(const auto& one:update.scenarios[i])update.galaxy->mScenarioStorage[i].mOneStorage.push_back(one);
 }
 return unknown?1:0;
}
"@
$s=Replace-Body $s 's32 SpinDriverPathStorage::serialize(' $topWrite
$s=Replace-Body $s 's32 SpinDriverPathStorage::deserialize(' $topRead
Set-Content "$n\src\generated\SpinDriverPathStorage.cpp" $s
$cut=$s.IndexOf('SpinDriverPathStorageGalaxy::SpinDriverPathStorageGalaxy(')
Set-Content "$n\src\generated\SpinDriverScenarioCodecFixture.cpp" $s.Substring(0,$cut)



$fixture=Replace-Body $s 'SpinDriverPathStorage::SpinDriverPathStorage()' '{mGalaxyStorage.init(50);}'
Set-Content "$n\src\generated\SpinDriverTopCodecFixture.cpp" $fixture

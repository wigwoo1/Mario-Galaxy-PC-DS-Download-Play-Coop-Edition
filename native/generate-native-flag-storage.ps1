$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Replace-Body($source,$signature,$body){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing $signature"}
 $open=$source.IndexOf('{',$start);$end=$open+1;$depth=1
 while($depth -gt 0){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 return $source.Substring(0,$open)+$body+$source.Substring($end)
}
$s='#include <vector>'+[Environment]::NewLine+(Get-Content "$p\Petari\src\Game\System\GameEventFlagStorage.cpp" -Raw)
$write=@"
{
 std::vector<u16> records;
 for(GameEventFlagIter iter=GameEventFlagTable::getBeginIter();!iter.isEnd();iter.goNext()){
  if((iter.getFlag()->mSaveFlag&1)!=0)continue;
  GameEventFlagAccessor accessor(iter.getFlag());
  const u16 hash=MR::getHashCode(accessor.getName())&0x7fff;
  records.push_back(hash|(isOn(iter.getFlag())?0x8000:0));
 }
 if(records.size()>0x7fffffff/2 || records.size()>size/2 || (!pData&&!records.empty()))return -1;
 for(size_t i=0;i<records.size();++i){pData[2*i]=u8(records[i]>>8);pData[2*i+1]=u8(records[i]);}
 return s32(records.size()*2);
}
"@
$read=@"
{
 if((size&1)!=0 || (!pData&&size))return -1;
 std::vector<std::pair<int,bool>> updates;
 bool unknown=false;
 for(u32 offset=0;offset<size;offset+=2){
  const u16 data=(u16(pData[offset])<<8)|pData[offset+1];
  const int index=GameEventFlagTable::getIndexFromHashCode(data&0x7fff);
  if(index<0){unknown=true;continue;}
  if(index>=mFlagBitArray->size())return -1;
  updates.emplace_back(index,(data&0x8000)!=0);
 }
 for(const auto& update:updates)mFlagBitArray->set(update.first,update.second);
 return unknown?1:0;
}
"@
$s=Replace-Body $s 's32 GameEventFlagStorage::serialize(' $write
$s=Replace-Body $s 's32 GameEventFlagStorage::deserialize(' $read
Set-Content "$n\src\generated\GameEventFlagStorage.cpp" $s

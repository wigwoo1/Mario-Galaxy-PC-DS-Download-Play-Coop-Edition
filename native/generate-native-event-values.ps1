$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Replace-Body($source,$signature,$body){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing $signature"}
 $open=$source.IndexOf('{',$start);$end=$open+1;$depth=1
 while($depth -gt 0){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 return $source.Substring(0,$open)+$body+$source.Substring($end)
}
$s='#include <vector>'+[Environment]::NewLine+(Get-Content "$p\Petari\src\Game\System\GameEventValueChecker.cpp" -Raw)
$write=@'
{
 if(mNumValues<0 || u32(mNumValues)>0x7fffffff/4 || u32(mNumValues)>maxBufferSize/4 || (!pData&&mNumValues))return -1;
 for(s32 i=0;i<mNumValues;++i){
  const u16 hash=MR::getHashCode(::cGameEventValueTable[i].mName);
  pData[4*i]=u8(hash>>8);pData[4*i+1]=u8(hash);
  pData[4*i+2]=u8(mValues[i]>>8);pData[4*i+3]=u8(mValues[i]);
 }
 return mNumValues*4;
}
'@
$read=@'
{
 if((maxBufferSize%4)!=0 || (!pData&&maxBufferSize) || mNumValues<0)return -1;
 std::vector<std::pair<s32,u16>> pending;bool unknown=false;
 for(u32 offset=0;offset<maxBufferSize;offset+=4){
  const u16 hash=(u16(pData[offset])<<8)|pData[offset+1];
  const u16 value=(u16(pData[offset+2])<<8)|pData[offset+3];
  const s32 index=findIndexFromHashCode(hash);
  if(index<0){unknown=true;continue;}
  if(index>=mNumValues)return -1;
  pending.emplace_back(index,value);
 }
 for(const auto& update:pending)mValues[update.first]=update.second;
 return unknown?1:0;
}
'@
$s=Replace-Body $s 's32 GameEventValueChecker::serialize(' $write
$s=Replace-Body $s 's32 GameEventValueChecker::deserialize(' $read
Set-Content "$n\src\generated\GameEventValueChecker.cpp" $s

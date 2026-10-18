$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Replace-Body($source,$signature,$body){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing $signature"}
 $open=$source.IndexOf('{',$start);$end=$open+1;$depth=1
 while($depth -gt 0){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 return $source.Substring(0,$open)+$body+$source.Substring($end)
}
$s=Get-Content "$p\Petari\src\Game\System\StarPieceAlmsStorage.cpp" -Raw
$write=@"
{
 if(!pData || size<32)return -1;
 for(int i=0;i<16;++i){pData[2*i]=u8(mValueArray[i]>>8);pData[2*i+1]=u8(mValueArray[i]);}
 return 32;
}
"@
$read=@"
{
 if(!pData || size<32)return -1;
 for(int i=0;i<16;++i)mValueArray[i]=(u16(pData[2*i])<<8)|pData[2*i+1];
 return 0;
}
"@
$s=$s.Replace('serialize(u8* pData, u32)','serialize(u8* pData, u32 size)').Replace('deserialize(const u8* pData, u32)','deserialize(const u8* pData, u32 size)')
$s=Replace-Body $s 's32 StarPieceAlmsStorage::serialize(' $write
$s=Replace-Body $s 's32 StarPieceAlmsStorage::deserialize(' $read
Set-Content "$n\src\generated\StarPieceAlmsStorage.cpp" $s

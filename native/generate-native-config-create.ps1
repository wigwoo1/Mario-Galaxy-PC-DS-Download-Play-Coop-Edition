$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\ConfigDataHolder.cpp" -Raw
$a=$s.IndexOf('s32 ConfigDataCreateChunk::serialize(');$b=$s.IndexOf('void ConfigDataCreateChunk::initializeData()', $a)
$codec=@"
s32 ConfigDataCreateChunk::serialize(u8* buffer,u32 size) const {
 if(!buffer||size<1)return -1;
 buffer[0]=mIsCreated?0xff:0;
 return 1;
}
s32 ConfigDataCreateChunk::deserialize(const u8* buffer,u32 size){
 if(!buffer||size<1)return -1;
 mIsCreated=buffer[0]!=0;
 return 0;
}

"@
if($a -lt 0 -or $b -lt 0){throw 'Missing CONF codec anchors'}
$s=$s.Substring(0,$a)+$codec+$s.Substring($b)
Set-Content "$n\src\generated\ConfigDataHolder.cpp" $s
Set-Content "$n\src\generated\ConfigCreateCodecFixture.cpp" ($s.Substring(0,$s.IndexOf('void ConfigDataHolder::setIsCreated')))

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\ConfigDataMisc.cpp" -Raw
$a=$s.IndexOf('s32 ConfigDataMisc::serialize(');$b=$s.IndexOf('void ConfigDataMisc::initializeData()', $a)
$codec=@"
s32 ConfigDataMisc::serialize(u8* buffer,u32 size) const {
 static_assert(sizeof(OSTime)==8);
 if(!buffer||size<9)return -1;
 buffer[0]=mFlag;
 const u64 value=static_cast<u64>(mLastModified);
 for(int i=0;i<8;++i)buffer[i+1]=u8(value>>(56-i*8));
 return 9;
}
s32 ConfigDataMisc::deserialize(const u8* buffer,u32 size){
 if(!buffer||size<1||(size>1&&size<9))return -1;
 u64 value=0;
 if(size>=9)for(int i=0;i<8;++i)value=(value<<8)|buffer[i+1];
 OSTime timestamp;std::memcpy(&timestamp,&value,sizeof(timestamp));
 mFlag=buffer[0];mLastModified=timestamp;
 return 0;
}

"@
if($a -lt 0 -or $b -lt 0){throw 'Missing MISC codec anchors'}
$s='#include <cstring>'+[Environment]::NewLine+$s.Substring(0,$a)+$codec+$s.Substring($b)
Set-Content "$n\src\generated\ConfigDataMisc.cpp" $s

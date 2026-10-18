$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\ConfigDataMii.cpp" -Raw
$a=$s.IndexOf('s32 ConfigDataMii::serialize(');$b=$s.IndexOf('void ConfigDataMii::initializeData()', $a)
$codec=@"
s32 ConfigDataMii::serialize(u8* buffer,u32 size) const {
 constexpr u32 idSize=sizeof(RFLCreateID);
 if(!buffer||size<idSize+2)return -1;
 buffer[0]=mFlag;std::memcpy(buffer+1,mMiiId,idSize);buffer[idSize+1]=mIconId;
 return idSize+2;
}
s32 ConfigDataMii::deserialize(const u8* buffer,u32 size){
 constexpr u32 idSize=sizeof(RFLCreateID);
 if(!buffer||size<idSize+1)return -1;
 const u8 icon=size==idSize+1?((buffer[0]&FLAG_UNK1)?ICON_ID_MII:ICON_ID_MARIO):buffer[idSize+1];
 mFlag=buffer[0];std::memcpy(mMiiId,buffer+1,idSize);mIconId=icon;
 return 0;
}

"@
if($a -lt 0 -or $b -lt 0){throw 'Missing MII codec anchors'}
$s='#include <cstring>'+[Environment]::NewLine+$s.Substring(0,$a)+$codec+$s.Substring($b)
Set-Content "$n\src\generated\ConfigDataMii.cpp" $s

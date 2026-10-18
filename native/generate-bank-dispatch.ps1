$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASBNKParser.cpp" -Raw
$includes=$s.Substring(0,$s.IndexOf('template < typename T >'))
$topStart=$s.IndexOf('JASBank* JASBNKParser::createBank')
$topEnd=$s.IndexOf('JASBNKParser::Ver1::TChunk*',$topStart)
$top=$s.Substring($topStart,$topEnd-$topStart).Replace('switch (filep->mVersion)','switch (nativeBNKU32(&filep->mVersion))')
$top=$top.Replace('return createBasicBank(stream, heap);','if (!stream) return nullptr;'+[Environment]::NewLine+'    return createBasicBank(stream, heap);')
$body=$s.Substring($s.IndexOf('JASBasicBank* JASBNKParser::Ver0::createBasicBank'))
foreach($owner in @('tinst','tosc','randEffect','senseEffect','tvmap','tpmap')){
 foreach($field in @('mVolume','mPitch','field_0x4','mScale','field_0x14','mBaseValue','mRange','mStartLvl','mEndLvl')){
  $old="$owner->$field";$body=$body.Replace($old,"nativeBankFloat(&$old)")
 }
}
foreach($field in @('tinst->mKeyRegionCount','tkeymap->mVeloRegionCount','tpmap->mVeloRegionCount','tperc->mMagic','tvmap->_4')){$body=$body.Replace($field,"nativeBNKU32(&$field)")}
$body=$body.Replace('tperc->mRelease[j]','nativeBankU16(&tperc->mRelease[j])').Replace('s16 tmp = ptr->_0;','s16 tmp = static_cast<s16>(nativeBankU16(&ptr->_0));')
$copy='JASCalc::bcopy(points, table, size * sizeof(JASOscillator::Point));'
$body=$body.Replace($copy,$copy+[Environment]::NewLine+'                            for(int k=0;k<size;k++){table[k]._0=static_cast<s16>(nativeBankU16(&points[k]._0));table[k]._2=static_cast<s16>(nativeBankU16(&points[k]._2));table[k]._4=static_cast<s16>(nativeBankU16(&points[k]._4));}')
$helpers=@"
#include <Native/HeapPlacement.hpp>
#include <cstring>
"@
$helpers+=[Environment]::NewLine+$includes
$helpers+=@"
static u16 nativeBankU16(const void* p){const auto* b=static_cast<const u8*>(p);return (u16(b[0])<<8)|b[1];}
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
"@
Set-Content "$n\src\generated\BankDispatchKernel.cpp" ($helpers+[Environment]::NewLine+$top+$body)


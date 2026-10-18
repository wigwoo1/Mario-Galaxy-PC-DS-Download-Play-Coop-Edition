$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASBNKParser.cpp" -Raw
$start=$s.IndexOf('JASBasicBank* JASBNKParser::Ver1::createBasicBank')
$end=$s.IndexOf('inline JASInstParam::JASInstParam()',$start)
$body=$s.Substring($start,$end-$start)
$includes=$s.Substring(0,$s.IndexOf('template < typename T >'))
$body=[regex]::Replace($body,'\*(data|ptr)\+\+','nativeBNKU32($1++)')
foreach($field in @('envt_chunk->mSize','list->count','list->mOffsets[i]','op->mTableOffset','op->_10','veloData->_4')){$body=$body.Replace($field,"nativeBNKU32(&$field)")}
foreach($field in @('op->_08','op->mScale','op->_18','veloData->mVolume','veloData->mPitch','instData->mVolume','instData->mPitch','perc_data->mVolume','perc_data->mPitch')){$body=$body.Replace($field,"nativeBankFloat(&$field)")}
$body=$body.Replace('data[0] >> 0x18','nativeBNKU32(data) >> 0x18').Replace('u32 veloCount = data[1];','u32 veloCount = nativeBNKU32(data+1);').Replace('u32 pmap_count = data[1];','u32 pmap_count = nativeBNKU32(data+1);').Replace('perc_data->mRelease','nativeBankU16(&perc_data->mRelease)')
$body=$body.Replace('u8* envt = new', 'if (!envt_chunk || !osc_chunk || !list_chunk) return nullptr;'+[Environment]::NewLine+'    u8* envt = new')
$copy='JASCalc::bcopy(envt_chunk->mData, envt, nativeBNKU32(&envt_chunk->mSize));'
$body=$body.Replace($copy,$copy+[Environment]::NewLine+'    for(u32 j=0;j+2<=nativeBNKU32(&envt_chunk->mSize);j+=2){u16 value=nativeBankU16(envt+j);std::memcpy(envt+j,&value,2);}')
$prefix=@"
#include <Native/HeapPlacement.hpp>
#include <cstring>
static u16 nativeBankU16(const void* p){const auto* b=static_cast<const unsigned char*>(p);return (u16(b[0])<<8)|b[1];}
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
"@
# Helpers follow original headers for SDK types.
$prefix=$prefix.Replace('static u16', $includes+[Environment]::NewLine+'static u16')
Set-Content "$n\src\generated\BankInstrumentKernel.cpp" ($prefix+[Environment]::NewLine+$body)

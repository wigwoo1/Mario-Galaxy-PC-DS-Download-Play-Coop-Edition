$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASBNKParser.cpp" -Raw
$start=$s.IndexOf('JASInstEffect* JASBNKParser::Ver1::createEffector')
$end=$s.IndexOf('JASBasicBank* JASBNKParser::Ver1::createBasicBank',$start)
$s=$s.Substring($start,$end-$start).Replace('switch (*data++)','switch (nativeBNKU32(data++))')
foreach($field in @('randData->mBase','randData->mRange','senseData->mStartLvl','senseData->mEndLvl')){$s=$s.Replace($field,"nativeBankFloat(&$field)")}
$prefix=@"
#include <Native/HeapPlacement.hpp>
#include <cstring>
#include "JSystem/JAudio2/JASBNKParser.hpp"
#include "JSystem/JAudio2/JASInstRand.hpp"
#include "JSystem/JAudio2/JASInstSense.hpp"
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
"@
Set-Content "$n\src\generated\BankEffectorKernel.cpp" ($prefix+[Environment]::NewLine+$s)

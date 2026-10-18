$ErrorActionPreference='Stop'
$p=Split-Path $PSScriptRoot -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\JMapInfo.cpp")
$s=$s.Replace('    case JMAP_VALUE_TYPE_STRING_PTR:','    case JMAP_VALUE_TYPE_STRING_PTR: {').Replace('        *pValueOut = pStringTable + *reinterpret_cast< const u32* >(valuePtr);',"        *pValueOut = pStringTable + *reinterpret_cast< const u32* >(valuePtr);`n    }")
$start=$s.IndexOf('bool JMapInfo::getValueFast(int entryIndex, int itemIndex, s32*')
$a='    const char* valuePtr = getEntryAddress(mData, mData->mDataOffset, entryIndex) + item->mOffsData;'
$i=$s.IndexOf($a,$start);if($i -lt 0){throw 'signed value address anchor'};$s=$s.Remove($i,$a.Length)
$i=$s.IndexOf('    if (item->mShift != 0) {',$start);$s=$s.Insert($i,$a+"`n")
$pattern='\*reinterpret_cast< const (u32|u16|u8|s32|s16|s8)\* >\(valuePtr\)'
if(([regex]::Matches($s,$pattern)).Count -ne 7){throw 'numeric read anchors'}
$s=[regex]::Replace($s,$pattern,'nativeJMapRead<$1>(valuePtr)')

$h=[IO.File]::ReadAllText("$p\Petari\include\Game\Util\JMapInfo.hpp")
$pattern='\*reinterpret_cast< const (f32|u32)\* >\(pValue\)'
if(([regex]::Matches($h,$pattern)).Count -ne 2){throw 'inline read anchors'}
$h=[regex]::Replace($h,$pattern,'nativeJMapRead<$1>(pValue)')
New-Item -ItemType Directory -Force "$PSScriptRoot\include-runtime\Game\Util" | Out-Null

$a='    bool attach(const void* pData);'
if(-not $h.Contains($a)){throw 'JMap attach declaration'}
$h=$h.Replace($a,$a+"`n    bool attachNative(const std::vector<std::uint8_t>&);")
$a='    /* 0x04 */ const char* mName;'
if(-not $h.Contains($a)){throw 'JMap ownership field'}
$h=$h.Replace($a,$a+"`n    std::shared_ptr<NativeJMapStorage> mNativeStorage;")
$h="#include <memory>`n#include <vector>`n#include <cstdint>`nclass NativeJMapStorage;`n"+$h
[IO.File]::WriteAllText("$PSScriptRoot\include-runtime\Game\Util\JMapInfo.hpp","#include <Native/JMapRead.hpp>`n"+$h)
$a='    mData = static_cast< const JMapData* >(pData);'
if(-not $s.Contains($a)){throw 'JMap attach body'}
$s=$s.Replace($a,$a+"`n    if(mNativeStorage && pData!=mNativeStorage->data())mNativeStorage.reset();")
$s+="`nbool JMapInfo::attachNative(const std::vector<std::uint8_t>& bytes){auto owned=std::make_shared<NativeJMapStorage>(bytes);mData=static_cast<const JMapData*>(owned->data());mNativeStorage=std::move(owned);return true;}`n"
[IO.File]::WriteAllText("$PSScriptRoot\src\generated\JMapInfo.cpp","#include <Native/JMapRead.hpp>`n#include `"../native_jmap_storage.hpp`"`n"+$s)

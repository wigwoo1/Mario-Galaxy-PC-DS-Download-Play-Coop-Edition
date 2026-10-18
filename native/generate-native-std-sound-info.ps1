$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUStdSoundInfo.cpp" -Raw
$s=$s.Replace('((JAUSoundTableBgm*)data)->mResourceId','nativeSoundTableU16(&((JAUSoundTableBgm*)data)->mResourceId)')
$s=$s.Replace('((JAUSoundTableSe*)data)->mAudibleSw','nativeSoundTableU16(&((JAUSoundTableSe*)data)->mAudibleSw)')
$s=$s.Replace('u16 bits = data->mChannelCtrl;', 'if (!data) return;'+[Environment]::NewLine+'    u16 bits = nativeSoundTableU16(&data->mChannelCtrl);')
$s=$s.Replace('return (const char*)JASGlobalInstance< JAUSoundTable >::getInstance()->mTable.mData + (u32)data->mStreamFileNameOffset;', 'if (!data) return nullptr;'+[Environment]::NewLine+'        return (const char*)JASGlobalInstance< JAUSoundTable >::getInstance()->mTable.mData + nativeSoundTableU32(&data->mStreamFileNameOffset);')
$s='#include <Native/SoundStreamFilename.hpp>'+[Environment]::NewLine+'#include <cstdint>'+[Environment]::NewLine+$s
$s=$s.Replace('return (const char*)JASGlobalInstance< JAUSoundTable >::getInstance()->mTable.mData + nativeSoundTableU32(&data->mStreamFileNameOffset);', 'auto* table=JASGlobalInstance< JAUSoundTable >::getInstance();'+[Environment]::NewLine+'        if (table->nativeResourceSize) { const auto base=reinterpret_cast<uintptr_t>(table->mTable.mData),record=reinterpret_cast<uintptr_t>(data); if(record<base)return nullptr;return Native::soundStreamFilename(table->mTable.mData,table->nativeResourceSize,record-base); }'+[Environment]::NewLine+'        return (const char*)table->mTable.mData + nativeSoundTableU32(&data->mStreamFileNameOffset);')
Set-Content "$n\src\generated\JAUStdSoundInfo.cpp" $s


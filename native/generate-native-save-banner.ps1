$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\SaveDataBannerCreator.cpp" -Raw
$s=$s.Replace('namespace {','namespace NativeSaveBannerNerves {').Replace('GET_NERVE_ANON(', 'GET_NERVE_DIRECT(NativeSaveBannerNerves, ')
$s=$s.Replace('NANDGetHomeDir(mHomeDir);', 'NANDGetHomeDir(mHomeDir); Native::encodeNandBanner(mBanner);')
$s='#include "../native_nand_banner.hpp"'+[Environment]::NewLine+'#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s
Set-Content "$n\src\generated\SaveDataBannerCreator.cpp" $s


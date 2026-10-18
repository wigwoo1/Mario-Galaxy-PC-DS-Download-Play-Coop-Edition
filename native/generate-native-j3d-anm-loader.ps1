$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphLoader\J3DAnmLoader.cpp")
$s=$s.Replace('(s32)indexPtr0','(std::intptr_t)indexPtr0').Replace('(s32)indexPtr1','(std::intptr_t)indexPtr1').Replace('(s32)dst->mAnmVtxColorIndexData[0][i].mpData','(std::intptr_t)dst->mAnmVtxColorIndexData[0][i].mpData').Replace('(s32)dst->mAnmVtxColorIndexData[1][i].mpData','(std::intptr_t)dst->mAnmVtxColorIndexData[1][i].mpData')
$s="#include <cstdint>`n"+$s
Set-Content "$n\src\generated\J3DAnmLoaderNative.cpp" $s

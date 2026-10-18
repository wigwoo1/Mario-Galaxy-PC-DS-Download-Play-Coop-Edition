$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\NPC\Syati.cpp" -Raw
$s=$s.Replace('initEffectKeeper(1, nullptr, nullptr);','initEffectKeeper(1, nullptr, false);').Replace('_20(&pSyati->mMarioMoveLocalOffsetVec)','_20(reinterpret_cast<Vec*>(&pSyati->mMarioMoveLocalOffsetVec))').Replace('roll.setEulerZ(-1.0f * rad);','roll.setRotate(TVec3f(0.0f, 0.0f, 1.0f), -1.0f * rad);').Replace('roll.setEulerZ(rad);','roll.setRotate(TVec3f(0.0f, 0.0f, 1.0f), rad);').Replace('MR::getRandom(0x78L, 0xF0L)','MR::getRandom(static_cast<s32>(0x78), static_cast<s32>(0xF0))')
Set-Content "$n\src\generated\Syati.cpp" $s

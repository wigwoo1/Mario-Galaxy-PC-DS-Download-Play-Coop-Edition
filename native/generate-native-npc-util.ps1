$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\NPCUtil.cpp" -Raw
$s=$s.Replace('getRandom(0L, 3L)','getRandom(static_cast<s32>(0), static_cast<s32>(3))')
$s=$s.Replace('blendVec(&pActor->mPosition, pActor->mPosition, rPos, posBlend);','{ Vec start{pActor->mPosition.x,pActor->mPosition.y,pActor->mPosition.z}; Vec end{rPos.x,rPos.y,rPos.z}; Vec result; blendVec(&result,start,end,posBlend); pActor->mPosition.set(result.x,result.y,result.z); }')
Set-Content "$n\src\generated\NPCUtil.cpp" $s

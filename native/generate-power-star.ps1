$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\PowerStar.cpp" -Raw
foreach($prefix in @('MR::startBckPlayer(MR::isStageKoopaVs3() ? "DemoKoopaGrandStarVs3" : "DemoKoopaGrandStar",','MR::startBckPlayer(mIsGrandStar ? "GrandStarGet" : "PowerStarGet",')){
$old=$prefix+' 0L);';if(!$s.Contains($old)){throw 'Original PowerStar integer animation anchor changed'}
$s=$s.Replace($old,$prefix+' static_cast<s32>(0));')
}
Set-Content "$n\src\generated\PowerStar.cpp" $s

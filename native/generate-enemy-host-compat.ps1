$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out=Join-Path $n 'src\generated\EnemyCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$legacyIntegerRandom='MR::getRandom\(\s*((?:0[xX][0-9A-Fa-f]+|\d+)[lL])\s*,\s*([^()]+?)\s*\)'

foreach($unit in @('Jellyfish','JellyfishElectric','KirairaChain','Meramera','Metbo','Unizo')){
    $source=Join-Path $p "Petari\src\Game\Enemy\$unit.cpp"
    $text=[System.IO.File]::ReadAllText($source)
    $count=[regex]::Matches($text,$legacyIntegerRandom).Count
    if($count -lt 1){throw "$unit expected at least one integer getRandom call"}
    $text=[regex]::Replace($text,$legacyIntegerRandom,'MR::getRandom(s32($1), s32($2))')
    [System.IO.File]::WriteAllText((Join-Path $out "$unit.cpp"),$text,[System.Text.UTF8Encoding]::new($false))
}

$source=Join-Path $p 'Petari\src\Game\Enemy\BallBeamer.cpp'
$text=[System.IO.File]::ReadAllText($source)
foreach($pair in @(
    @('initModelManagerWithAnm("BallBeamer", nullptr, nullptr);','initModelManagerWithAnm("BallBeamer", nullptr, false);'),
    @('initEffectKeeper(3, nullptr, nullptr);','initEffectKeeper(3, nullptr, false);'),
    @('initSound(2, nullptr);','initSound(2, false);')
)){
    if([regex]::Matches($text,[regex]::Escape($pair[0])).Count -ne 1){throw "BallBeamer expected one host bool adapter site: $($pair[0])"}
    $text=$text.Replace($pair[0],$pair[1])
}
[System.IO.File]::WriteAllText((Join-Path $out 'BallBeamer.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\BegomanBase.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='MR::getWaterAreaObj(&info, mPosition) != nullptr'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'BegomanBase expected one bool water-area predicate'}
$text=$text.Replace($old,'MR::getWaterAreaObj(&info, mPosition) != false')
[System.IO.File]::WriteAllText((Join-Path $out 'BegomanBase.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\Mogucchi.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='MR::calcGravityVector(this, MR::getRailPos(this), &mRailGravity, nullptr, nullptr);'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Mogucchi expected one integer host sentinel for calcGravityVector'}
$text=$text.Replace($old,'MR::calcGravityVector(this, MR::getRailPos(this), &mRailGravity, nullptr, 0);')
[System.IO.File]::WriteAllText((Join-Path $out 'Mogucchi.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\OtaRock.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='mThrowCocoNutCounter(nullptr)'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'OtaRock expected one zero-initialized scalar counter'}
$text=$text.Replace($old,'mThrowCocoNutCounter(0)')
[System.IO.File]::WriteAllText((Join-Path $out 'OtaRock.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\HammerHeadPackun.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='return jmap->mData ? jmap->mData->mNumEntries : nullptr;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'HammerHeadPackun expected one zero entry-count fallback'}
$text=$text.Replace($old,'return jmap->mData ? jmap->mData->mNumEntries : 0;')
[System.IO.File]::WriteAllText((Join-Path $out 'HammerHeadPackun.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Boss\BossKameckStateBattle.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='mBattlePattarn->_10 == nullptr'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'BossKameckStateBattle expected one bool false check'}
$text=$text.Replace($old,'mBattlePattarn->_10 == false')
[System.IO.File]::WriteAllText((Join-Path $out 'BossKameckStateBattle.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Boss\DinoPackunTailNode.cpp'
$text=[System.IO.File]::ReadAllText($source)
if([regex]::Matches($text,'\bM_PI\b').Count -lt 1){throw 'DinoPackunTailNode expected host-independent PI constant sites'}
$text=[regex]::Replace($text,'\bM_PI\b','3.14159265358979323846')
[System.IO.File]::WriteAllText((Join-Path $out 'DinoPackunTailNode.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Boss\Dodoryu.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old="        1600.0f,`r`n        5.0f,`r`n        -1,`r`n        45,`r`n        -1,"
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Dodoryu expected one u32 all-bits-set sentinel'}
$text=$text.Replace($old,"        1600.0f,`r`n        5.0f,`r`n        -1,`r`n        45,`r`n        0xFFFFFFFFu,")
$old='Vec* pY = &yDir;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Dodoryu expected one SDK-layout Y-axis pointer adapter'}
$text=$text.Replace($old,'Vec* pY = reinterpret_cast<Vec *>(&yDir);')
$old='pZ = &zDir;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Dodoryu expected one SDK-layout Z-axis pointer adapter'}
$text=$text.Replace($old,'pZ = reinterpret_cast<Vec *>(&zDir);')
$old='PSVECCrossProduct(pZ, &xDir, pY);'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Dodoryu expected one SDK-layout first cross-product adapter'}
$text=$text.Replace($old,'PSVECCrossProduct(pZ, reinterpret_cast<Vec *>(&xDir), pY);')
$old='PSVECCrossProduct(&xDir, pY, pZ);'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Dodoryu expected one SDK-layout second cross-product adapter'}
$text=$text.Replace($old,'PSVECCrossProduct(reinterpret_cast<Vec *>(&xDir), pY, pZ);')
[System.IO.File]::WriteAllText((Join-Path $out 'Dodoryu.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Boss\TripodBossAccesser.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='u32 getTripodBossGravityHostID() {'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'TripodBossAccesser expected one host-width return declaration'}
$text=$text.Replace($old,'std::uintptr_t getTripodBossGravityHostID() {')
$old='reinterpret_cast< u32 >(getTripodBossAccesser())'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'TripodBossAccesser expected one host pointer width cast'}
$text=$text.Replace($old,'reinterpret_cast< std::uintptr_t >(getTripodBossAccesser())')
[System.IO.File]::WriteAllText((Join-Path $out 'TripodBossAccesser.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Boss\TripodBossKillerGeneraterCircle.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='bool mHasCollision;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'TripodBossKillerGeneraterCircle expected one bitmask storage type'}
$text=$text.Replace($old,'u32 mHasCollision;')
[System.IO.File]::WriteAllText((Join-Path $out 'TripodBossKillerGeneraterCircle.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\CannonShellBase.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='std::ptr_fun(&MR::isDead)'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'CannonShellBase expected one removed ptr_fun predicate'}
$text=$text.Replace($old,'[](const CannonShellBase* pShell) { return MR::isDead(pShell); }')
[System.IO.File]::WriteAllText((Join-Path $out 'CannonShellBase.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

$source=Join-Path $p 'Petari\src\Game\Enemy\KoopaJrShip.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='std::not1(std::ptr_fun(&MR::isDead))'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 2){throw 'KoopaJrShip expected two legacy active-object predicates'}
$text=$text.Replace($old,'[](const LiveActor* pActor) { return !MR::isDead(pActor); }')
$old='std::mem_func(&HomingKiller::makeActorDead)'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'KoopaJrShip expected one legacy member-function adapter'}
$text=$text.Replace($old,'[](HomingKiller* pActor) { pActor->makeActorDead(); }')
[System.IO.File]::WriteAllText((Join-Path $out 'KoopaJrShip.cpp'),$text,[System.Text.UTF8Encoding]::new($false))

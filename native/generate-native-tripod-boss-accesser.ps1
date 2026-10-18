$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
New-Item -ItemType Directory -Force "$n\include\Game\Boss" | Out-Null
$h=Get-Content "$p\Petari\include\Game\Boss\TripodBossAccesser.hpp" -Raw
if(!$h.Contains('u32 getTripodBossGravityHostID()')){throw 'Missing gravity host declaration'}
Set-Content "$n\include\Game\Boss\TripodBossAccesser.hpp" ('#include <cstdint>'+"`n"+$h.Replace('u32 getTripodBossGravityHostID()','std::uintptr_t getTripodBossGravityHostID()'))
$s=Get-Content "$p\Petari\src\Game\Boss\TripodBossAccesser.cpp" -Raw
$s=$s.Replace('u32 getTripodBossGravityHostID()', 'std::uintptr_t getTripodBossGravityHostID()').Replace('reinterpret_cast< u32 >(getTripodBossAccesser())','reinterpret_cast< std::uintptr_t >(getTripodBossAccesser())')
Set-Content "$n\src\generated\TripodBossAccesser.cpp" $s

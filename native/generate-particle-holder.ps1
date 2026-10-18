$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Effect\ParticleEmitterHolder.cpp" -Raw
$old='std::find_if_array(mEmitters.begin(), mEmitters.end(), std::not1(std::mem_func(&ParticleEmitter::isValid)))'
if(!$s.Contains($old)){throw 'Original available-emitter search anchor missing'}
$s=$s.Replace($old,'std::find_if(mEmitters.begin(), mEmitters.end(), [](const ParticleEmitter& emitter) { return !emitter.isValid(); })')
Set-Content "$n\src\generated\ParticleEmitterHolder.cpp" ('#include <algorithm>'+"`n"+$s)

param([switch]$OriginalHeader)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
if(!$OriginalHeader){& "$n\generate-jpa-emitter-header.ps1"}
$out=Join-Path $n 'build\particle-link-test';New-Item -ItemType Directory -Force -Path $out | Out-Null
$original=[IO.File]::ReadAllText("$p\Petari\src\Game\Effect\EffectSystemUtil.cpp")
$kernel='#include <Game/Effect/EffectSystemUtil.hpp>'+"`n"+'#include <Game/Effect/SingleEmitter.hpp>'+"`n"+'#include <JSystem/JParticle/JPAEmitter.hpp>'+"`nnamespace MR { namespace Effect {`n"
foreach($signature in @('void setLinkSingleEmitter(','SingleEmitter* getLinkSingleEmitter(')){
 $start=$original.IndexOf($signature);if($start-lt 0){throw "Original particle link function missing: $signature"}
 $open=$original.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth){if($original[$end]-eq '{'){$depth++};if($original[$end]-eq '}'){$depth--};$end++}
 $kernel+=$original.Substring($start,$end-$start)+"`n"
}
$kernel+="} }`n"
foreach($unit in @(
 @{Path="$p\Petari\src\Game\Effect\SingleEmitter.cpp";Signatures=@('SingleEmitter::SingleEmitter()','void SingleEmitter::link(','void SingleEmitter::unlink(')},
 @{Path="$p\Petari\src\Game\Effect\ParticleEmitter.cpp";Signatures=@('ParticleEmitter::ParticleEmitter()')}
)){
 $text=[IO.File]::ReadAllText($unit.Path)
 foreach($signature in $unit.Signatures){
  $start=$text.IndexOf($signature);if($start-lt 0){throw "Original lifecycle function missing: $signature"}
  $open=$text.IndexOf('{',$start);$depth=1;$end=$open+1
  while($depth){if($text[$end]-eq '{'){$depth++};if($text[$end]-eq '}'){$depth--};$end++}
  $kernel+=$text.Substring($start,$end-$start)+"`n"
 }
}
$kernelPath=Join-Path $out 'original-particle-link.cpp';[IO.File]::WriteAllText($kernelPath,$kernel)
$arguments=@('-std=c++17','-O2','-UNDEBUG','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu')
if($OriginalHeader){
 $originalInclude=Join-Path $out 'original-layout';$directory=Join-Path $originalInclude 'JSystem\JParticle';New-Item -ItemType Directory -Force -Path $directory | Out-Null
 Copy-Item -LiteralPath "$p\Petari\libs\JSystem\include\JSystem\JParticle\JPAEmitter.hpp" -Destination "$directory\JPAEmitter.hpp"
 $arguments+=('-I'+$originalInclude)
}
foreach($include in @("$n\include-runtime","$n\include","$p\Petari\include","$p\Petari\libs\JSystem\include","$p\Petari\libs\RVL_SDK\include")){$arguments+=('-I'+$include)}
$arguments+=@('-include','revolution/types.h',"$n\src\particle_link_tests.cpp",$kernelPath,"$p\Petari\src\JSystem\JSupport\JSUList.cpp",'-o',"$out\particle-link-tests.exe")
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @arguments *> "$out\build.log"
if($LASTEXITCODE){Get-Content -LiteralPath "$out\build.log" -Tail 25;exit $LASTEXITCODE}
& "$out\particle-link-tests.exe"
exit $LASTEXITCODE

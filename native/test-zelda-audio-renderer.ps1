$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-zelda-audio-renderer.ps1')
$out=Join-Path $n 'build\zelda-audio-renderer';New-Item -ItemType Directory -Force -Path $out | Out-Null
$original=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioDSPInterfaceHost.cpp'))
$tables='#include <JSystem/JAudio2/JASDspInterface.hpp>'+"`n"+'#include <JSystem/JAudio2/JASWaveInfo.hpp>'+"`n"+'u32 JASWaveInfo::one=1;'+"`n"
foreach($anchor in @('u32 const ATTRIBUTE_ALIGN(32) JASDsp::DSPRES_FILTER[320] = {','u8 const ATTRIBUTE_ALIGN(32) JASDsp::DSPADPCM_FILTER[64] = {')){
 $start=$original.IndexOf($anchor);$end=$original.IndexOf('};',$start)
 if($start-lt 0-or $end-le $start){throw 'Original DSP coefficient table boundary changed'}
 $tables+=$original.Substring($start,$end+2-$start)+"`n"
}
foreach($signature in @('void JASDsp::TChannel::setWaveInfo(JASWaveInfo const& waveInfo, u32 param_1, u32 param_2) {','void JASDsp::TChannel::playStart() {')){
 $start=$original.IndexOf($signature)
 if($start-lt 0){throw 'Original voice setup method missing'}
 $end=$original.IndexOf('{',$start)+1;$depth=1
 while($end-lt $original.Length-and $depth){if($original[$end]-eq '{'){$depth++}elseif($original[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Original voice setup method boundary invalid'}
 $tables+=$original.Substring($start,$end-$start)+"`n"
}
$tablePath=Join-Path $out 'OriginalDspTables.cpp';[IO.File]::WriteAllText($tablePath,$tables,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $out 'renderer-tests.exe';$log=Join-Path $out 'build.txt'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -fwrapv -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$n\include-runtime" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\zelda_audio_renderer_tests.cpp" "$n\src\generated\ZeldaAudioRenderer.cpp" "$n\src\dsp_address_space.cpp" "$n\src\dsp_memory_view.cpp" $tablePath -lbcrypt -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 25;exit 1}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' (Join-Path $out 'retail-stereo.wav') *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-zelda-audio-renderer.ps1')
& (Join-Path $n 'generate-audio-dsp-host.ps1')
$out=Join-Path $n 'build\dsp-native-audio-bridge';New-Item -ItemType Directory -Force -Path $out | Out-Null
$original=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioDSPInterfaceHost.cpp'))
$fixture=@'
#include <Native/DspAddressSpace.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
#include <revolution/os.h>
u32 JASWaveInfo::one=1;
JASDsp::TChannel* JASDsp::CH_BUF;
JASDsp::FxBuf* JASDsp::FX_BUF;
'@
foreach($anchor in @('u32 const ATTRIBUTE_ALIGN(32) JASDsp::DSPRES_FILTER[320] = {','u8 const ATTRIBUTE_ALIGN(32) JASDsp::DSPADPCM_FILTER[64] = {','u16 ATTRIBUTE_ALIGN(32) JASDsp::SEND_TABLE[] = {')){
 $start=$original.IndexOf($anchor);$end=$original.IndexOf('};',$start)
 if($start-lt 0-or $end-le $start){throw 'Original DSP table boundary changed'}
 $fixture+="`n"+$original.Substring($start,$end+2-$start)+"`n"
}
foreach($signature in @('void JASDsp::TChannel::setWaveInfo(JASWaveInfo const& waveInfo, u32 param_1, u32 param_2) {','void JASDsp::TChannel::playStart() {','u16 DSP_CreateMap2(u32 param_0) {','bool JASDsp::TChannel::isActive() const {','void JASDsp::TChannel::setBusConnect(u8 param_0, u8 param_1) {','int JASDsp::setFXLine(u8 param_0, s16* buffer, JASDsp::FxlineConfig_* param_2) {','void JASDsp::setFilterTable(s16* param_0, s16* param_1, u32 param_2) {')){
 $start=$original.IndexOf($signature)
 if($start-lt 0){throw 'Original DSP method missing'}
 $end=$original.IndexOf('{',$start)+1;$depth=1
 while($end-lt $original.Length-and $depth){if($original[$end]-eq '{'){$depth++}elseif($original[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Original DSP method boundary invalid'}
 $fixture+=$original.Substring($start,$end-$start)+"`n"
}
if(!$fixture.Contains('Native::nativeDspAddressSpace().map(puVar3->_4')){throw 'Generated effects-ring registration missing'}
$fixturePath=Join-Path $out 'OriginalDspSetup.cpp';[IO.File]::WriteAllText($fixturePath,$fixture,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $out 'native-bridge-tests.exe';$log=Join-Path $out 'build.txt'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -fwrapv -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$n\include-runtime" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\dsp_native_audio_bridge_tests.cpp" "$n\src\dsp_native_audio_bridge.cpp" "$n\src\dsp_audio_frame_executor.cpp" "$n\src\dsp_audio_command_hle.cpp" "$n\src\generated\ZeldaAudioRenderer.cpp" "$n\src\dsp_address_space.cpp" "$n\src\dsp_memory_view.cpp" "$n\src\generated\DspUcodeImage.cpp" "$n\src\generated\AudioDSPCommandKernel.cpp" $fixturePath -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 30;exit 1}
& $exe *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code

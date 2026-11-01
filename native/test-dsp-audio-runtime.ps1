$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-native-audio-thread.ps1')
& (Join-Path $n 'test-dsp-audio-dispatch.ps1')
if($LASTEXITCODE){exit $LASTEXITCODE}
$out=Join-Path $n 'build\dsp-audio-runtime';New-Item -ItemType Directory -Force -Path $out | Out-Null
foreach($file in @('OriginalDspSetup.cpp','OriginalAudioCallback.cpp')){Copy-Item -LiteralPath (Join-Path (Join-Path $n 'build\dsp-audio-dispatch') $file) -Destination (Join-Path $out $file)}
# Compile the complete production transport bodies without pulling unrelated
# archive/decompression/heap routines into this focused audio test. The complete
# provider is compiled separately below and exercised by its existing ARAM suite.
$aram=[IO.File]::ReadAllText((Join-Path $n 'src\native_jkr_aram_port.cpp'))
$globals=[regex]::Match($aram,'namespace \{std::mutex transportMutex;[^\r\n]+\}')
if(!$globals.Success){throw 'Native ARAM transport globals boundary changed'}
$transport='#include "native_jkr_aram_port.hpp"'+"`n"+$globals.Value+"`n"
foreach($signature in @('bool nativeBindAramTransport(','void nativeReleaseAramTransport(','std::shared_ptr<NativeAramMemory> nativeAcquireAramTransport(','bool nativeReadBoundAramSamples(','bool nativeUploadAram(','bool nativeDownloadAram(')){
 $start=$aram.IndexOf($signature);if($start-lt 0){throw 'Native ARAM transport method missing'}
 $end=$aram.IndexOf('{',$start)+1;$depth=1
 while($end-lt $aram.Length-and $depth){if($aram[$end]-eq '{'){$depth++}elseif($aram[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Native ARAM transport method boundary changed'}
 $transport+=$aram.Substring($start,$end-$start)+"`n"
}
$transportPath=Join-Path $out 'OriginalAramTransport.cpp';[IO.File]::WriteAllText($transportPath,$transport,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $out 'runtime-tests.exe';$log=Join-Path $out 'build.txt'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$sources=@('dsp_audio_runtime_tests.cpp','dsp_audio_runtime.cpp','dsp_audio_dispatch.cpp','dsp_native_audio_bridge.cpp','dsp_audio_frame_executor.cpp','dsp_audio_command_hle.cpp','dsp_mailbox_port.cpp','dsp_boot_hle.cpp','dsp_rom_boot_hle.cpp','dsp_address_space.cpp','dsp_memory_view.cpp','generated\ZeldaAudioRenderer.cpp','generated\DspUcodeImage.cpp','generated\AudioDSPCommandKernel.cpp','generated\AudioDSPSendKernel.cpp','generated\AudioDSPTaskWorkKernel.cpp','generated\AudioDSPDeferredKernel.cpp','generated\AudioDSPCompletionKernel.cpp','os_critical_port.cpp','os_message_port.cpp') | ForEach-Object{Join-Path (Join-Path $n 'src') $_}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -fwrapv -UNDEBUG -ffunction-sections -fdata-sections '-Wl,--gc-sections' -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include\Native\HeapPlacement.hpp" @sources $transportPath (Join-Path $out 'OriginalDspSetup.cpp') (Join-Path $out 'OriginalAudioCallback.cpp') -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 35;exit 1}
& $exe *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');if($code){exit $code}
# Compile the complete generated thread class as well as the callback fixture.
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include math_types.hpp -c "$n\src\generated\AudioThreadKernel.cpp" -o (Join-Path $out 'AudioThreadKernel.obj') *> (Join-Path $out 'thread-build.txt')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'thread-build.txt') -Tail 25;exit $LASTEXITCODE}
'PASS complete generated native audio thread compiles with activation, service-before-wait, stop and preserved startup handshake'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include math_types.hpp -include "$n\include\Native\HeapPlacement.hpp" -c "$n\src\native_jkr_aram_port.cpp" -o (Join-Path $out 'native_jkr_aram_port.obj') *> (Join-Path $out 'aram-provider-build.txt')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'aram-provider-build.txt') -Tail 25;exit $LASTEXITCODE}
'PASS complete native ARAM transport/archive provider compiles'

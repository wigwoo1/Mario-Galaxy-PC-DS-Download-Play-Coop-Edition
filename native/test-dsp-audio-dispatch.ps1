$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-audio-frame.ps1')
& (Join-Path $n 'generate-audio-dsp-completion.ps1')
# Regenerates and verifies the original native JAS method/table fixture.
& (Join-Path $n 'test-dsp-native-audio-bridge.ps1')
if($LASTEXITCODE){exit $LASTEXITCODE}
$out=Join-Path $n 'build\dsp-audio-dispatch';New-Item -ItemType Directory -Force -Path $out | Out-Null
$fixture=Join-Path $out 'OriginalDspSetup.cpp'
Copy-Item -LiteralPath (Join-Path $n 'build\dsp-native-audio-bridge\OriginalDspSetup.cpp') -Destination $fixture
$original=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASAudioThread.cpp'))
$start=$original.IndexOf('void JASAudioThread::DSPCallback(void*)');if($start-lt 0){throw 'Original audio callback missing'}
$end=$original.IndexOf('{',$start)+1;$depth=1
while($end-lt $original.Length-and $depth){if($original[$end]-eq '{'){$depth++}elseif($original[$end]-eq '}'){$depth--};$end++}
if($depth){throw 'Original audio callback boundary changed'}
$callback='#include "dsp_audio_dispatch_test_callback.hpp"'+"`n"+'#include <revolution/dsp.h>'+"`n"+'#include <JSystem/JAudio2/JASDspInterface.hpp>'+"`n"+$original.Substring($start,$end-$start).Replace('JASAudioThread','TestAudioThread')
$callbackPath=Join-Path $out 'OriginalAudioCallback.cpp';[IO.File]::WriteAllText($callbackPath,$callback,[Text.UTF8Encoding]::new($false))
$exe=Join-Path $out 'dispatch-tests.exe';$log=Join-Path $out 'build.txt'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$sources=@('dsp_audio_dispatch_tests.cpp','dsp_audio_dispatch.cpp','dsp_native_audio_bridge.cpp','dsp_audio_frame_executor.cpp','dsp_audio_command_hle.cpp','dsp_mailbox_port.cpp','dsp_boot_hle.cpp','dsp_rom_boot_hle.cpp','dsp_address_space.cpp','dsp_memory_view.cpp','generated\ZeldaAudioRenderer.cpp','generated\DspUcodeImage.cpp','generated\AudioDSPCommandKernel.cpp','generated\AudioDSPSendKernel.cpp','generated\AudioDSPTaskWorkKernel.cpp','generated\AudioDSPDeferredKernel.cpp','generated\AudioDSPCompletionKernel.cpp','os_critical_port.cpp','os_message_port.cpp') | ForEach-Object{Join-Path (Join-Path $n 'src') $_}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -fwrapv -UNDEBUG -ffunction-sections -fdata-sections '-Wl,--gc-sections' -target x86_64-windows-gnu "-I$n\include" "-I$n\include-runtime" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h @sources $fixture $callbackPath -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 25;exit 1}
& $exe *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code

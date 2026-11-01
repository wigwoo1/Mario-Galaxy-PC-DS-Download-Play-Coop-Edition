$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-audio-frame.ps1')
& (Join-Path $n 'generate-audio-dsp-host.ps1')
& (Join-Path $n 'generate-zelda-audio-renderer.ps1')
$out=Join-Path $n 'build\jas-channel-lifecycle';New-Item -ItemType Directory -Force -Path $out | Out-Null
# Reuse the already verified exact original callback body; regenerate if absent.
$callback=Join-Path $n 'build\dsp-audio-dispatch\OriginalAudioCallback.cpp'
if(!(Test-Path -LiteralPath $callback)){& (Join-Path $n 'test-dsp-audio-dispatch.ps1');if($LASTEXITCODE){exit $LASTEXITCODE}}
$original=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASAudioThread.cpp'))
$start=$original.IndexOf('void JASAudioThread::DSPCallback(void*)');$end=$original.IndexOf('{',$start)+1;$depth=1
while($end-lt $original.Length-and $depth){if($original[$end]-eq '{'){$depth++}elseif($original[$end]-eq '}'){$depth--};$end++}
if($start-lt 0-or $depth){throw 'Original callback boundary changed'}
$body=$original.Substring($start,$end-$start).Replace('JASAudioThread','TestAudioThread')
if(![IO.File]::ReadAllText($callback).Contains($body)){throw 'Original callback fixture is stale'}
$callbackPath=Join-Path $out 'OriginalAudioCallback.cpp';Copy-Item -LiteralPath $callback -Destination $callbackPath
$names=@('jas_channel_lifecycle_tests.cpp','dsp_audio_runtime.cpp','dsp_audio_dispatch.cpp','dsp_native_audio_bridge.cpp','dsp_audio_frame_executor.cpp','dsp_audio_command_hle.cpp','dsp_mailbox_port.cpp','dsp_boot_hle.cpp','dsp_rom_boot_hle.cpp','dsp_address_space.cpp','dsp_memory_view.cpp','generated\ZeldaAudioRenderer.cpp','generated\DspUcodeImage.cpp','generated\AudioDSPCommandKernel.cpp','generated\AudioDSPSendKernel.cpp','generated\AudioDSPTaskWorkKernel.cpp','generated\AudioDSPDeferredKernel.cpp','generated\AudioDSPInterfaceHost.cpp','generated\AudioDSPChannelKernel.cpp','generated\AudioCalcKernel.cpp','os_critical_port.cpp','os_zero_memory_port.cpp','generated\AllocatorDispatchKernel.cpp','generated\LayoutDefaultAllocatorKernel.cpp','generated\JKRUnitHeap.cpp','generated\JKRSolidHeap.cpp','generated\JKRExpHeap.cpp','generated\JKRHeapDispatch.cpp','generated\JKRDisposer.cpp','os_mutex_port.cpp','native_thread_identity_port.cpp','os_thread_port.cpp','os_message_port.cpp','generated\AsyncJobKernel.cpp','generated\CurrentHeapKernel.cpp','generated\HeapMemoryWatcher.cpp','input_workspace_port.cpp','os_panic_port.cpp','heap_error_port.cpp','heap_arena_port.cpp','native_jkr_aram_port.cpp','aram_read_port.cpp','sdk_report_port.cpp','aram_manager_kernel.cpp','generated\AramQueueKernel.cpp','generated\JKRAramHeap.cpp','generated\JKRAramBlock.cpp','generated\AramCompletionKernel.cpp','generated\AramCommandKernel.cpp','generated\JKRDecomp.cpp','generated\JKRThread.cpp','jut_console_port.cpp','sdk_cache_port.cpp','gpu_resources.cpp')
$sources=@($names | ForEach-Object{Join-Path (Join-Path $n 'src') $_})+@(Join-Path $p 'Petari\src\JSystem\JSupport\JSUList.cpp')
$backend=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioHeapKernel.cpp'))
$begin=$backend.IndexOf('JASGenericMemPool::JASGenericMemPool()');$end=$backend.IndexOf('u32 JASKernel::sAramBase;',$begin)
if($begin-lt 0-or $end-lt $begin){throw 'Original generic pool boundaries changed'}
$pool=Join-Path $out 'OriginalGenericPool.cpp'
[IO.File]::WriteAllText($pool,'#include <Native/HeapPlacement.hpp>'+"`n"+'#include <JSystem/JAudio2/JASHeapCtrl.hpp>'+"`n"+'#include <JSystem/JKernel/JKRSolidHeap.hpp>'+"`n"+$backend.Substring($begin,$end-$begin),[Text.UTF8Encoding]::new($false))
# Isolate the actual rate getter/default; no substitute driver timing function.
$ai=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioAiCtrlHost.cpp'))
$rate='f32 JASDriver::getDacRate() {'+"`n"+'    return sDacRate;'+"`n"+'}'
if(!$ai.Replace("`r`n","`n").Contains($rate)-or !$ai.Contains('f32 JASDriver::sDacRate = 32028.5f;')){throw 'Original DAC rate anchors changed'}
$rateSource=Join-Path $out 'OriginalDacRate.cpp'
[IO.File]::WriteAllText($rateSource,'#include <JSystem/JAudio2/JASAiCtrl.hpp>'+"`n"+'f32 JASDriver::sDacRate = 32028.5f;'+"`n"+$rate,[Text.UTF8Encoding]::new($false))
$counter='u32 JASDriver::getSubFrameCounter() {'+"`n"+'    return sSubFrameCounter;'+"`n"+'}'
if(!$ai.Replace("`r`n","`n").Contains($counter)-or !$ai.Contains('volatile u32 JASDriver::sSubFrameCounter;')){throw 'Original subframe counter anchors changed'}
[IO.File]::AppendAllText($rateSource,"`n"+'volatile u32 JASDriver::sSubFrameCounter;'+"`n"+$counter,[Text.UTF8Encoding]::new($false))
$sources+=@($pool,$rateSource)
$sources+=@('JASChannel.cpp','JASLfo.cpp','JASOscillator.cpp','JASSoundParams.cpp','JASDriverIF.cpp','JASCallback.cpp')|ForEach-Object{Join-Path $p ('Petari\src\JSystem\JAudio2\'+$_)}
$sources+=Join-Path $p 'Petari\src\JSystem\JMath\JMATrigonometric.cpp'
$evidence=@($sources)+@($callbackPath,(Join-Path $n 'include\JSystem\JAudio2\JASChannel.hpp'),(Join-Path $n 'include\JSystem\JAudio2\JASHeapCtrl.hpp'),(Join-Path $n 'test-jas-channel-lifecycle.ps1'))
$evidence|ForEach-Object{[pscustomobject]@{Path=$_;SHA256=(Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash}}|ConvertTo-Json -Depth 4|Set-Content -LiteralPath (Join-Path $out 'sources.json')
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache';$exe=Join-Path $out 'channel-lifecycle-tests.exe';$log=Join-Path $out 'build.txt'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -fwrapv -UNDEBUG -ffunction-sections -fdata-sections '-Wl,--gc-sections' -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include\Native\HeapPlacement.hpp" @sources $callbackPath -lbcrypt -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 40;exit 1}
& $exe *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code

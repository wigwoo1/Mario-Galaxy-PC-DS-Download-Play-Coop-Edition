$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-native-jas-aram-stream.ps1')
& (Join-Path $n 'generate-native-stream-object.ps1')
& (Join-Path $n 'generate-native-stream-manager.ps1')
& (Join-Path $n 'generate-native-sound-id-header.ps1')
& (Join-Path $n 'generate-audio-frame.ps1')
& (Join-Path $n 'generate-audio-dsp-host.ps1')
& (Join-Path $n 'generate-native-audio-thread.ps1')
& (Join-Path $n 'generate-zelda-audio-renderer.ps1')
$out=Join-Path $n 'build\audio-thread-stream-pause';New-Item -ItemType Directory -Force -Path $out | Out-Null
$providers=@(
 'native\src\aram_manager_kernel.cpp',
 'native\src\aram_read_port.cpp',
 'native\src\audio_report_port.cpp',
 'native\src\dsp_address_space.cpp',
 'native\src\dsp_audio_command_hle.cpp',
 'native\src\dsp_audio_dispatch.cpp',
 'native\src\dsp_audio_frame_executor.cpp',
 'native\src\dsp_audio_runtime.cpp',
 'native\src\dsp_boot_hle.cpp',
 'native\src\dsp_mailbox_port.cpp',
 'native\src\dsp_memory_view.cpp',
 'native\src\dsp_native_audio_bridge.cpp',
 'native\src\dsp_rom_boot_hle.cpp',
 'native\src\generated\AllocatorDispatchKernel.cpp',
 'native\src\generated\AramCommandKernel.cpp',
 'native\src\generated\AramCompletionKernel.cpp',
 'native\src\generated\AramQueueKernel.cpp',
 'native\src\generated\AsyncJobKernel.cpp',
 'native\src\generated\AudioCalcKernel.cpp',
 'native\src\generated\AudioDSPChannelKernel.cpp',
 'native\src\generated\AudioDSPCommandKernel.cpp',
 'native\src\generated\AudioDSPDeferredKernel.cpp',
 'native\src\generated\AudioDSPInterfaceHost.cpp',
 'native\src\generated\AudioDSPSendKernel.cpp',
 'native\src\generated\AudioDSPTaskWorkKernel.cpp',
 'native\src\generated\CurrentHeapKernel.cpp',
 'native\src\generated\DspUcodeImage.cpp',
 'native\src\generated\HeapMemoryWatcher.cpp',
 'native\src\generated\JKRAramBlock.cpp',
 'native\src\generated\JKRAramHeap.cpp',
 'native\src\generated\JKRDecomp.cpp',
 'native\src\generated\JKRDisposer.cpp',
 'native\src\generated\JKRExpHeap.cpp',
 'native\src\generated\JKRHeapDispatch.cpp',
 'native\src\generated\JKRSolidHeap.cpp',
 'native\src\generated\JKRThread.cpp',
 'native\src\generated\JKRUnitHeap.cpp',
 'native\src\generated\LayoutDefaultAllocatorKernel.cpp',
 'native\src\generated\ZeldaAudioRenderer.cpp',
 'native\src\gpu_resources.cpp',
 'native\src\heap_arena_port.cpp',
 'native\src\heap_error_port.cpp',
 'native\src\input_workspace_port.cpp',
 'native\src\jut_console_port.cpp',
 'native\src\native_jkr_aram_port.cpp',
 'native\src\native_thread_identity_port.cpp',
 'native\src\os_critical_port.cpp',
 'native\src\os_message_port.cpp',
 'native\src\os_mutex_port.cpp',
 'native\src\os_panic_port.cpp',
 'native\src\os_thread_port.cpp',
 'native\src\os_zero_memory_port.cpp',
 'native\src\sdk_cache_port.cpp',
 'native\src\sdk_report_port.cpp',
 'Petari\src\JSystem\JAudio2\JASCallback.cpp',
 'Petari\src\JSystem\JAudio2\JASChannel.cpp',
 'Petari\src\JSystem\JAudio2\JASDriverIF.cpp',
 'Petari\src\JSystem\JAudio2\JASLfo.cpp',
 'Petari\src\JSystem\JAudio2\JASOscillator.cpp',
 'Petari\src\JSystem\JAudio2\JASSoundParams.cpp',
 'Petari\src\JSystem\JMath\JMATrigonometric.cpp',
 'Petari\src\JSystem\JSupport\JSUList.cpp'
)
$sources=@($providers | ForEach-Object {Join-Path $p $_})+@(
 (Join-Path $n 'src\audio_thread_stream_pause_tests.cpp'),
 (Join-Path $n 'src\native_audio_ai.cpp'),
 (Join-Path $n 'src\generated\AudioAiCtrlHost.cpp'),
 (Join-Path $n 'src\generated\AudioThreadKernel.cpp'),
 (Join-Path $n 'src\generated\AudioHeapKernel.cpp'),
 (Join-Path $n 'src\generated\AudioProbeKernel.cpp'),
 (Join-Path $n 'src\os_time_port.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JAudio2\JASCmdStack.cpp'))
$sources+=@('generated\AudioTaskThreadKernel.cpp','generated\AudioDvdThreadKernel.cpp','generated\JASAramStream.cpp','native_dvd_port.cpp','generated\JAIStream.cpp','generated\JAIStreamMgr.cpp','generated\JAUStreamFileTable.cpp')|ForEach-Object{Join-Path (Join-Path $n 'src') $_}
$sources+=@('JAIStreamDataMgr.cpp','JAISound.cpp','JAISoundParams.cpp','JAISoundChild.cpp')|ForEach-Object{Join-Path $p ('Petari\src\JSystem\JAudio2\'+$_)}
# Full sound providers reference these exact original sequence-track helpers;
# sequence-track execution is outside this retail-stream test's scope.
$track=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASTrack.cpp'))
$trackText='#include <JSystem/JAudio2/JASTrack.hpp>'+"`n"
foreach($signature in @('void JASTrack::setChannelMgrCount(u32 count)','void JASTrack::assignExtBuffer(u32 mgr, JASSoundParams* buffer)','JASTrack::TChannelMgr::TChannelMgr(JASTrack* track)','void JASTrack::TChannelMgr::init()')){
 $begin=$track.IndexOf($signature);if($begin-lt 0){throw 'Original track helper missing'};$end=$track.IndexOf('{',$begin)+1;$depth=1
 while($depth-and $end-lt $track.Length){if($track[$end]-eq '{'){$depth++}elseif($track[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Original track helper unbalanced'};$trackText+=$track.Substring($begin,$end-$begin)+"`n"
}
$trackPath=Join-Path $out 'OriginalTrackHelpers.cpp';[IO.File]::WriteAllText($trackPath,$trackText,[Text.UTF8Encoding]::new($false));$sources+=$trackPath
$evidence=$sources+@((Join-Path $n 'generate-native-audio-thread.ps1'),(Join-Path $n 'generate-audio-dsp-host.ps1'),$PSCommandPath)
$evidence|ForEach-Object{[pscustomobject]@{Path=$_;SHA256=(Get-FileHash -LiteralPath $_).Hash}}|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $out 'sources.json')
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache';$exe=Join-Path $out 'audio-thread-stream-pause-tests.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -fwrapv -UNDEBUG -DTRUE=1 -DFALSE=0 -ffunction-sections -fdata-sections '-Wl,--gc-sections' -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include\Native\HeapPlacement.hpp" @sources -lbcrypt -lwinmm -o $exe *> (Join-Path $out 'build.txt')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.txt') -Tail 35;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code
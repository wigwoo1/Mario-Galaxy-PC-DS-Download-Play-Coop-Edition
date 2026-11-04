param([string]$CacheDirectory)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-native-track.ps1')
& (Join-Path $n 'generate-sequence-reader.ps1')
& (Join-Path $n 'generate-sequence-parser.ps1')
& (Join-Path $n 'generate-audio-frame.ps1')
& (Join-Path $n 'generate-audio-dsp-host.ps1')
& (Join-Path $n 'generate-native-audio-thread.ps1')
& (Join-Path $n 'generate-zelda-audio-renderer.ps1')
$out=Join-Path $n 'build\retail-sound-router';New-Item -ItemType Directory -Force -Path $out | Out-Null
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
 (Join-Path $n 'src\retail_sound_router_tests.cpp'),
 (Join-Path $n 'src\native_audio_ai.cpp'),
 (Join-Path $n 'src\generated\AudioAiCtrlHost.cpp'),
 (Join-Path $n 'src\generated\AudioThreadKernel.cpp'),
 (Join-Path $n 'src\generated\AudioHeapKernel.cpp'),
 (Join-Path $n 'src\generated\AudioProbeKernel.cpp'),
 (Join-Path $n 'src\os_time_port.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JAudio2\JASCmdStack.cpp'))
$sources+=@(
 (Join-Path $n 'src\generated\JASTrack.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JAudio2\JASSeqCtrl.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JAudio2\JASRegisterParam.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JAudio2\JASTrackPort.cpp'),
 (Join-Path $p 'Petari\src\JSystem\JGadget\linklist.cpp'),
 (Join-Path $n 'src\generated\SequenceReaderKernel.cpp'),
 (Join-Path $n 'src\generated\SequenceParserKernel.cpp'),
 (Join-Path $n 'src\generated\AudioCalcKernel.cpp'),(Join-Path $p 'Petari\src\JSystem\JMath\random.cpp'),
 (Join-Path $n 'src\generated\JASBank.cpp'),
 (Join-Path $n 'src\generated\GameInstrumentParamKernel.cpp'))
$sources+=@(
'native\src\generated\AudioArcInterpreterKernel.cpp','native\src\generated\BankInstrumentKernel.cpp','native\src\generated\BankEffectorKernel.cpp','native\src\generated\BankDispatchKernel.cpp','native\src\generated\BasicBankKernel.cpp','native\src\generated\BasicInstrumentKernel.cpp','native\src\generated\DrumSetKernel.cpp','native\src\bank_chunk_port.cpp','native\src\generated\WaveSystemParserKernel.cpp','native\src\generated\JASSimpleWaveBankKernel.cpp','native\src\generated\JASBasicWaveBankKernel.cpp','native\src\generated\JASWaveArcLoaderKernel.cpp','native\src\generated\AudioTaskThreadKernel.cpp','native\src\generated\AudioDvdThreadKernel.cpp','native\src\native_dvd_port.cpp','native\src\generated\JKRDvdFile.cpp','native\src\generated\AramStreamKernel.cpp','native\src\generated\DvdAramRipperEntryKernel.cpp','native\src\generated\DvdAramRipperTransferKernel.cpp','native\src\generated\AramCompletionKernel.cpp','native\src\jsu_stream_lifecycle_port.cpp','Petari\src\JSystem\JAudio2\JASInstRand.cpp','Petari\src\JSystem\JAudio2\JASInstSense.cpp','Petari\src\JSystem\JSupport\JSUFileStream.cpp','Petari\src\JSystem\JSupport\JSUInputStream.cpp')|ForEach-Object{Join-Path $p $_}
$sources+=@((Join-Path $n 'src\vi_port.cpp'),(Join-Path $p 'Petari\src\JSystem\JSupport\JSUOutputStream.cpp'))
$sources+=@('Petari\src\JSystem\JAudio2\JAISe.cpp','Petari\src\JSystem\JAudio2\JAISeMgr.cpp','Petari\src\JSystem\JAudio2\JAISound.cpp','Petari\src\JSystem\JAudio2\JAISoundParams.cpp','Petari\src\JSystem\JAudio2\JAISoundInfo.cpp','Petari\src\JSystem\JAudio2\JAISoundChild.cpp','Petari\src\JSystem\JAudio2\JAISeqDataMgr.cpp','native\src\generated\JAUSeqCollection.cpp','native\src\generated\JAUStdSoundInfo.cpp','native\src\generated\SoundTableKernel.cpp','native\src\generated\JAIStream.cpp','native\src\generated\JAIStreamMgr.cpp','native\src\generated\JASAramStream.cpp','native\src\generated\JAUStreamFileTable.cpp','Petari\src\JSystem\JAudio2\JAIStreamDataMgr.cpp')|ForEach-Object{Join-Path $p $_}
$sources+=@('Petari\src\JSystem\JAudio2\JAUSoundMgr.cpp','Petari\src\JSystem\JAudio2\JAISoundStarter.cpp','Petari\src\JSystem\JAudio2\JAISeq.cpp','Petari\src\JSystem\JAudio2\JAISeqMgr.cpp')|ForEach-Object{Join-Path $p $_}
$evidence=$sources+@((Join-Path $p 'native\include\JSystem\JAudio2\JAUSoundTable.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAISe.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAISeMgr.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAISeqDataMgr.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAUSoundInfo.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAUSeqCollection.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JAISound.hpp'),(Join-Path $p 'native\generate-native-std-sound-info.ps1'),(Join-Path $p 'native\src\assets.hpp'),(Join-Path $p 'native\src\wbfs.hpp'),(Join-Path $p 'native\src\crypto.hpp'),(Join-Path $p 'native\include\JSystem\JAudio2\JASWSParser.hpp'),(Join-Path $p 'native\include\JSystem\JAudio2\JASBNKParser.hpp'),(Join-Path $p 'native\include\JSystem\JAudio2\JASBasicInst.hpp'),(Join-Path $p 'native\include\JSystem\JAudio2\JASDSPInterface.hpp'),(Join-Path $p 'native\include\JSystem\JAudio2\JASWaveArcLoader.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASBasicWaveBank.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASSimpleWaveBank.hpp'),(Join-Path $p 'Petari\src\Game\AudioLib\AudUtil.cpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JASBank.cpp'),(Join-Path $n 'include\JSystem\JAudio2\JASBank.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASBank.hpp'),(Join-Path $p 'Petari\src\Game\AudioLib\OverwriteJAudio.cpp'),(Join-Path $n 'generate-sequence-reader.ps1'),(Join-Path $n 'generate-sequence-parser.ps1'),(Join-Path $n 'include\JSystem\JAudio2\JASSeqReader.hpp'),(Join-Path $n 'include\Native\SequenceReport.hpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JASSeqParser.cpp'),(Join-Path $n 'generate-native-track.ps1'),(Join-Path $n 'include\JSystem\JAudio2\JASTrack.hpp'),(Join-Path $n 'include\JSystem\JGadget\linklist.hpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JASTrack.cpp'),(Join-Path $n 'generate-native-audio-thread.ps1'),(Join-Path $n 'generate-audio-dsp-host.ps1'),(Join-Path $n 'include\Native\AudioPoolHeapLifetime.hpp'),(Join-Path $n 'include\Native\DspAudioRuntime.hpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JASDSPInterface.cpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JASAiCtrl.cpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASTrack.hpp'),(Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASOscillator.hpp'),$PSCommandPath)
$evidence|ForEach-Object{[pscustomobject]@{Path=$_;SHA256=(Get-FileHash -LiteralPath $_).Hash}}|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $out 'sources.json')
$env:ZIG_GLOBAL_CACHE_DIR=if($CacheDirectory){$CacheDirectory}else{Join-Path $p 'tools\zig-cache'};$exe=Join-Path $out 'retail-sound-router-tests.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -g -O2 -fwrapv -UNDEBUG -DTRUE=1 -DFALSE=0 -ffunction-sections -fdata-sections '-Wl,--gc-sections' '-Wl,--export-all-symbols' -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include\Native\HeapPlacement.hpp" @sources -lbcrypt -lwinmm -ldbghelp -o $exe *> (Join-Path $out 'build.txt')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.txt') -Tail 35;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code

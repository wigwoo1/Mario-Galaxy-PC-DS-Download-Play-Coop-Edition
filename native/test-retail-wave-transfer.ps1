$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include cmath "$n\src\retail_wave_transfer_tests.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\generated\LayoutDefaultAllocatorKernel.cpp" "$n\src\generated\JKRUnitHeap.cpp" "$n\src\generated\JKRSolidHeap.cpp" "$n\src\generated\JKRExpHeap.cpp" "$n\src\generated\JKRHeapDispatch.cpp" "$n\src\generated\JKRDisposer.cpp" "$p\Petari\src\JSystem\JSupport\JSUList.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_message_port.cpp" "$n\src\generated\AsyncJobKernel.cpp" "$n\src\generated\CurrentHeapKernel.cpp" "$n\src\generated\HeapMemoryWatcher.cpp" "$n\src\input_workspace_port.cpp" "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\heap_arena_port.cpp" "$n\src\generated\BasicInstrumentKernel.cpp" "$n\src\generated\AudioCalcKernel.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gpu_resources.cpp" -lbcrypt "$n\src\os_zero_memory_port.cpp" "$n\src\generated\AudioArcInterpreterKernel.cpp" "$n\src\bank_chunk_port.cpp" "$n\src\generated\BankInstrumentKernel.cpp" "$n\src\generated\BankEffectorKernel.cpp" "$n\src\generated\BasicBankKernel.cpp" "$n\src\generated\DrumSetKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstRand.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstSense.cpp" "$p\Petari\src\JSystem\JMath\random.cpp" "$n\src\generated\GameInstrumentParamKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASSoundParams.cpp" "$p\Petari\src\JSystem\JAudio2\JASOscillator.cpp" "$n\src\generated\WaveSystemParserKernel.cpp" "$n\src\generated\JASSimpleWaveBankKernel.cpp" "$n\src\generated\JASBasicWaveBankKernel.cpp" "$n\src\generated\JASWaveArcLoaderKernel.cpp" "$n\src\generated\AudioHeapKernel.cpp" "$n\src\generated\AudioDSPStateKernel.cpp" "$n\src\generated\AudioDvdThreadKernel.cpp" "$n\src\generated\AudioTaskThreadKernel.cpp" "$n\src\native_dvd_port.cpp" "$n\src\native_jkr_aram_port.cpp" "$n\src\sdk_report_port.cpp" "$n\src\generated\JKRThread.cpp" "$n\src\os_critical_port.cpp" "$n\src\generated\AramCompletionKernel.cpp" "$n\src\generated\JKRDecomp.cpp" "$n\src\generated\DvdAramRipperEntryKernel.cpp" "$n\src\generated\JKRDvdFile.cpp" "$n\src\generated\DvdAramRipperTransferKernel.cpp" "$n\src\generated\AramStreamKernel.cpp" "$p\Petari\src\JSystem\JSupport\JSUFileStream.cpp" "$n\src\vi_port.cpp" "$n\src\aram_manager_kernel.cpp" "$n\src\generated\JKRAramHeap.cpp" "$n\src\generated\JKRAramBlock.cpp" "$p\Petari\src\JSystem\JSupport\JSUInputStream.cpp" "$n\src\jsu_stream_lifecycle_port.cpp" -o "$n\build\retail-wave-transfer-tests.exe" *> "$n\build\retail-wave-transfer-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\retail-wave-transfer-tests.log" -Tail 15;exit 1}
& "$n\build\retail-wave-transfer-tests.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
exit $LASTEXITCODE




































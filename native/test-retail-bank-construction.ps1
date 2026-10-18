$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include cmath "$n\src\retail_bank_construction_tests.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\generated\LayoutDefaultAllocatorKernel.cpp" "$n\src\generated\JKRUnitHeap.cpp" "$n\src\generated\JKRSolidHeap.cpp" "$n\src\generated\JKRExpHeap.cpp" "$n\src\generated\JKRHeapDispatch.cpp" "$n\src\generated\JKRDisposer.cpp" "$p\Petari\src\JSystem\JSupport\JSUList.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_message_port.cpp" "$n\src\generated\AsyncJobKernel.cpp" "$n\src\generated\CurrentHeapKernel.cpp" "$n\src\generated\HeapMemoryWatcher.cpp" "$n\src\input_workspace_port.cpp" "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\heap_arena_port.cpp" "$n\src\generated\BasicInstrumentKernel.cpp" "$n\src\generated\AudioCalcKernel.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gpu_resources.cpp" -lbcrypt "$n\src\os_zero_memory_port.cpp" "$n\src\generated\AudioArcInterpreterKernel.cpp" "$n\src\bank_chunk_port.cpp" "$n\src\generated\BankInstrumentKernel.cpp" "$n\src\generated\BankEffectorKernel.cpp" "$n\src\generated\BasicBankKernel.cpp" "$n\src\generated\DrumSetKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstRand.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstSense.cpp" "$p\Petari\src\JSystem\JMath\random.cpp" "$n\src\generated\GameInstrumentParamKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASSoundParams.cpp" "$p\Petari\src\JSystem\JAudio2\JASOscillator.cpp" -o "$n\build\retail-bank-construction-tests.exe" *> "$n\build\retail-bank-construction-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\retail-bank-construction-tests.log" -Tail 15;exit 1}
& "$n\build\retail-bank-construction-tests.exe" "$n\build\retail-audio.baa"
exit $LASTEXITCODE
























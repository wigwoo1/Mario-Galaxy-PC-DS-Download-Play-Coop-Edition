$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $env:TEMP 'galaxy-unit-heap-cache'
$exe=Join-Path $env:TEMP 'galaxy-unit-heap-live-tests.exe';$log=Join-Path $env:TEMP 'galaxy-unit-heap-live-tests.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\unit_heap_live_tests.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\generated\LayoutDefaultAllocatorKernel.cpp" "$n\src\generated\JKRUnitHeap.cpp" "$n\src\generated\JKRSolidHeap.cpp" "$n\src\generated\JKRExpHeap.cpp" "$n\src\generated\JKRHeapDispatch.cpp" "$n\src\generated\JKRDisposer.cpp" "$p\Petari\src\JSystem\JSupport\JSUList.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_message_port.cpp" "$n\src\generated\AsyncJobKernel.cpp" "$n\src\generated\CurrentHeapKernel.cpp" "$n\src\generated\HeapMemoryWatcher.cpp" "$n\src\input_workspace_port.cpp" "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\heap_arena_port.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content $log -Tail 15;exit 1}
& $exe
exit $LASTEXITCODE















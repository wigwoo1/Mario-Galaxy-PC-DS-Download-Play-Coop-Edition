$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$build=Join-Path $n 'build'
$cache=if($env:NATIVE_ZIG_CACHE_DIR){$env:NATIVE_ZIG_CACHE_DIR}else{Join-Path $build 'zig-cache'}
$env:ZIG_GLOBAL_CACHE_DIR=$cache
$log=Join-Path $env:TEMP 'galaxy-dsp-mailbox-tests.log';$exe=Join-Path $env:TEMP 'galaxy-dsp-mailbox-tests.exe'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\dsp_mailbox_tests.cpp" "$n\src\dsp_mailbox_port.cpp" "$n\src\dsp_address_space.cpp" "$n\src\dsp_rom_boot_hle.cpp" "$n\src\generated\DspUcodeImage.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content $log -Tail 25;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE

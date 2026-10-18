$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$tmp=Join-Path $env:TEMP 'galaxy-dsp-audio-command-hle-integration'
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $tmp 'zig-cache'
$log=Join-Path $tmp 'build.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\dsp_audio_command_hle_integration_tests.cpp" "$n\src\dsp_audio_command_hle.cpp" "$n\src\dsp_mailbox_port.cpp" "$n\src\dsp_address_space.cpp" "$n\src\dsp_rom_boot_hle.cpp" "$n\src\generated\DspUcodeImage.cpp" "$n\src\generated\AudioDSPCommandKernel.cpp" "$n\src\generated\AudioDSPSendKernel.cpp" "$n\src\generated\AudioDSPDeferredKernel.cpp" "$n\src\generated\AudioDSPTaskWorkKernel.cpp" -o (Join-Path $tmp 'dsp-audio-command-hle-integration-tests.exe') *> $log
if($LASTEXITCODE){Get-Content $log -Tail 40;exit $LASTEXITCODE}
& (Join-Path $tmp 'dsp-audio-command-hle-integration-tests.exe')
exit $LASTEXITCODE

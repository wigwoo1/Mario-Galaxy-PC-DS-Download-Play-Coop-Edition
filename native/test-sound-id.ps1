$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
& (Join-Path $n 'generate-native-sound-id-header.ps1')
& (Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -DGALAXY_NATIVE_JAI_SOUND_ID=1 -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$root\Petari\include" "-I$root\Petari\libs\JSystem\include" "-I$root\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\sound_id_tests.cpp" -o "$n\build\sound-id-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\sound-id-tests.exe"
exit $LASTEXITCODE
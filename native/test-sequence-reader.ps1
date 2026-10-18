$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-sequence-reader.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\sequence_reader_tests.cpp" "$n\src\generated\SequenceReaderKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASTrackPort.cpp" -o "$n\build\sequence-reader-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\sequence-reader-tests.exe"
exit $LASTEXITCODE

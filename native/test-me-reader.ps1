$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\me_reader_tests.cpp" "$p\Petari\src\Game\RhythmLib\AudMeSeqReader.cpp" -o "$n\build\me-reader-tests.exe" *> "$n\build\me-reader-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\me-reader-tests.log" -Tail 15;exit 1}
& "$n\build\me-reader-tests.exe"
exit $LASTEXITCODE



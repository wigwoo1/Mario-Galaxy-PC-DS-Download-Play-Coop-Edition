$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-native-star-piece-storage.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\star_piece_storage_tests.cpp" "$n\src\generated\StarPieceAlmsStorage.cpp" -o "$n\build\star-piece-storage-tests.exe" *> "$n\build\star-piece-storage-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\star-piece-storage-tests.log" -Tail 15;exit 1}
& "$n\build\star-piece-storage-tests.exe"
exit $LASTEXITCODE


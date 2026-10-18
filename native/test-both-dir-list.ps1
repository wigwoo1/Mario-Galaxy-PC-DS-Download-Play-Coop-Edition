$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-both-dir-list.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\both_dir_list_tests.cpp" "$n\src\generated\BothDirListKernel.cpp" -o "$n\build\both-dir-list-tests.exe" *> "$n\build\both-dir-list-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\both-dir-list-tests.log" -Tail 20;exit 1}
& "$n\build\both-dir-list-tests.exe"
exit $LASTEXITCODE

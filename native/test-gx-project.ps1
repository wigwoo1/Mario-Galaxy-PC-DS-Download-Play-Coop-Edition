$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-gx-project.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -include revolution/types.h -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\gx_project_tests.cpp" "$n\src\generated\GXProjectKernel.cpp" -o "$n\build\gx-project-tests.exe" *> "$n\build\gx-project-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gx-project-tests.log" -Tail 15;exit 1}
& "$n\build\gx-project-tests.exe"
exit $LASTEXITCODE


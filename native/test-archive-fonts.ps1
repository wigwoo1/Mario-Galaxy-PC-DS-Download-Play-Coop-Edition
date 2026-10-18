$ErrorActionPreference='Stop'
$project='D:\Desktop\Games\Mario galaxy'
$native=Join-Path $project 'native'
$output=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output 'zig-cache'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\src" "-I$project\Petari\include" "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\libs\nw4r\include" "-I$project\Petari\libs\RVL_SDK\include" -include revolution/types.h "$native\src\archive_font_tests.cpp" "$native\src\resource_font_port.cpp" "$native\src\gpu_resources.cpp" "$native\src\game_font_backend.cpp" "$native\src\generated\GameFontHolderKernel.cpp" "$project\Petari\src\nw4r\ut\ut_Font.cpp" "$project\Petari\src\nw4r\ut\ut_ResFontBase.cpp" "$project\Petari\src\nw4r\ut\ut_CharStrmReader.cpp" -lbcrypt -o "$output\archive-font-tests.exe" *> "$output\archive-font-tests.log"
if($LASTEXITCODE){Get-Content "$output\archive-font-tests.log" -Tail 15;exit 1}
& "$output\archive-font-tests.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
exit $LASTEXITCODE





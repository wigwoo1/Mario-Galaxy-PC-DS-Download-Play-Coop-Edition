$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-dotcam-reader.ps1')
$out=Join-Path $n 'build\dotcam-reader-test';New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$includes=@();foreach($rel in @('native\src','native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include')){$includes+=('-I'+(Join-Path $p $rel))}
$sources=@('native\src\dotcam_reader_tests.cpp','native\src\generated\DotCamParams.cpp','native\src\generated\JMapInfo.cpp','Petari\src\JSystem\JGadget\hashcode.cpp','native\src\string_equal_case_port.cpp') | ForEach-Object {Join-Path $p $_}
$exe=Join-Path $out 'dotcam-reader-tests.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu @includes -include revolution/types.h @sources -lbcrypt -o $exe *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 18;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE

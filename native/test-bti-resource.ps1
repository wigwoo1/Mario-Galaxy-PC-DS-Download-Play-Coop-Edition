param([switch]$RawAttachment)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\bti-resource-test';New-Item -ItemType Directory -Force -Path $out|Out-Null
$exe=Join-Path $out 'bti-resource-tests.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu')
foreach($rel in @('native\src','native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include')){$compileArgs+=('-I'+(Join-Path $p $rel))}
$compileArgs+=@('-include','revolution/types.h',(Join-Path $n 'src\bti_resource_tests.cpp'),(Join-Path $n 'src\generated\TextureBufferSizeKernel.cpp'),'-lbcrypt','-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 18;exit $LASTEXITCODE}
$testArgs=@('D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs');if($RawAttachment){$testArgs+='raw'}
& $exe @testArgs
exit $LASTEXITCODE

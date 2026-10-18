$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\jai-stream-manager-cache"
$out="$n\build\actor-runtime-jai-stream-manager"
New-Item -ItemType Directory -Force -Path $out|Out-Null
$source="$p\Petari\src\JSystem\JAudio2\JAIStreamMgr.cpp"
$adapted="$out\JAIStreamMgrHost.cpp"
$original=[IO.File]::ReadAllText($source)
$old='bool result = mStreamAramMgr->deleteStreamAram((u32)aramAddr);'
$new='bool result = reinterpret_cast<uintptr_t>(aramAddr) <= UINT32_MAX && mStreamAramMgr->deleteStreamAram(static_cast<u32>(reinterpret_cast<uintptr_t>(aramAddr)));'
if(-not $original.Contains($old)){throw 'Expected original JAIStreamMgr ARAM address conversion was not found'}
[IO.File]::WriteAllText($adapted,$original.Replace($old,$new),[Text.UTF8Encoding]::new($false))
$compiler="$p\tools\zig-x86_64-windows-0.16.0\zig.exe"
$common=@('-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-std=c++17','-O2','-c','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\RVLFaceLib\include","-I$p\Petari\libs\nw4r\include",'-include',"$n\include-runtime\native_no_weak.h",'-include','revolution/types.h','-include',"$n\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$n\include-runtime\nw4r\ut\Color.h")
& $compiler c++ @common $adapted -o "$out\JAIStreamMgr.obj" *> "$out\compile.log"
if($LASTEXITCODE){Get-Content "$out\compile.log" -Tail 24;exit 1}
Write-Output 'PASS original JAIStreamMgr compiled with a checked Wii ARAM address conversion'

param([string]$ImagePath='D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs')
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-bas-resource.ps1')
$out=Join-Path $n 'build\bas-animation';New-Item -ItemType Directory -Force -Path $out | Out-Null
$original=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JAISound.cpp'))
$methods=foreach($signature in @('void JAISound::stop()','void JAISound::releaseHandle()')){
 if([regex]::Matches($original,[regex]::Escape($signature)).Count-ne1){throw "Original sound method anchor: $signature"}
 $start=$original.IndexOf($signature);$open=$original.IndexOf('{',$start);$end=$open+1;$depth=1
 while($depth-and $end-lt $original.Length){if($original[$end]-eq '{'){$depth++};if($original[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Unbalanced original sound method'};$original.Substring($start,$end-$start)
}
$source=[IO.File]::ReadAllText((Join-Path $n 'src\bas_animation_tests.cpp'))
$cpp=Join-Path $out 'bas_animation_tests.cpp';[IO.File]::WriteAllText($cpp,$source.Replace('// ORIGINAL_SOUND_HANDLE_METHODS',($methods -join "`n")),[Text.UTF8Encoding]::new($false))
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-target','x86_64-windows-gnu')
foreach($path in @('native\src','native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include','Petari\libs\nw4r\include')){$compileArgs+=('-I'+(Join-Path $p $path))}
$compileArgs+=@('-include','revolution/types.h',$cpp)
foreach($unit in @('generated\AllocatorDispatchKernel','generated\LayoutDefaultAllocatorKernel','generated\JKRUnitHeap','generated\JKRSolidHeap','generated\JKRExpHeap','generated\JKRHeapDispatch','generated\JKRDisposer','os_mutex_port','native_thread_identity_port','os_thread_port','os_message_port','generated\AsyncJobKernel','generated\CurrentHeapKernel','generated\HeapMemoryWatcher','input_workspace_port','os_panic_port','heap_error_port','heap_arena_port')){$compileArgs+=(Join-Path $n ('src\'+$unit+'.cpp'))}
$compileArgs+=@((Join-Path $p 'Petari\src\JSystem\JSupport\JSUList.cpp'),(Join-Path $p 'Petari\src\JSystem\JAudio2\JAUSoundAnimator.cpp'),'-lbcrypt','-o',(Join-Path $out 'bas-animation-tests.exe'))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 18;exit $LASTEXITCODE}
& (Join-Path $out 'bas-animation-tests.exe') $ImagePath
exit $LASTEXITCODE

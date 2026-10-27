param([string]$ImagePath='D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs')
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-tev-reg-resource.ps1')
$out=Join-Path $n 'build\tev-reg-animation';New-Item -ItemType Directory -Force -Path $out | Out-Null
$materialSource=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\J3DGraphAnimator\J3DMaterialAttach.cpp'))
$start=$materialSource.IndexOf('void J3DMaterialTable::clear()')
$end=$materialSource.IndexOf('int J3DMaterialTable::removeMatColorAnimator(', $start)
if($start-lt 0-or $end-le $start){throw 'Original material-table lifetime methods missing'}
$materialLifetime=Join-Path $out 'MaterialTableLifetime.cpp'
[IO.File]::WriteAllText($materialLifetime,'#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>'+"`n"+$materialSource.Substring($start,$end-$start),[Text.UTF8Encoding]::new($false))
$argsList=@('-std=c++17','-O2','-UNDEBUG','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-target','x86_64-windows-gnu')
foreach($path in @('native\src','native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include','Petari\libs\nw4r\include')){$argsList+=('-I'+(Join-Path $p $path))}
$argsList+=@('-include','revolution/types.h','-include','revolution/os.h','-include','revolution/gx/GXVert.h','-include','revolution/os/OSTime.h','-include','JSystem/J3DGraphBase/J3DGD.hpp','-include','JSystem/J3DGraphBase/J3DFifo.hpp',(Join-Path $n 'src\tev_reg_animation_tests.cpp'))
foreach($unit in @('generated\AllocatorDispatchKernel','generated\LayoutDefaultAllocatorKernel','generated\JKRUnitHeap','generated\JKRSolidHeap','generated\JKRExpHeap','generated\JKRHeapDispatch','generated\JKRDisposer','os_mutex_port','native_thread_identity_port','os_thread_port','os_message_port','generated\AsyncJobKernel','generated\CurrentHeapKernel','generated\HeapMemoryWatcher','input_workspace_port','os_panic_port','heap_error_port','heap_arena_port','generated\J3DAnimation')){$argsList+=(Join-Path $n ('src\'+$unit+'.cpp'))}
$argsList+=@((Join-Path $p 'Petari\src\JSystem\JSupport\JSUList.cpp'),(Join-Path $p 'Petari\src\JSystem\JUtility\JUTNameTab.cpp'),$materialLifetime,'-lbcrypt','-o',(Join-Path $out 'tev-reg-animation-tests.exe'))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @argsList *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 25;exit $LASTEXITCODE}
& (Join-Path $out 'tev-reg-animation-tests.exe') $ImagePath | Tee-Object -FilePath (Join-Path $out 'result.txt')
exit $LASTEXITCODE

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-camera-animation-runtime.ps1')
$out=Join-Path $n 'build\camera-animation-runtime'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$zig=Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$common=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include",'-include','revolution/types.h')
foreach($version in @('old','current')){
 $path=if($version-eq 'old'){Join-Path $p 'Petari\src\Game\Camera\CameraAnim.cpp'}else{Join-Path $n 'src\generated\CameraAnimRuntime.cpp'}
 $text=[IO.File]::ReadAllText($path)
 $start=$text.IndexOf('void CamAnmDataAccessor::set(');$end=$text.IndexOf('CameraAnim::CameraAnim(',$start)
 if($start-lt 0-or $end-le $start){throw 'Original camera accessor boundaries changed'}
 $sampler=Join-Path $out ($version+'-sampler.cpp')
 $body='#include <Game/Util/MathUtil.hpp>'+"`n"+'#include <Game/Camera/CameraAnim.hpp>'+"`n"+'#include <JSystem/JMath/JMath.hpp>'+"`n"+$text.Substring($start,$end-$start)
 [IO.File]::WriteAllText($sampler,$body,[Text.UTF8Encoding]::new($false))
 $exe=Join-Path $out ($version+'-camera-tests.exe')
 & $zig c++ @common (Join-Path $n 'src\retail_camera_animation_tests.cpp') $sampler -lbcrypt -o $exe *> (Join-Path $out ($version+'-compile.txt'))
 if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out ($version+'-compile.txt')) -Tail 24;exit 1}
 $args=@('D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs')
 if($version-eq 'old'){$args+='--old'}
 & $exe @args *> (Join-Path $out ($version+'-run.txt'))
 $code=$LASTEXITCODE
 Get-Content -LiteralPath (Join-Path $out ($version+'-run.txt'))
 if($version-eq 'old'){
  if($code-ne -1073741819){throw "Expected old raw reader access violation, got $code"}
  'PASS original retail camera access violation reproduced' | Tee-Object -FilePath (Join-Path $out 'before-result.txt')
 }elseif($code){exit $code}
}

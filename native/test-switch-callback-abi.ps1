param([string]$ProviderObject)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\switch-callback-abi-test';New-Item -ItemType Directory -Force -Path $out|Out-Null
$provider=if($ProviderObject){$ProviderObject}else{Join-Path $p 'Petari\src\Game\Util\SwitchEventFunctorListener.cpp'}
$exe=Join-Path $out 'switch-callback-abi-tests.exe'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu')
foreach($relative in @('native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include')){$compileArgs+=('-I'+(Join-Path $p $relative))}
$compileArgs+=@('-include','revolution/types.h',(Join-Path $n 'src\switch_callback_abi_tests.cpp'),$provider,'-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 15;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE

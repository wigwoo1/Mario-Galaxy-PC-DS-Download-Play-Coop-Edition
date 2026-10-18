$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out="$n\build\actor-runtime-jutexception"
New-Item -ItemType Directory -Path $out -Force|Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\actor-runtime-jutexception-cache"
$header=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JUtility\JUTException.hpp" -Raw
$header=$header -replace 'virtual ~JUTException\(\)\s*\{\s*\}', 'virtual ~JUTException();'
$header=$header -replace 'virtual void\* run\(\)\s*\{\s*return nullptr;\s*\}', 'virtual void* run();'
if($header -match 'virtual ~JUTException\(\)\s*\{' -or $header -match 'virtual void\* run\(\)\s*\{') {throw 'JUTException host vtable header transformation failed'}
$headerPath="$out\include\JSystem\JUtility\JUTException.hpp"
New-Item -ItemType Directory -Path (Split-Path $headerPath) -Force|Out-Null
Set-Content -LiteralPath $headerPath -Value $header -NoNewline
$common=@('-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',
    "-I$out\include","-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include",
    "-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\nw4r\include",'-include','revolution/types.h')
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @common -include "$n\include\Native\HeapPlacement.hpp" `
    -c "$n\src\jut_exception_host.cpp" -o "$out\JUTExceptionHost.obj" *> "$out\JUTExceptionHost.log"
if($LASTEXITCODE){Get-Content "$out\JUTExceptionHost.log" -Tail 50;exit $LASTEXITCODE}
$diagnostics=Get-Content "$n\src\jut_console_port.cpp" | Where-Object {$_ -notmatch '^extern "C" void JUTReportConsole'}
Set-Content -LiteralPath "$out\JUTConsoleHostDiagnosticsNoPanic.cpp" -Value $diagnostics
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @common -c "$out\JUTConsoleHostDiagnosticsNoPanic.cpp" `
    -o "$out\JUTConsoleHostDiagnosticsNoPanic.obj" *> "$out\JUTConsoleHostDiagnosticsNoPanic.log"
if($LASTEXITCODE){Get-Content "$out\JUTConsoleHostDiagnosticsNoPanic.log" -Tail 30;exit $LASTEXITCODE}
Write-Output 'Compiled host exception startup services and non-duplicate console diagnostics.'

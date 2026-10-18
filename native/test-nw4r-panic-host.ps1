$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$zig = "$p\tools\zig-x86_64-windows-0.16.0\zig.exe"
$outDir = "$n\build\nw4r-panic-host-test"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

& $zig c++ -std=c++17 -O2 -target x86_64-windows-gnu `
    "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\nw4r\include" `
    "$n\src\nw4r_panic_host.cpp" "$n\src\nw4r_panic_host_tests.cpp" `
    -o "$outDir\nw4r-panic-host-tests.exe" *> "$outDir\compile.log"
if ($LASTEXITCODE -ne 0) {
    Get-Content "$outDir\compile.log" -Tail 20
    exit $LASTEXITCODE
}

$stderr = "$outDir\panic.stderr.txt"
$process = Start-Process -FilePath "$outDir\nw4r-panic-host-tests.exe" -NoNewWindow -Wait -PassThru -RedirectStandardError $stderr
$diagnostic = Get-Content $stderr -Raw
if ($process.ExitCode -eq 0) { throw 'NW4R Panic unexpectedly returned normally' }
if ($diagnostic -notmatch 'nw4r-panic-test\.cpp:73: bad state=9') {
    throw "NW4R Panic diagnostic was incomplete: $diagnostic"
}
Write-Output 'PASS NW4R Panic reports source/line and formatted arguments, then terminates'

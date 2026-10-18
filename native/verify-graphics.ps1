param([string]$DiscPath = 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs')
$ErrorActionPreference = 'Stop'
$probe = Join-Path $PSScriptRoot 'build\galaxy-probe.exe'
if (-not (Test-Path -LiteralPath $probe)) { throw 'Run build.ps1 first' }
& $probe --verify-graphics $DiscPath 1> "$PSScriptRoot\build\graphics-verification.tsv" 2> "$PSScriptRoot\build\graphics-verification-log.txt"
$verifyExit = $LASTEXITCODE
Get-Content "$PSScriptRoot\build\graphics-verification-log.txt" -Tail 1
if ($verifyExit -ne 0) { throw 'Graphics verification failed' }

param([string]$DiscPath = 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs')
$ErrorActionPreference = 'Stop'
$probe = Join-Path $PSScriptRoot 'build\galaxy-probe.exe'
if (-not (Test-Path -LiteralPath $probe)) { throw 'Run build.ps1 first' }
& $probe --list-assets $DiscPath 1> "$PSScriptRoot\build\asset-index.tsv" 2> "$PSScriptRoot\build\asset-index-log.txt"
if ($LASTEXITCODE -ne 0) { throw 'Asset indexing failed; see build/asset-index-log.txt' }
& $probe --verify-archives $DiscPath 1> "$PSScriptRoot\build\archive-verification.tsv" 2> "$PSScriptRoot\build\archive-verification-log.txt"
Get-Content "$PSScriptRoot\build\archive-verification-log.txt" -Tail 1
if ($LASTEXITCODE -ne 0) { throw 'Archive verification failed' }

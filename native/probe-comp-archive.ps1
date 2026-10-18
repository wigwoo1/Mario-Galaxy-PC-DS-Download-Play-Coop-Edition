$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$line=Get-Content "$n\test-owned-archive-game.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$line=$line.Replace('c++ -std','c++ -c -std').Replace('src\owned_archive_game_tests.cpp','src\generated\JKRCompArchive.cpp').Replace(' -lbcrypt','').Replace('owned-archive-game-tests.exe','comp-archive-probe.obj').Replace('owned-archive-game-tests.log','comp-archive-probe.log')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\comp-archive-probe.log" -Tail 25;exit 1}
Write-Output 'PASS combined archive provider compiles; execution remains pending'

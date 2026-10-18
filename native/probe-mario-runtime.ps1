param([switch]$AllPlayer)
# Diagnostic only: this does not build or verify playable Mario gameplay.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$compiler = Join-Path $projectRoot 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$petariRoot = Join-Path $projectRoot 'Petari'
$outputRoot = Join-Path $PSScriptRoot 'build\mario-runtime-probe'
New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $projectRoot 'tools\zig-cache'
& (Join-Path $PSScriptRoot 'generate-mario-runtime.ps1')
$results = @()
$units=@('Mario','MarioActor','MarioJump','MarioActorInit','MarioModule','MarioState','MarioTask','MarioMove','MarioMove2D','MarioMove25D','MarioMoveSphere','MarioInit','MarioActorGravity','MarioCollision','MarioFlip')
if ($AllPlayer) {
    $outputRoot=Join-Path $PSScriptRoot 'build\mario-runtime-all-player'
    New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null
    $units=@(Get-ChildItem -LiteralPath (Join-Path $petariRoot 'src\Game\Player') -Filter 'Mario*.cpp' | Sort-Object Name | ForEach-Object { $_.BaseName })
}
foreach ($unit in $units) {
    $sourcePath = Join-Path $petariRoot "src\Game\Player\$unit.cpp"
    if ($unit -eq 'Mario') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\Mario.cpp' }
    if ($unit -eq 'MarioActor') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioActor.cpp' }
    if ($unit -eq 'MarioJump') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioJump.cpp' }
    if ($unit -eq 'MarioMove') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioMove.cpp' }
    if ($unit -eq 'MarioActorGravity') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioActorGravity.cpp' }
    if ($unit -eq 'MarioCollision') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioCollision.cpp' }
    if ($unit -eq 'MarioEffect') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioEffect.cpp' }
    if ($unit -in @('MarioBee','MarioDamage','MarioStick','MarioWall','MarioActorSensor','MarioActorEye','MarioTeresa','MarioSideStep','MarioActorRushMsg','MarioActorParts')) { $sourcePath = Join-Path $PSScriptRoot ("src\generated\$unit.cpp") }
    if ($unit -in @('MarioSwim','MarioWarp')) { $sourcePath = Join-Path $PSScriptRoot ("src\generated\$unit.cpp") }
    if ($unit -eq 'MarioShadow') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioShadow.cpp' }
    if ($unit -eq 'MarioSearchLight') { $sourcePath = Join-Path $PSScriptRoot 'src\generated\MarioSearchLight.cpp' }
    if ($unit -in @('MarioActorDraw','MarioActorSpecialDraw')) { $sourcePath = Join-Path $PSScriptRoot ("src\generated\$unit.cpp") }
    $logPath = Join-Path $outputRoot "$unit.log"
    & $compiler c++ -std=c++17 -c -ferror-limit=20 -target x86_64-windows-gnu "-I$PSScriptRoot\include-runtime" "-I$PSScriptRoot\include" "-I$petariRoot\include" "-I$petariRoot\libs\JSystem\include" "-I$petariRoot\libs\RVL_SDK\include" "-I$petariRoot\libs\nw4r\include" -include revolution/types.h $sourcePath -o (Join-Path $outputRoot "$unit.obj") *> $logPath
    $compilerStatus = $LASTEXITCODE
    $results += [pscustomobject]@{ Unit=$unit; ExitCode=$compilerStatus; Log=$logPath }
}
$results | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $outputRoot 'results.json')
$results | Format-Table -AutoSize
if (@($results | Where-Object { $_.ExitCode -ne 0 }).Count -ne 0) { exit 1 }




















param([string]$Wbfs='D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs',[string]$Stage='/StageData/EggStarGalaxy.arc')
$ErrorActionPreference='Stop'
& "$PSScriptRoot\build\galaxy-probe.exe" --preview-runtime-body-smoke $Wbfs $Stage
if($LASTEXITCODE){throw 'Original runtime rendered-body verification failed'}
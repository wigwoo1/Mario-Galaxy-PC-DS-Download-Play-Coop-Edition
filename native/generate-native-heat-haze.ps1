$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Screen\HeatHazeEffect.cpp" -Raw
Set-Content "$n\src\generated\HeatHazeEffect.cpp" ('constexpr float PI = 3.1415927f;'+"`n"+$s)




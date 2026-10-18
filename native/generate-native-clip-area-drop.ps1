$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\ClipAreaDrop.cpp" -Raw
Set-Content "$n\src\generated\ClipAreaDrop.cpp" ('#include "JSystem/JMath/JMath.hpp"'+"`r`n"+$s)

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Enemy\KarikariDirector.cpp" -Raw
Set-Content "$n\src\generated\KarikariDirector.cpp" ('#include <JSystem/JGeometry/TMatrix.hpp>'+"`r`n"+'#include "math_types.hpp"'+"`r`n"+$s)


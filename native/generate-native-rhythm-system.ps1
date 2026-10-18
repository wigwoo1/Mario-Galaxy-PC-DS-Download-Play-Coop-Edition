$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\RhythmLib\AudRhythmMeSystem.cpp" -Raw
Set-Content "$n\src\generated\AudRhythmMeSystem.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)

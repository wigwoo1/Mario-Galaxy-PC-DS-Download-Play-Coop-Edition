$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\RhythmLib\AudMeObject.cpp" -Raw
Set-Content "$n\src\generated\AudMeObject.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)

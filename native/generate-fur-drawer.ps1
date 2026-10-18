$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\FurDrawer.cpp" -Raw
Set-Content "$n\src\generated\FurDrawer.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s) -Encoding UTF8

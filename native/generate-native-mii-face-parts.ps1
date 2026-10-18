$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\include\Game\Util\FixedPosition.hpp" -Raw
if(!$h.Contains('* _180_PI')){throw 'Missing original conversion'}
Set-Content "$n\include\Game\Util\FixedPosition.hpp" ($h.Replace('* _180_PI','* 57.29578f'))
$s=Get-Content "$p\Petari\src\Game\NPC\MiiFaceParts.cpp" -Raw
Set-Content "$n\src\generated\MiiFaceParts.cpp" $s

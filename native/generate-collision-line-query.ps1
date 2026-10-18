$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\CollisionCategorizedKeeper.cpp" -Raw
$signature='s32 CollisionCategorizedKeeper::checkStrikeLine('
$start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing original filtered line query'}
$brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
$includes=$s.Substring(0,$s.IndexOf('CollisionCategorizedKeeper::CollisionCategorizedKeeper'))
Set-Content "$n\src\generated\CollisionLineQuery.cpp" ($includes+$s.Substring($start,$end-$start))

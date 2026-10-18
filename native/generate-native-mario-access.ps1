$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Player\MarioAccess.cpp" -Raw
$a=$s.IndexOf('    MarioActor* getPlayerActor()');if($a -lt 0){throw 'Original actor lookup anchor missing'}
$o=$s.IndexOf('{',$a);$depth=1;$end=$o+1
while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
$s=$s.Remove($a,$end-$a)
Set-Content "$n\src\generated\MarioAccess.cpp" $s

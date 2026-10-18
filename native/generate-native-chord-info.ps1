$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\RhythmLib\AudChordInfo.cpp" -Raw
foreach($signature in @('void AudScaleData::initScaleData(', 'AudChordTable::AudChordTable(', 'bool AudChordTable::setChordTableResource(')){
$a=$s.IndexOf($signature);if($a -lt 0){throw "Missing $signature"};$b=$s.IndexOf('{',$a);$depth=1;$end=$b+1
while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
$s=$s.Remove($a,$end-$a)
}
Set-Content "$n\src\generated\AudChordInfo.cpp" $s

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\SysConfigFile.cpp" -Raw
foreach($signature in @('s32 SysConfigChunk::deserialize(', 's32 SysConfigChunk::serialize(')) {
$a=$s.IndexOf($signature);if($a -lt 0){throw "Missing $signature"};$b=$s.IndexOf('{',$a);$depth=1;$end=$b+1
while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
$s=$s.Remove($a,$end-$a)
}
$s += Get-Content "$n\src\sysconfig_serialization.inc" -Raw
Set-Content "$n\src\generated\SysConfigFile.cpp" $s

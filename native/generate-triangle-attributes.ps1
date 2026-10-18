$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\HitInfo.cpp" -Raw
$signature='JMapInfoIter Triangle::getAttributes() const'
$a=$s.IndexOf($signature);if($a -lt 0){throw 'Missing original triangle attributes provider'}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth){throw 'Unbalanced source body'}
$out=$s.Substring(0,$s.IndexOf('Triangle::Triangle()'))+$s.Substring($a,$i-$a)
foreach($signature in @('const char* Triangle::getHostName() const','s32 Triangle::getHostPlacementZoneID() const','bool Triangle::isValid() const')) {
$a=$s.IndexOf($signature);if($a -lt 0){throw "Missing original $signature"}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth){throw 'Unbalanced source body'}
$out += [Environment]::NewLine+$s.Substring($a,$i-$a)
}
Set-Content "$n\src\generated\TriangleAttributes.cpp" $out


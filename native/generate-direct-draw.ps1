$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\DirectDraw.cpp" -Raw
# ModelFogKernel already supplies the identical original function.
$a=$s.IndexOf('    void mixFogColor(');if($a -lt 0){throw 'Missing fog duplicate anchor'};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
Set-Content "$n\src\generated\DirectDrawKernel.cpp" ($s.Remove($a,$e-$a))

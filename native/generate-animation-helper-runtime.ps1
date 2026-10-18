$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-animation-helpers.ps1"
$source=[System.IO.File]::ReadAllText("$n\src\generated\AnimationHelperKernel.cpp")
$signature='void extractString('
$start=$source.IndexOf($signature)
if($start -lt 0){throw 'Generated animation extractString helper anchor missing'}
$open=$source.IndexOf('{',$start)
if($open -lt 0){throw 'Generated animation helper body missing'}
$depth=1;$end=$open+1
while($depth -gt 0 -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++}elseif($source[$end] -eq '}'){$depth--};$end++}
if($depth -ne 0){throw 'Unbalanced extractString helper body'}
$source=$source.Remove($start,$end-$start)
[System.IO.File]::WriteAllText("$n\src\generated\AnimationHelperRuntimeKernel.cpp",$source,[System.Text.UTF8Encoding]::new($false))

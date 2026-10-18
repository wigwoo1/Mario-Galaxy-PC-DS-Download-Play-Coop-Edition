$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\GCapture.cpp" -Raw
$s="namespace NativeGCapture {} using namespace NativeGCapture;`r`n"+$s.Replace('namespace {','namespace NativeGCapture {')
Set-Content "$n\src\generated\GCapture.cpp" $s

$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\Note.cpp" -Raw
$a='mGravity.set(0.0f, -1.0f, 0.0);'
if(!$s.Contains($a)){throw 'Missing original gravity initializer'}
Set-Content "$n\src\generated\Note.cpp" $s.Replace($a,'mGravity.set(0.0f, -1.0f, 0.0f);')

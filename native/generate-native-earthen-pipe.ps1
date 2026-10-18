$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\EarthenPipe.cpp" -Raw
$a='MR::startBckPlayer("EarthenPipeJump", 0L);'
if(!$s.Contains($a)){throw 'Missing original pipe animation call'}
Set-Content "$n\src\generated\EarthenPipe.cpp" $s.Replace($a,'MR::startBckPlayer("EarthenPipeJump", static_cast<s32>(0));')

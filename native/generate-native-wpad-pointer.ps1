$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\WPadPointer.cpp" -Raw
$start=$s.IndexOf('void WPadPointer::reset()')
if($start -lt 0){throw 'Missing original pointer reset'}
$prefix=$s.Substring(0,$start);$resetAndRest=$s.Substring($start)
$anchor='    mValidInScreenTime = 0;'
if(!$resetAndRest.Contains($anchor)){throw 'Missing original reset counter'}
$at=$resetAndRest.IndexOf($anchor);$resetAndRest=$resetAndRest.Insert($at+$anchor.Length,"`n    mInValidOutScreenTime = 0;")
Set-Content "$n\src\generated\WPadPointer.cpp" ($prefix+$resetAndRest)


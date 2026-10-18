$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Enemy\BegomanBaby.cpp" -Raw
$a='GET_NERVE_ANON(NrvBegomanBaby::HostTypeNrvWait)'
if(!$s.Contains($a)){throw 'Missing original baby nerve anchor'}
Set-Content "$n\src\generated\BegomanBaby.cpp" $s.Replace($a,'GET_NERVE_DIRECT(NrvBegomanBaby, HostTypeNrvWait)')

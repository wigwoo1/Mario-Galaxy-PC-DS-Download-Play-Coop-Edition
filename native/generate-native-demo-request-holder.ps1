$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Demo\DemoStartRequestHolder.cpp" -Raw
$tail=$s.IndexOf('template <>');$increment=$s.IndexOf('template <>', $s.IndexOf('::push_back(', $tail))
$body=$s.IndexOf('DemoStartInfo::DemoStartInfo()')
if($tail -lt 0 -or $increment -lt 0 -or $body -lt 0){throw 'Original request ring specialization boundaries missing'}
$specializations=$s.Substring($increment)+"`r`n"+$s.Substring($tail,$increment-$tail)
($s.Substring(0,$body)+$specializations+"`r`n"+$s.Substring($body,$tail-$body))|Set-Content "$n\src\generated\DemoStartRequestHolder.cpp"

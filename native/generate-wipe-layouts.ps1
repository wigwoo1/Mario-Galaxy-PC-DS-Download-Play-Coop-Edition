$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($name in @('WipeRing','WipeKoopa')){
$s=Get-Content "$p\Petari\src\Game\Screen\$name.cpp" -Raw
$anchor='namespace {'
if(!$s.Contains($anchor)){throw "Original $name anonymous state anchor missing"}
$s=$s.Replace($anchor,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n"+$anchor)
Set-Content "$n\src\generated\$name.cpp" $s
}

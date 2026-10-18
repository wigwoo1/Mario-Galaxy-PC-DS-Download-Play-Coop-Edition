$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($entry in @(@('Screen','TitleSequenceProduct'),@('Screen','EncouragePal60Window'),@('Screen','ErrorMessageWindow'),@('Screen','HomeButtonLayout'))){
 $unit=$entry[1];$s=Get-Content "$p\Petari\src\Game\$($entry[0])\$unit.cpp" -Raw
 $a=$s.IndexOf('namespace {');if($a -ge 0){$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n")}
 Set-Content "$n\src\generated\$unit.cpp" $s
}




$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($unit in @('StorySequenceExecutor','FindingLuigiEventScheduler','GalaxyCometScheduler','SaveDataHandleSequence','NANDErrorSequence')){
 $s=Get-Content "$p\Petari\src\Game\System\$unit.cpp" -Raw
 $a=$s.IndexOf('namespace {')
 if($a -ge 0){$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n")}
 $s=$s.Replace('tryNextDemoInfo() != nullptr','tryNextDemoInfo()')
 Set-Content "$n\src\generated\$unit.cpp" $s
}




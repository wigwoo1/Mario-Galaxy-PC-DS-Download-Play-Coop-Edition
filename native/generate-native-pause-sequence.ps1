$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($unit in @('GamePauseSequence','PauseMenu','GameScenePauseControl','GameSceneScenarioOpeningCameraState','ScenarioTitle')){
 $directory=if($unit -in @('GameScenePauseControl','GameSceneScenarioOpeningCameraState')){'Scene'}else{'Screen'}
 $s=Get-Content "$p\Petari\src\Game\$directory\$unit.cpp" -Raw
 $a=$s.IndexOf('namespace {');if($a -ge 0){$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n")}
 $s=$s.Replace('ARRAY_SIZEU(starPaneNames)','(sizeof(starPaneNames)/sizeof(starPaneNames[0]))')
 Set-Content "$n\src\generated\$unit.cpp" $s
}





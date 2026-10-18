$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudSceneMgr.cpp" -Raw
$s=$s.Replace('if (scenarioNo <= 0) {','if (scenarioNo <= 0 || scenarioNo > 6) {').Replace('if (pSoundInfo->mScenarioId < 0) {','if (pSoundInfo->mScenarioId < 0 || pSoundInfo->mScenarioId >= ARRAY_SIZE(::cScenarioNoToWaveSet)) {')
Set-Content "$n\src\generated\AudioSceneKernel.cpp" $s

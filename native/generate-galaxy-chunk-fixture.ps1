$ErrorActionPreference='Stop'
$n=$PSScriptRoot
& "$n\generate-native-content-header.ps1"
& "$n\generate-native-galaxy-chunk-codec.ps1"
$s=Get-Content "$n\src\generated\GameDataGalaxyStorage.cpp" -Raw
$f=$s.Substring(0,$s.IndexOf('bool GameDataSomeScenarioAccessor::hasPowerStar'))
foreach($method in @('GameDataSomeGalaxyStorage::GameDataSomeGalaxyStorage','void GameDataSomeGalaxyStorage::resetAllData','s32 GameDataSomeGalaxyStorage::deserialize','void GameDataSomeGalaxyStorage::serialize','s32 GameDataAllGalaxyStorage::getGalaxyNum','GameDataSomeGalaxyStorage* GameDataAllGalaxyStorage::getGalaxyStorage','u32 GameDataAllGalaxyStorage::makeHeaderHashCode','u32 GameDataAllGalaxyStorage::getSignature','s32 GameDataAllGalaxyStorage::deserialize','void GameDataAllGalaxyStorage::initializeData','void GameDataAllGalaxyStorage::initHeaderSerializer','s32 GameDataAllGalaxyStorage::findIndex(u16','s32 GameDataAllGalaxyStorage::serialize')){
 $a=$s.IndexOf($method);if($a -lt 0){throw "Missing $method"};$b=$s.IndexOf('{',$a);$depth=1;$e=$b+1
 while($depth){if($s[$e] -eq '{'){$depth++};if($s[$e] -eq '}'){$depth--};$e++}
 $f+=$s.Substring($a,$e-$a)+[Environment]::NewLine
}
Set-Content "$n\src\generated\GalaxyChunkCodecFixture.cpp" $f

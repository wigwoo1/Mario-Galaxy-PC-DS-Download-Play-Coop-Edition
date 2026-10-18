$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out="$n\build\camera-pointer-candidate"
$rows=Get-Content "$out\actor-runtime-probe\results.json" -Raw|ConvertFrom-Json
if(@($rows|Where-Object ExitCode -ne 0).Count){throw 'Candidate actor rebuild has failed units'}
& "$n\generate-native-audio-arc-loader.ps1"
& "$n\generate-native-dvd-ripper.ps1"
$s=Get-Content "$n\build\actor-runtime-link.rsp" -Raw
$s=[regex]::Replace($s,'"[^"\r\n]*native-input-provider\\native_(kpad_input|keyboard_kpad|wpad_port)\.obj"','')
$s=[regex]::Replace($s,'"[^"\r\n]*RuntimeStringScans\.obj"','')
$s=$s.Replace("$p\Petari\src\JSystem\JAudio2\JAUAudioArcLoader.cpp", "$n\src\generated\AudioArcLoaderKernel.cpp")
$s=[regex]::Replace($s,'"[^"\r\n]*actor-runtime-probe\\JKRDvdRipper\.obj"','')
$s+=' "'+"$n\src\generated\JKRDvdRipperKernel.cpp"+'" '
$s='-I"'+"$p\Petari\libs\RVLFaceLib\include"+'" '+$s

$s='-I"'+"$out\include"+'" '+$s
$s=$s.Replace('"'+"$n\src\generated\CameraAccessKernel.cpp"+'"','')
$s=$s.Replace("$n\build\actor-runtime-probe\","$out\actor-runtime-probe\")
$s=$s.Replace('"'+"$out\actor-runtime-probe\StageResourceLoader.obj"+'"','')
$s=$s.Replace('"'+"$out\actor-runtime-probe\StarPointerTarget.obj"+'"','')
& "$n\generate-actor-runtime.ps1"
$s=$s.Replace('"'+ "$out\actor-runtime-probe\ResourceHolder.obj"+'"','"'+ "$n\src\generated\ResourceHolder.cpp"+'"')
$s='-DGALAXY_NATIVE_MATERIAL_ANIMATOR_POINTERS '+$s
foreach($unit in @('J3DMaterial','J3DMaterialAnm','J3DAnimation')){
 $source="$n\src\generated\$unit.cpp"
 if(!(Test-Path $source)){$source="$p\Petari\src\JSystem\J3DGraphAnimator\$unit.cpp"}
 if($unit -eq 'J3DMaterial'){$source="$p\Petari\src\JSystem\J3DGraphBase\J3DMaterial.cpp"}
 $s=$s.Replace('"'+ "$out\actor-runtime-probe\$unit.obj"+'"','"'+$source+'"')
}
$objects=[regex]::Matches($s,'"([^"\r\n]*actor-runtime-probe\\[^"\r\n]*\.obj)"')
$missingCandidateObjects=[System.Collections.Generic.List[string]]::new()
foreach($m in $objects){
 $object=$m.Groups[1].Value
 if(Test-Path -LiteralPath $object){continue}
 $unit=[IO.Path]::GetFileNameWithoutExtension($object)
 $source="$n\src\generated\$unit.cpp"
 if(!(Test-Path $source)){
  if($unit.StartsWith('GD')){$source="$p\Petari\src\RVL_SDK\gd\$unit.c"}
  else{$source="$p\Petari\src\JSystem\JUtility\$unit.cpp"}
 }
 if(!(Test-Path $source)){$s=$s.Replace($m.Value,'');$missingCandidateObjects.Add($unit);continue}
 # Compile these sources in the same invocation/header layout instead of reusing old objects.
 $s=$s.Replace($m.Value,'"'+$source+'"')
}
& "$n\generate-native-draw-sync.ps1"
& "$n\generate-pointer-projection.ps1"
$additional=' "'+"$n\src\generated\PointerProjection.cpp"+'"'
$additional+=' "'+"$n\src\gx_depth_peek_port.cpp"+'"'
$additional+=' "'+"$n\src\gx_draw_sync_completion_port.cpp"+'"'
$additional+=' "'+"$n\src\generated\DrawSyncManager.cpp"+'"'
foreach($unit in @('CameraParamChunk','CamTranslatorAnim','CameraManGame','CameraUtil','CameraParamChunkHolder','CameraAnim')){
 $additional+=' "'+"$out\src\$unit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\Camera\CameraParamString.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Camera\CameraParamChunkID.cpp"+'" "'+"$p\Petari\src\Game\Camera\CameraManEvent.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Camera\DotCamParams.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Camera\CameraHolder.cpp"+'"'
foreach($cameraUnit in @('CameraFollow','CameraParallel','CamTranslatorFollow','CamTranslatorParallel','CameraCalc','CameraTripodPlanet','CameraCubePlanet','CamTranslatorTripodPlanet','CamTranslatorCubePlanet','CameraWaterFollow','CameraWaterPlanet','CamTranslatorWaterFollow','CamTranslatorWaterPlanet','CameraTalk','CamTranslatorTalk','CameraWaterPlanetBoss','CamTranslatorWaterPlanetBoss','CameraTripodBoss','CamTranslatorTripodBoss','CameraTower','CamTranslatorTower','CameraRailFollow','CameraRailWatch','CamTranslatorRailFollow','CamTranslatorRailWatch','CameraRailHolder','CameraFix','CameraFixedPoint','CamTranslatorFix','CamTranslatorFixedPoint','CameraFooFighterPlanet','CamTranslatorFooFighterPlanet','CameraBlackHole','CamTranslatorBlackHole','CameraDPD','CamTranslatorDPD','CameraCover','CameraViewInterpolator','CameraRotChecker','CameraGround','CamTranslatorGround','CameraSpiral','CamTranslatorSpiral','CameraInnerCylinder','CamTranslatorInnerCylinder','CameraTrundle','CamTranslatorTrundle','CameraWonderPlanet','CamTranslatorWonderPlanet','CameraBehind','CamTranslatorBehind','CameraInwardTower','CamTranslatorInwardTower','CameraFixedThere','CamTranslatorFixedThere','CameraTowerPos','CamTranslatorTowerPos','CameraDead','CamTranslatorDead','CameraInwardSphere','CamTranslatorInwardSphere','CameraRailDemo','CamTranslatorRailDemo','CameraTripodBossJoint','CamTranslatorTripodBossJoint','CameraCharmedTripodBoss','CamTranslatorCharmedTripodBoss','CameraObjParallel','CamTranslatorObjParallel','CameraCharmedFix','CamTranslatorCharmedFix','CameraMtxRegParallel','CamTranslatorMtxRegParallel','CameraCharmedVecReg','CamTranslatorCharmedVecReg','CameraMedianPlanet','CamTranslatorMedianPlanet','CameraTwistedPassage','CamTranslatorTwistedPassage','CameraMedianTower','CamTranslatorMedianTower','CameraCharmedVecRegTower','CamTranslatorCharmedVecRegTower','CameraFrontAndBack','CamTranslatorFrontAndBack','CameraRaceFollow','CamTranslatorRaceFollow','CameraSlide','CamTranslatorSlide','CameraFooFighter','CamTranslatorFooFighter','CameraTowerBase')){
 $additional+=' "'+"$p\Petari\src\Game\Camera\$cameraUnit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\Util\TriangleFilter.cpp"+'"'
$additional+= ' "'+"$n\src\native_wpad_rumble_query.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Boss\TripodBoss.cpp"+'"'
foreach($tripodUnit in @('TripodBossStepSequence','TripodBossStepPoint')) {
 $additional+=' "'+"$p\Petari\src\Game\Boss\$tripodUnit.cpp"+'"'
}
& "$n\generate-native-tripod-movable-area.ps1"
$additional+=' "'+"$n\src\generated\TripodBossMovableArea.cpp"+'"'
& "$n\generate-native-tripod-leg.ps1"
$additional+=' "'+"$n\src\generated\TripodBossLeg.cpp"+'"'
foreach($tripodPartsUnit in @('TripodBossFixPartsBase','TripodBossFixParts','TripodBossRailMoveParts','TripodBossRotateParts','TripodBossBaseJointPosition')) {
 $additional+=' "'+"$p\Petari\src\Game\Boss\$tripodPartsUnit.cpp"+'"'
}
$additional+= ' "'+"$p\Petari\src\Game\Boss\TripodBossBreakMovement.cpp"+'"'
foreach($tripodEncounterUnit in @('TripodBossCore','TripodBossShell','TripodBossCoin','TripodBossKinokoOneUp')) {
 $additional+=' "'+"$p\Petari\src\Game\Boss\$tripodEncounterUnit.cpp"+'"'
}
foreach($tripodTriggerUnit in @('TripodBossGuardWall','TripodBossKillerGenerater','TripodBossStepStartArea')) {
 $additional+=' "'+"$p\Petari\src\Game\Boss\$tripodTriggerUnit.cpp"+'"'
}
& "$n\generate-native-tripod-cannon-circle.ps1"
$additional+=' "'+"$n\src\generated\TripodBossKillerGeneraterCircle.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Boss\TripodBossGuardWallPart.cpp"+'"'
& "$n\generate-native-homing-killer.ps1"
$additional+=' "'+"$n\src\generated\HomingKiller.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\DummyDisplayModel.cpp"+'"'
& "$n\generate-player-distance.ps1"
& "$n\generate-model-light-counts.ps1"
$additional+=' "'+"$n\src\generated\ModelLightCounts.cpp"+'"'
foreach($arrowUnit in @('ArrowSwitchMultiHolder','ArrowSwitchMulti')) {
 $additional+=' "'+"$p\Petari\src\Game\MapObj\$arrowUnit.cpp"+'"'
}
& "$n\generate-native-earthen-pipe.ps1"
$additional+=' "'+"$n\src\generated\EarthenPipe.cpp"+'"'
foreach($fishRailUnit in @('SkeletalFishBossRailHolder','SkeletalFishBossRail')) {
 $additional+=' "'+"$p\Petari\src\Game\Boss\$fishRailUnit.cpp"+'"'
}
foreach($flameUnit in @('KabokuriFireHolder','KabokuriFire')) {
 $additional+=' "'+"$p\Petari\src\Game\Enemy\$flameUnit.cpp"+'"'
}
& "$n\generate-native-begoman-base.ps1"
$additional+=' "'+"$n\src\generated\BegomanBase.cpp"+'"'
foreach($begomanVariant in @('BegomanSpike','BegomanSpring','BegomanSpringHead')) {
 $additional+=' "'+"$p\Petari\src\Game\Enemy\$begomanVariant.cpp"+'"'
}
& "$n\generate-native-begoman-baby.ps1"
$additional+=' "'+"$n\src\generated\BegomanBaby.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\BegomanLauncher.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\QuakeEffectGenerator.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\WarpPod.cpp"+'"'
& "$n\generate-native-note.ps1"
$additional+=' "'+"$n\src\generated\Note.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\NoteFairy.cpp"+'"'
foreach($fieldDrawUnit in @('FallOutFieldDraw','ClipFieldFillDraw')) {
 $additional+=' "'+"$p\Petari\src\Game\MapObj\$fieldDrawUnit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\LiveActor\VolumeModelDrawer.cpp"+'"'
& "$n\generate-native-water-plant.ps1"
$additional+=' "'+"$n\src\generated\WaterPlant.cpp"+'"'
& "$n\generate-native-heat-haze.ps1"
$additional+=' "'+"$n\src\generated\HeatHazeEffect.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\Trapeze.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\AirBubble.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\AirBubbleHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\AirBubbleGenerator.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MiniatureGalaxyHolder.cpp"+'"'
& "$n\generate-native-sphere-selector.ps1"
$additional+=' "'+"$n\src\generated\SphereSelector.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\SphereSelectorHandle.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\CometRetryButton.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\LiveActor\MessageSensorHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Boss\BossAccessor.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\GalaxyNamePlateDrawer.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Boss\SkeletalFishBabyRail.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Boss\SkeletalFishBabyRailHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KameckBeamHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KameckFireBall.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KameckBeam.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KameckTurtle.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\JetTurtle.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\JetTurtleShadow.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\TakoHeiInk.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\TakoHeiInkHolder.cpp"+'"'
& "$n\generate-native-water-pressure-bullet.ps1"
$additional+=' "'+"$n\src\generated\WaterPressureBullet.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\WaterPressureBulletHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\FirePressureBullet.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\FirePressureBulletHolder.cpp"+'"'
& "$n\generate-native-clip-area-drop.ps1"
$additional+=' "'+"$n\src\generated\ClipAreaDrop.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipAreaDropHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipAreaDropLaser.cpp"+'"'
& "$n\generate-native-swing-rope.ps1"
$additional+=' "'+"$n\src\generated\SwingRope.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\SwingRopePoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\PlantLeaf.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\PlantStalk.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\ShareUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\MiiFacePartsHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\MiiFaceIconHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\MiiFaceIcon.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\MiiFaceRecipe.cpp"+'"'
& "$n\generate-native-mii-face-parts.ps1"
$additional+=' "'+"$n\src\generated\MiiFaceParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\MiiDatabase.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipFieldMapParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\FlexibleSphere.cpp"+'"'
& "$n\generate-native-assembly-block.ps1"
$additional+=' "'+"$n\src\generated\AssemblyBlock.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MercatorFixParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MercatorRailMoveParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MercatorRotateParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SimpleNormalMapObj.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\DeathArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\SwitchArea.cpp"+'"'
& "$n\generate-native-change-bgm-cube.ps1"
$additional+=' "'+"$n\src\generated\ChangeBgmCube.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\BgmProhibitArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\SoundEmitterCube.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\SoundEmitterSphere.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\QuakeEffectArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\HazeCube.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\SunLightArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\PlayerSeArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\SpinGuidanceArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\MessageArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\AstroChangeStageCube.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\CollisionArea.cpp"+'"'
& "$n\generate-native-dynamic-collision.ps1"
$additional+= ' "'+"$n\src\generated\DynamicCollisionObj.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\Caretaker.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\CollectTico.cpp"+'"'
& "$n\generate-native-stray-tico.ps1"
$additional+=' "'+"$n\src\generated\StrayTico.cpp"+'"'
& "$n\generate-native-syati.ps1"
$additional+=' "'+"$n\src\generated\Syati.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\LuigiLetter.cpp"+'"'
& "$n\generate-native-battery-info.ps1"
$additional+= ' "'+"$n\src\generated\BatteryInfo.cpp"+'"'
& "$n\generate-native-game-system-exception.ps1"
$additional+= ' "'+"$n\src\generated\GameSystemException.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\MultiEventCamera.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\PlantRailInfo.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\PlantPoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\SledRopePoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\DirectDrawUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\NormalMapBase.cpp"+'"'
& "$n\generate-native-npc-actor.ps1"
$additional+= ' "'+"$n\src\generated\NPCActor.cpp"+'"'
& "$n\generate-native-npc-util.ps1"
$additional+= ' "'+"$n\src\generated\NPCUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\BombTimerLayout.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\PrizeRing.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Boss\SkeletalFishBossFunc.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Gravity\WireGravity.cpp"+'"'
$additional+= ' "'+"$n\src\generated\SegmentGravity.cpp"+'"'
& "$n\generate-native-segment-gravity.ps1"
$additional+= ' "'+"$n\src\generated\ParallelGravity.cpp"+'"'
& "$n\generate-native-parallel-gravity.ps1"
$additional+= ' "'+"$p\Petari\src\Game\Gravity\DiskTorusGravity.cpp"+'"'
& "$n\generate-native-disk-gravity.ps1"
$additional+= ' "'+"$n\src\generated\DiskGravity.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Gravity\CubeGravity.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Gravity\ConeGravity.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Ride\SphereController.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\MapObj\KeySwitch.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\MapObj\GravityLightRoad.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\MapObj\GravityLight.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Gravity\PointGravity.cpp"+'"'
& "$n\generate-native-j3d-anm-loader.ps1"
$additional+= ' "'+"$n\src\generated\J3DAnmLoaderNative.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\MapObj\SwingLight.cpp"+'"'
$additional+= ' "'+"$p\Petari\src\Game\Util\ParabolicPath.cpp"+'"'
& "$n\generate-native-tripod-boss-accesser.ps1"
$additional+=' "'+"$n\src\generated\TripodBossAccesser.cpp"+'"'
& "$n\generate-native-wpad-pointer.ps1"
$additional+=' "'+"$n\src\generated\WPadPointer.cpp"+'"'
& "$n\generate-native-wpad-stick.ps1"
$additional+=' "'+"$n\src\generated\WPadStick.cpp"+'"'
& "$n\generate-native-wpad-button.ps1"
$additional+=' "'+"$n\src\generated\WPadButton.cpp"+'" "'+"$p\Petari\src\Game\System\WPadHolder.cpp"+'"'
foreach($inputUnit in @('WPad','WPadHVSwing','WPadInfoChecker','WPadLeaveWatcher','WPadRumble')){$additional+=' "'+"$p\Petari\src\Game\System\$inputUnit.cpp"+'"'}
$accel=Get-Content "$p\Petari\src\Game\System\WPadAcceleration.cpp" -Raw
$accel=$accel.Replace('PSVECSquareDistance(&rA, &rB)', 'PSVECSquareDistance(static_cast<const Vec*>(rA), static_cast<const Vec*>(rB))')
Set-Content "$n\src\generated\WPadAcceleration.cpp" ('#include <cstring>'+"`n"+'#include <JSystem/JMath/JMath.hpp>'+"`n"+$accel)
$additional+=' "'+"$n\src\generated\WPadAcceleration.cpp"+'"'
$additional+=' "'+"$n\src\native_kpad_input.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSystemObjHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\CinemaFrame.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\ScenarioSelectLayout.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\BackButton.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\ScenarioSelectStar.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\MessageUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TalkMessageInfo.cpp"+'"'
$additional+=' "'+"$n\src\native_pane_constructor.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSequenceFunction.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSequenceDirector.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\SequenceUtil.cpp"+'"'
& "$n\generate-native-game-sequence.ps1"
$additional+=' "'+"$n\src\generated\GameSequenceProgress.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GalaxyMoveArgument.cpp"+'"'
& "$n\generate-native-progression.ps1"
foreach($unit in @('StorySequenceExecutor','FindingLuigiEventScheduler','GalaxyCometScheduler','SaveDataHandleSequence','NANDErrorSequence')){
 $additional+=' "'+"$n\src\generated\$unit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\System\LuigiLeftSupplier.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\StageResultSequenceChecker.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\UserFile.cpp"+'"'
& "$n\generate-native-config-create.ps1"
$additional+=' "'+"$n\src\generated\ConfigDataHolder.cpp"+'"'
& "$n\generate-native-config-misc.ps1"
$additional+=' "'+"$n\src\generated\ConfigDataMisc.cpp"+'"'
& "$n\generate-native-config-mii.ps1"
$additional+=' "'+"$n\src\generated\ConfigDataMii.cpp"+'"'
foreach($unit in @('SaveIcon','SysInfoWindow','YesNoController','IconAButton')){
 $additional+=' "'+"$p\Petari\src\Game\Screen\$unit.cpp"+'"'
}
& "$n\generate-native-movie-sequence.ps1"
foreach($unit in @('MoviePlayingSequence','MovieSubtitles','LogoFader','MoviePlayerSimple','THPSimplePlayerWrapper')){
 $additional+=' "'+"$n\src\generated\$unit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\Screen\MovieSubtitlesDataTable.cpp"+'"'
& "$n\generate-native-boot-scenes.ps1"
$additional+=' "'+"$n\src\generated\TitleSequenceProduct.cpp"+'"'
$additional+=' "'+"$n\src\generated\EncouragePal60Window.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\TriggerChecker.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\HomeButtonStateNotifier.cpp"+'"'
& "$n\generate-native-pause-sequence.ps1"
foreach($unit in @('GamePauseSequence','PauseMenu','GameScenePauseControl','GameSceneScenarioOpeningCameraState','ScenarioTitle')){
 $additional+=' "'+"$n\src\generated\$unit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\System\PauseButtonCheckerInGame.cpp"+'"'
& "$n\generate-production-scene.ps1"
$additional+=' "'+"$p\Petari\src\Game\Camera\CameraPolygonCodeUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Camera\GameCameraCreator.cpp"+'"'
$additional+=' "'+"$n\src\generated\ProductionSceneRegistration.cpp"+'"'
$s=$s.Replace("$n\src\generated\CollisionSceneKernel.cpp","$n\src\generated\ProductionCollisionKernel.cpp")
$s=$s.Replace("$n\src\native_collision_scene.cpp","$n\src\generated\ProductionCollisionScene.cpp")
$additional+=' "'+"$p\Petari\src\Game\Scene\SceneObjHolder.cpp"+'"'
$additional+=' "'+"$n\src\player_event_sequence_port.cpp"+'"'
& "$n\generate-production-events.ps1"
$additional+=' "'+"$n\src\generated\ProductionEventSequencerInit.cpp"+'"'
foreach($eventUnit in @('PlayerEventDown','PlayerEventAbyss','PlayerEventFireDown','PlayerEventRaceDown','PlayerEventGhostRaceDown','PlayerEventGameOver')){
 $additional+=' "'+"$p\Petari\src\Game\Player\$eventUnit.cpp"+'"'
}
$additional+=' "'+"$n\src\generated\ErrorMessageWindow.cpp"+'"'
$additional+=' "'+"$n\src\generated\HomeButtonLayout.cpp"+'"'
foreach($flagUnit in @('GameEventFlag','GameEventFlagTable')){
 $additional+=' "'+"$p\Petari\src\Game\System\$flagUnit.cpp"+'"'
}
& "$n\generate-native-event-flags.ps1"
$additional+=' "'+"$n\src\generated\GameEventFlagChecker.cpp"+'"'
& "$n\generate-native-flag-storage.ps1"
$additional+=' "'+"$n\src\generated\GameEventFlagStorage.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\BitArray.cpp"+'"'
& "$n\generate-native-event-values.ps1"
$additional+=' "'+"$n\src\generated\GameEventValueChecker.cpp"+'"'
& "$n\generate-native-star-piece-storage.ps1"
$additional+=' "'+"$n\src\generated\StarPieceAlmsStorage.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameDataConst.cpp"+'"'

$additional=$additional.Replace("$p\Petari\src\Game\System\GameDataConst.cpp","$n\src\generated\GameDataConst.cpp")
$additional=$additional.Replace("$p\Petari\src\Game\System\GameEventFlagTable.cpp","$n\src\generated\GameEventFlagTable.cpp")
$additional+=' "'+"$n\src\generated\NativeBuiltinTables.cpp"+'"'
$additional+=' "'+"$n\src\native_runtime_strings.cpp"+'"'
& "$n\generate-native-race-manager.ps1"
$additional+=' "'+"$n\src\generated\RaceManager.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\ScenarioProgressTestRun.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\PlacedHiddenStarScenarioTable.cpp"+'"'
& "$n\generate-native-spin-storage.ps1"
$additional+=' "'+"$n\src\generated\SpinDriverPathStorage.cpp"+'"'
$additional+=' "'+"$n\src\generated\DrawUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\GameAudio\AudCameraWatcher.cpp"+'"'
& "$n\generate-native-audio-effector.ps1"
$additional+=' "'+"$n\src\generated\AudEffector.cpp"+'"'
& "$n\generate-native-sound-util.ps1"
$additional+=' "'+"$n\src\generated\SoundUtil.cpp"+'"'
foreach($unit in @('AudBgmMgr','AudBgmKeeper','AudBgm','AudBgmRhythmStrategy','AudRemixSequencer')){
 $additional+=' "'+"$p\Petari\src\Game\AudioLib\$unit.cpp"+'"'
}
foreach($unit in @('AudBgmConductor','AudEffectDirector','AudSeKeeper','AudTalkSoundData')){
 $additional+=' "'+"$p\Petari\src\Game\GameAudio\$unit.cpp"+'"'
}
& "$n\generate-native-stage-bgm.ps1"
$additional+=' "'+"$n\src\generated\AudStageBgmWrap.cpp"+'"'
& "$n\generate-native-stage-bgm-table.ps1"
$additional+=' "'+"$n\src\generated\AudStageBgmTable.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\AudSystemWrapper.cpp"+'"'
& "$n\generate-native-rhythm-system.ps1"
$additional+=' "'+"$n\src\generated\AudRhythmMeSystem.cpp"+'"'
foreach($unit in @('AudRhythmWrap','AudRhythmHolder','AudRhythmSeqParser','AudBgmTempoAdjuster','AudMeTrackCallback','AudMeTrack','AudMeChannelMgr','AudMeSeqCtrl','AudMeSeqReader','AudMeSeqParser')){
 $additional+=' "'+"$p\Petari\src\Game\RhythmLib\$unit.cpp"+'"'
}
& "$n\generate-native-bgm-setting.ps1"
$additional+=' "'+"$n\src\generated\AudBgmSetting.cpp"+'"'
& "$n\generate-native-me-object.ps1"
$additional+=' "'+"$n\src\generated\AudMeObject.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\RhythmLib\AudMeHandles.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AudioLib\AudMeNameConverter.cpp"+'"'
& "$n\generate-native-me-reader.ps1"
foreach($unit in @('JAUSoundMgr','JAISoundInfo','JAIAudience','JAIAudible','JAISoundStarter','JAIStreamDataMgr','JAISeqDataMgr','JAISeMgr','JAISeqMgr','JAISe','JAISeq','JAISoundChild','JASTrack','JAUSeqDataBlockMgr','JAUBankTable')){
 $additional+=' "'+"$p\Petari\src\JSystem\JAudio2\$unit.cpp"+'"'
}
& "$n\generate-native-audio-initializer.ps1"
$additional+=' "'+"$n\src\generated\JAUInitializer.cpp"+'"'
& "$n\generate-native-audio-params.ps1"
$additional+=' "'+"$n\src\generated\AudParams.cpp"+'"'
& "$n\generate-native-res-arc-loader.ps1"
$additional+=' "'+"$n\src\generated\JASResArcLoader.cpp"+'"'


& "$n\generate-native-stream-manager.ps1"
$additional+=' "'+"$n\src\generated\JAIStreamMgr.cpp"+'"'
& "$n\generate-native-stream-object.ps1"
$additional+=' "'+"$n\src\generated\JAIStream.cpp"+'"'
& "$n\generate-native-jas-aram-stream.ps1"
$additional+=' "'+"$n\src\generated\JASAramStream.cpp"+'"'
& "$n\generate-native-std-sound-info.ps1"
$additional+=' "'+"$n\src\generated\JAUStdSoundInfo.cpp"+'"'
& "$n\generate-native-seq-collection.ps1"
$additional+=' "'+"$n\src\generated\JAUSeqCollection.cpp"+'"'
& "$n\generate-audio-dsp-completion.ps1"
$additional+=' "'+"$n\src\generated\AudioDSPCompletionKernel.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Scene\StageResourceLoader.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NameObj\ModelChangableObjFactory.cpp"+'"'
& "$n\generate-native-map-parts-archive-name.ps1"
$additional+=' "'+"$p\Petari\src\Game\Util\MapPartsUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NameObj\NameObjFactory.cpp"+'"'

& "$n\generate-native-morph-effect-header.ps1"
$additional+=' "'+"$p\Petari\src\Game\Map\PlanetMapCreator.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\PlanetMap.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\PlanetMapWithoutHighModel.cpp"+'"'
$additional+=' "'+"$n\src\generated\OceanHomeMapCtrl.cpp"+'"'
& "$n\generate-native-ocean-controller.ps1"
$additional+=' "'+"$p\Petari\src\Game\Screen\ScreenAlphaCapture.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\NPCFunction.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\NPCDirector.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TalkDirector.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\NPCParameter.cpp"+'"'
$additional+=' "'+"$n\src\generated\TalkState.cpp"+'"'
& "$n\generate-native-talk-state.ps1"
$additional+=' "'+"$n\src\generated\DemoUtil.cpp"+'"'
$additional+=' "'+"$n\src\generated\DemoDirector.cpp"+'"'
& "$n\generate-native-demo-director.ps1"
$additional+=' "'+"$n\src\generated\DemoStartRequestHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoSimpleCastHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoCastGroupHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoStartRequestUtil.cpp"+'"'
& "$n\generate-native-demo-request-holder.ps1"
$additional+=' "'+"$n\src\generated\DemoExecutor.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoCastGroup.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoFunction.cpp"+'"'
& "$n\generate-native-demo-executor.ps1"
$additional+=' "'+"$n\src\generated\DemoTimeKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoCameraKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoSubPartKeeper.cpp"+'"'
$additional+=' "'+"$n\src\generated\DemoCameraFunction.cpp"+'"'
& "$n\generate-native-demo-camera-function.ps1"
& "$n\generate-native-demo-sound-keeper.ps1"
$additional+=' "'+"$n\src\generated\DemoSoundKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoActionKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoWipeKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoPlayerKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Demo\DemoTalkAnimCtrl.cpp"+'"'
& "$n\generate-native-demo-time-keeper.ps1"
& "$n\generate-native-message-string.ps1"
$additional+=' "'+"$n\src\generated\MessageStringKernel.cpp"+'"'
& "$n\generate-native-time-string.ps1"
$additional+=' "'+"$n\src\generated\TimeStringKernel.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\LightUtil.cpp"+'"'
& "$n\generate-model-animation-create.ps1"
$additional+=' "'+"$n\src\generated\ModelAnimationCreateKernel.cpp"+'"'
& "$n\generate-native-odh-converter.ps1"
$additional+=' "'+"$n\src\generated\OdhConverter.cpp"+'"'
& "$n\generate-native-odh-codec.ps1"
$additional+=' "'+"$n\src\generated\odh.cpp"+'"'
& "$n\generate-player-scene-queries.ps1"
$additional+=' "'+"$n\src\generated\PlayerSceneQueries.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActor.cpp"+'"'
& "$n\generate-native-mario-access.ps1"
$additional+=' "'+"$n\src\generated\MarioAccess.cpp"+'"'
$additional+=' "'+"$n\src\generated\Mario.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioState.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioModule.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioMove.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioJump.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorGravity.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioWalk.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorPad.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioDamage.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioSwim.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioWait.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSwimDamage.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioAnimator.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioConst.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFlow.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioWall.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFaint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioBlown.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioHang.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSlider.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioStep.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioBump.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioDamageParalyze.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioDamageStun.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioDamageFreeze.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioDamageCrush.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMoveSphere.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioRecovery.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFlip.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioSideStep.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFrontStep.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioStick.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioRabbit.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSukekiyo.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioClimb.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSkate.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFoo.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioWarp.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioTeresa.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioTalk.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioFpView.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorInit.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorCamera.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorMatrix.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorGameOver.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorSensor.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioEffect.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMessenger.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMagic.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorBlackHole.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorDraw.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorMorph.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorParts.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioCollision.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioEnforce.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\Mario2D.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorEye.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioSearchLight.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorRushMsg.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorRush.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorTakeMsg.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioActorSpecialDraw.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioPress.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSpecial.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSpin.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioNullBck.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSlope.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMove2D.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMove25D.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioInit.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorClap.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorPunch.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorDefensiveMsg.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorOffensiveMsg.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorHand.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioAnimationEfx.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSound.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioTask.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorShadow.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioParts.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioActorWipe.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioBee.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioSlip.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MarioLauncher.cpp"+'"'
$additional+=' "'+"$n\src\generated\MarioLauncherAttractor.cpp"+'"'
& "$n\generate-native-mario-launcher-attractor.ps1"
& "$n\generate-runtime-math-conversions.ps1"
$additional+=' "'+"$n\src\generated\RuntimeMathConversions.cpp"+'"'
& "$n\generate-runtime-matrix-transforms.ps1"
$additional+=' "'+"$n\src\generated\RuntimeMatrixTransforms.cpp"+'"'
& "$n\generate-runtime-string-scans.ps1"
$additional+=' "'+"$n\src\generated\RuntimeStringScans.cpp"+'"'
& "$n\generate-runtime-model-resources.ps1"
$additional+=' "'+"$n\src\generated\RuntimeModelResources.cpp"+'"'
& "$n\generate-actor-reset-position.ps1"
$additional+=' "'+"$n\src\generated\ActorResetPosition.cpp"+'"'
& "$n\generate-filtered-map-line-queries.ps1"
$additional+=' "'+"$n\src\generated\FilteredMapLineQueries.cpp"+'"'
& "$n\generate-triangle-attributes.ps1"
$additional+=' "'+"$n\src\generated\TriangleAttributes.cpp"+'"'
$additional+=' "'+"$n\src\generated\IsbnManager.cpp"+'"'
$additional+=' "'+"$n\src\generated\LensFlare.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapController.cpp"+'"'
& "$n\generate-native-screen-providers.ps1"
$additional+=' "'+"$n\src\generated\GalaxyMap.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapBackground.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapSelectButton.cpp"+'"'
$additional+=' "'+"$n\src\generated\PowerStarList.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapGalaxyPlain.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapGalaxyDetail.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapTitle.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapIcon.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapCometIcon.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapDomeIcon.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapMarioIcon.cpp"+'"'
$additional+=' "'+"$n\src\generated\GalaxyMapTicoIcon.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Scene\MultiSceneActor.cpp"+'"'
$additional+=' "'+"$n\src\generated\MultiSceneEffectKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Effect\AutoEffectGroup.cpp"+'"'
& "$n\generate-native-multi-scene-effects.ps1"
$additional+=' "'+"$p\Petari\src\Game\Util\ActorSwitchUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\CollisionCode.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Player\MarioMapCode.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\Air.cpp"+'"'
$additional+=' "'+"$n\src\generated\SpinDriverPathDrawer.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThread.cpp"+'"'
& "$n\generate-native-spin-driver-path-drawer.ps1"
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadWindCtrl.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadHangInfo.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadMainPoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadRadialLine.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadPart.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpiderThreadPoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\RailUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpinDriverShootPath.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpinDriver.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpinDriverCamera.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpinDriverOperateRing.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\SpinDriverUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\ActorCameraUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\CoinRotater.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\CoinBox.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\CoinSpot.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\CoinReplica.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\StarPieceGroup.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\StarPieceFollowGroup.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\StarPieceMother.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\StarPieceSpot.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\PurpleCoinHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Camera\OnlyCamera.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\PurpleCoinStarter.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipAreaHolder.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\ClipAreaShape.cpp"+'"'
$additional+=' "'+"$n\src\generated\ClipAreaMovable.cpp"+'"'
& "$n\generate-native-clip-area-movable.ps1"
$additional+=' "'+"$n\src\generated\InformationObserver.cpp"+'"'
$additional+=' "'+"$n\src\generated\MeterLayout.cpp"+'"'
$additional+=' "'+"$n\src\generated\SuddenDeathMeter.cpp"+'"'
$additional+=' "'+"$n\src\generated\SubMeterLayout.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\RumbleCalculator.cpp"+'"'
$additional+=' "'+"$n\src\generated\PlayerLeft.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\PowerStarEventKeeper.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TimeAttackEventKeeper.cpp"+'"'
$additional+=' "'+"$n\src\generated\KarikariDirector.cpp"+'"'
& "$n\generate-native-karikari-director.ps1"
& "$n\generate-native-gcapture.ps1"
$additional+=' "'+"$n\src\generated\GCapture.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\GCaptureRibbon.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\SpringValue.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Gravity\GlobalGravityObj.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Gravity\GravityCreator.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Gravity\GraviryFollower.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\CoinGroup.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\CircleCoinGroup.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\RailCoin.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\ExterminationChecker.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigBubbleGenerator.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigBubbleMoveLimitter.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigBubbleHolder.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigBubbleDrawer.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\AreaObj\BigBubbleGoalArea.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\AreaObj\BigBubbleCameraArea.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Ride\BigBubble.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Util\OctahedronBezierSurface.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Util\BezierSurface.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\SuperSpinDriver.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Map\SunshadeMapParts.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Map\SunshadeMapHolder.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigFanHolder.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\BigFan.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\native\src\generated\ElectricRailHolder.cpp"+'"'
& "$n\generate-native-electric-rail-holder.ps1"
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\ElectricRail.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\MapObj\ElectricRailMoving.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Ride\FluffWind.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Ride\Fluff.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Ride\Tamakoro.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\Ride\TamakoroTutorial.cpp"+'"'
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\native\src\generated\SphereAccelSensorController.cpp"+'"'
& "$n\generate-native-sphere-accel.ps1"
$additional+=' "'+"D:\Desktop\Games\Mario galaxy\Petari\src\Game\GameAudio\AudTamakoroBgmCtrl.cpp"+'"'
& "$n\generate-runtime-talk-queries.ps1"
$additional+=' "'+"$n\src\generated\RuntimeTalkQueries.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\Kuribo.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KuriboChief.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\KuriboMini.cpp"+'"'
foreach($enemyUnit in @('AnimScaleController','ItemGenerator','WalkerStateFindPlayer','WalkerStateBindStarPointer','WalkerStateStagger','WalkerStateWander','WalkerStateChase','WalkerStateParam','WalkerStateFunction')){
$additional+=' "'+"$p\Petari\src\Game\Enemy\$enemyUnit.cpp"+'"'
}
$additional+=' "'+"$n\src\generated\ActorStateLiveUpdates.cpp"+'"'
$additional+=' "'+"$n\src\generated\PlayerActionGuidance.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\MercatorTransformCube.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AudioLib\AudSystemVolumeController.cpp"+'"'
$remixOriginal=Get-Content "$p\Petari\src\Game\AudioLib\AudRemixMgr.cpp" -Raw
Set-Content "$n\src\generated\AudRemixMgr.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$remixOriginal)
$additional+=' "'+"$n\src\generated\AudRemixMgr.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AudioLib\AudLimitedSound.cpp"+'"'
foreach($soundUnit in @('AudSoundObject_Kawamura','AudSoundObject_Takezawa','AudSoundObject_Gohara')){
$additional+=' "'+"$p\Petari\src\Game\AudioLib\$soundUnit.cpp"+'"'
}
foreach($audioUnit in @('AudBgmVolumeController','AudTrackController','AudFader')){
$additional+=' "'+"$p\Petari\src\Game\AudioLib\$audioUnit.cpp"+'"'
}
$additional+=' "'+"$p\Petari\src\Game\AudioLib\AudAudience.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\JMath\random.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\GalaxyInfoLayoutSetter.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AreaObj\AudioEffectArea.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\AudioLib\AudUtil.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Enemy\TerritoryMover.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\IconComet.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TalkNodeCtrl.cpp"+'"'
$talkSource=Get-Content "$p\Petari\src\Game\NPC\TalkMessageCtrl.cpp" -Raw
foreach($callback in @('mEventFunc','mAnimeFunc','mKillFunc')){$talkSource=$talkSource.Replace($callback+'->operator()(nodeEvent->mUnknown) == nullptr', $callback+'->operator()(nodeEvent->mUnknown) == false')}
Set-Content "$n\src\generated\TalkMessageCtrl.cpp" $talkSource
$additional+=' "'+"$n\src\generated\TalkMessageCtrl.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TalkSupportPlayerWatcher.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\NPC\TalkTextFormer.cpp"+'"'
$balloonSource=Get-Content "$p\Petari\src\Game\NPC\TalkBalloon.cpp" -Raw
$balloonSource=[regex]::Replace($balloonSource,'\bfmin\b','talkBalloonMin')
$balloonSource=[regex]::Replace($balloonSource,'\bfmax\b','talkBalloonMax')
Set-Content "$n\src\generated\TalkBalloon.cpp" $balloonSource
$additional+=' "'+"$n\src\generated\TalkBalloon.cpp"+'"'


foreach($mapUnit in @('MapParts','GeneralMapParts')){
$mapSource=Get-Content "$p\Petari\src\Game\MapObj\$mapUnit.cpp" -Raw
$mapSource=$mapSource.Replace('_8C.setPSZeroVec();','_8C.set(0.0f, 0.0f, 0.0f);')
$mapSource=$mapSource.Replace('std::mem_fun(&MapPartsFunction::end)', '[](MapPartsFunction* function) { function->end(); }')
$mapSource=$mapSource.Replace('std::binder2nd< std::mem_fun1_t< bool, MapPartsFunction, u32 >, u32 >(std::mem_func(&MapPartsFunction::receiveMsg), msg)', '[msg](MapPartsFunction* function) { return function->receiveMsg(msg); }')
Set-Content "$n\src\generated\$mapUnit.cpp" $mapSource
$additional+=' "'+"$n\src\generated\$mapUnit.cpp"+'"'
}
$postureSource=Get-Content "$p\Petari\src\Game\MapObj\MapPartsRailPosture.cpp" -Raw
$postureSource=$postureSource.Replace('mMovePosture == 1 || isPostureTypeRailDirRailUseShadowGravity)', 'mMovePosture == 1 || isPostureTypeRailDirRailUseShadowGravity())')
Set-Content "$n\src\generated\MapPartsRailPosture.cpp" $postureSource
$additional+=' "'+"$n\src\generated\MapPartsRailPosture.cpp"+'"'
$railRotationSource=Get-Content "$p\Petari\src\Game\MapObj\MapPartsRailRotator.cpp" -Raw
$railRotationSource=$railRotationSource.Replace('_2C.setEuler(mHost->mRotation * (PI / 180.0f));', '_2C.setRotate(mHost->mRotation * (PI / 180.0f));')
Set-Content "$n\src\generated\MapPartsRailRotator.cpp" $railRotationSource
$additional+=' "'+"$n\src\generated\MapPartsRailRotator.cpp"+'"'

$rotatorSource=Get-Content "$p\Petari\src\Game\MapObj\MapPartsRotator.cpp" -Raw
$rotatorSource=$rotatorSource.Replace('mBaseHostMtx.setEuler(mHost->mRotation * (PI / 180.0f));', 'mBaseHostMtx.setRotate(mHost->mRotation * (PI / 180.0f));')
$rotatorSource=$rotatorSource.Replace('mRotateMtx.makeRotateDegree(rotateAxisDir, angle);', 'mRotateMtx.zeroTrans(); mRotateMtx.setRotate(rotateAxisDir, angle * (PI / 180.0f));')
Set-Content "$n\src\generated\MapPartsRotator.cpp" $rotatorSource
$additional+=' "'+"$n\src\generated\MapPartsRotator.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsRailMover.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsAppearController.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsRailPointPassChecker.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsFunction.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Util\PostureHolder.cpp"+'"'
$additional+=' "'+"$n\src\generated\MapPartsBaseDefaults.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsRailGuideHolder.cpp"+'"'
$guideSource=Get-Content "$p\Petari\src\Game\MapObj\MapPartsRailGuideDrawer.cpp" -Raw
$guideSource=$guideSource.Replace('std::mem_fun(&LiveActor::appear)', '[](LiveActor* actor) { actor->appear(); }').Replace('std::mem_fun(&LiveActor::kill)', '[](LiveActor* actor) { actor->kill(); }')
Set-Content "$n\src\generated\MapPartsRailGuideDrawer.cpp" $guideSource
$additional+=' "'+"$n\src\generated\MapPartsRailGuideDrawer.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\MapObj\MapPartsRailGuidePoint.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\JAudio2\JAUAudience.cpp"+'"'
$additional+=' "'+"$n\src\native_vec_scale_add.cpp"+'"'
$additional+=' "'+"$n\src\generated\SupplementalGamePadQueries.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Ride\SpherePadController.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\JUtility\JUTFont.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\J2DGraph\J2DPicture.cpp"+'"'
$paneSource=Get-Content "$p\Petari\src\JSystem\J2DGraph\J2DPane.cpp" -Raw
$paneSource='#include <macros.h>'+[Environment]::NewLine+$paneSource
Set-Content "$n\src\generated\J2DPane.cpp" $paneSource
$additional+=' "'+"$n\src\generated\J2DPane.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\J2DGraph\J2DScreen.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\J2DGraph\J2DManage.cpp"+'"'
$additional += ' "' + "$p\Petari\src\Game\Screen\FullScreenBlur.cpp" + '"'
$additional += ' "' + "D:\Desktop\Games\Mario galaxy\Petari\src\Game\Screen\ScreenBlurEffect.cpp" + '"'
$prologue=Get-Content "$p\Petari\src\Game\Demo\PrologueDirector.cpp" -Raw
$prologue=$prologue.Replace('namespace {'+[Environment]::NewLine+'    NEW_NERVE(PrologueDirectorNrvWait','namespace NativePrologueNerves {'+[Environment]::NewLine+'    NEW_NERVE(PrologueDirectorNrvWait')
$prologue=$prologue.Replace('GET_NERVE_ANON(', 'GET_NERVE_DIRECT(NativePrologueNerves, ')
Set-Content "$n\src\generated\PrologueDirector.cpp" $prologue
$additional += ' "' + "$n\src\generated\PrologueDirector.cpp" + '"'
$letter=Get-Content "$p\Petari\src\Game\Screen\PrologueLetter.cpp" -Raw
$letter=$letter.Replace('namespace {','namespace NativePrologueLetterNerves {').Replace('GET_NERVE_ANON(', 'GET_NERVE_DIRECT(NativePrologueLetterNerves, ')
Set-Content "$n\src\generated\PrologueLetter.cpp" $letter
$additional += ' "' + "$n\src\generated\PrologueLetter.cpp" + '"'
$additional += ' "' + "D:\Desktop\Games\Mario galaxy\Petari\src\Game\Screen\ProloguePictureBook.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\ChipHolder.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\ChipCounter.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\ChipGroup.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\CollectCounter.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\ChipBase.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\BlueChip.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\MapObj\YellowChip.cpp" + '"'
$comet=Get-Content "$p\Petari\src\Game\System\GalaxyCometState.cpp" -Raw
$comet=$comet.Replace('namespace {','namespace NativeGalaxyCometState {').Replace('GET_NERVE_ANON(', 'GET_NERVE_DIRECT(NativeGalaxyCometState, ')
Set-Content "$n\src\generated\GalaxyCometState.cpp" $comet
$additional += ' "' + "$n\src\generated\GalaxyCometState.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\System\NANDManager.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\System\NANDManagerThread.cpp" + '"'
$additional += ' "' + "$n\src\native_nand_port.cpp" + '"'
$additional += ' "' + "$n\src\native_save_chunk_holder.cpp" + '"'
& "$n\generate-native-sysconfig.ps1"
$additional += ' "' + "$n\src\generated\SysConfigFile.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\System\SaveDataFileAccessor.cpp" + '"'
& "$n\generate-native-save-handler.ps1"
$additional += ' "' + "$n\src\generated\SaveDataHandler.cpp" + '"'
& "$n\generate-native-save-banner.ps1"
$additional += ' "' + "$n\src\generated\SaveDataBannerCreator.cpp" + '"'
$additional += ' "' + "$n\src\native_nand_banner.cpp" + '"'
$additional += ' "' + "$p\Petari\src\JSystem\JSupport\JSUFileStream.cpp" + '"'
& "$n\generate-native-stream-file-table.ps1"
$additional += ' "' + "$n\src\generated\JAUStreamFileTable.cpp" + '"'
$additional += ' "' + "$p\Petari\src\JSystem\JAudio2\JASVoiceBank.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\AudioLib\AudSeStrategy.cpp" + '"'
& "$n\generate-native-me-player.ps1"
$additional += ' "' + "$n\src\generated\AudMePlayer.cpp" + '"'
$additional += ' "' + "$n\src\native_me_params.cpp" + '"'
& "$n\generate-native-chord-info.ps1"
$additional += ' "' + "$n\src\generated\AudChordInfo.cpp" + '" "' + "$n\src\native_chord_table.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\Screen\PlayerMissLeft.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\Screen\StaffRoll.cpp" + '"'
$additional += ' "' + "$p\Petari\src\Game\Demo\DemoPadRumbler.cpp" + '"'
& "$n\generate-native-draw-util.ps1"
& "$n\generate-native-language.ps1"
& "$n\generate-native-game-system.ps1"
& "$n\generate-stream-lifecycle-header.ps1"
& "$n\generate-native-content-header.ps1"
$additional+=' "'+"$n\src\generated\BinaryDataContentAccessor.cpp"+'"'
$additional+=' "'+"$n\src\generated\NativeMemoryStreams.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\JSupport\JSUOutputStream.cpp"+'"'
$additional+=' "'+"$p\Petari\src\JSystem\JSupport\JSUInputStream.cpp"+'"'
& "$n\generate-native-player-status.ps1"
$additional+=' "'+"$n\src\generated\GameDataPlayerStatus.cpp"+'"'
& "$n\generate-native-galaxy-chunk-codec.ps1"
$additional+=' "'+"$n\src\generated\GameDataGalaxyStorage.cpp"+'"'
& "$n\generate-native-game-data-holder.ps1"
& "$n\generate-native-builtin-tables.ps1"
$additional+=' "'+"$n\src\generated\GameDataHolder.cpp"+'"'
foreach($saveUnit in @('GameDataFunction','GameDataTemporaryInGalaxy')){
 $additional+=' "'+"$p\Petari\src\Game\System\$saveUnit.cpp"+'"'
}
& "$n\generate-native-error-watcher.ps1"
$additional+=' "'+"$n\src\generated\GameSystemErrorWatcher.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSystemResetAndPowerProcess.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSystemDimmingWatcher.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSystemFrameControl.cpp"+'"'
$additional+=' "'+"$n\src\generated\GameSystem.cpp"+'"'
& "$n\generate-native-stationed-loader.ps1"
$additional+=' "'+"$n\src\generated\GameSystemStationedArchiveLoader.cpp"+'"'
$additional+=' "'+"$n\src\generated\StationedArchiveLoader.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\StationedFileInfo.cpp"+'"'
& "$n\generate-native-archive-collector.ps1"
$additional+=' "'+"$n\src\generated\NameObjArchiveListCollector.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\System\GameSystemFunction.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Scene\SceneDataInitializer.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Scene\ScenePlayingResult.cpp"+'"'
$additional+=' "'+"$n\src\generated\Language.cpp"+'" "'+"$n\src\native_game_language.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Screen\ScreenPreserver.cpp"+'"'
$pointerDirector=Get-Content "$p\Petari\src\Game\Screen\StarPointerDirector.cpp" -Raw
$pointerDirector=$pointerDirector.Replace('mTransHolder->mProjMtx.setInline(MR::getCameraProjectionMtx());','mTransHolder->mProjMtx.set(MR::getCameraProjectionMtx());')
Set-Content "$n\src\generated\StarPointerDirector.cpp" $pointerDirector
$additional+=' "'+"$n\src\generated\StarPointerDirector.cpp"+'"'
$pointerTarget=Get-Content "$p\Petari\src\Game\Screen\StarPointerTarget.cpp" -Raw
$pointerTarget=$pointerTarget.Replace('*pos.toTVec3()', 'TVec3f(pos.x,pos.y,pos.z)')
$pointerTarget=$pointerTarget.Replace('mtx.setInline(MR::getStarPointerProjMtx());', 'mtx.set(MR::getStarPointerProjMtx());')
$pointerTarget=$pointerTarget.Replace('rPointerPos.squared(screenPos)', '((rPointerPos.x-screenPos.x)*(rPointerPos.x-screenPos.x)+(rPointerPos.y-screenPos.y)*(rPointerPos.y-screenPos.y))')
$pointerTarget=$pointerTarget.Replace('pos.squared(rPos)', '((pos.x-rPos.x)*(pos.x-rPos.x)+(pos.y-rPos.y)*(pos.y-rPos.y))')
Set-Content "$n\src\generated\StarPointerTarget.cpp" $pointerTarget
$additional+=' "'+"$n\src\generated\StarPointerTarget.cpp"+'"'
foreach($pointerUnit in @('StarPointerController','StarPointerLayout','StarPointerGuidance','StarPointerBlur','StarPointerCommandStream')){$additional+=' "'+"$p\Petari\src\Game\Screen\$pointerUnit.cpp"+'"'}
$pointerUtil=Get-Content "$p\Petari\src\Game\Util\StarPointerUtil.cpp" -Raw
$pointerUtil=$pointerUtil.Replace('posA.subInline(controller->mScreenVel)', '(posA-controller->mScreenVel)').Replace('screenVel.scale(1.0f / rad2D);', 'screenVel=screenVel*(1.0f / rad2D);').Replace('screenVel.scale(0.726f / speed);', 'screenVel=screenVel*(0.726f / speed);').Replace('screenPos -= targetPos;', 'screenPos.sub(targetPos);')
Set-Content "$n\src\generated\StarPointerUtil.cpp" $pointerUtil
$additional+=' "'+"$n\src\generated\StarPointerUtil.cpp"+'" "'+"$p\Petari\src\Game\System\StarPointerOnOffController.cpp"+'"'
& "$n\generate-camera-input-queries.ps1"
$additional+=' "'+"$n\src\generated\CameraInputQueries.cpp"+'"'
& "$n\generate-camera-map-queries.ps1"
$additional+=' "'+"$n\src\generated\CameraMapQueries.cpp"+'"'
& "$n\generate-collision-line-query.ps1"
$additional+=' "'+"$n\src\generated\CollisionLineQuery.cpp"+'"'
& "$n\generate-camera-collision-math.ps1"
$additional+=' "'+"$n\src\generated\CameraCollisionMath.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\LiveActor\RailRider.cpp"+'"'
$additional+=' "'+"$p\Petari\src\Game\Map\BezierRail.cpp"+'"'
$additional+=' "'+"$n\src\generated\RailPart.cpp"+'"'
& "$n\generate-rail-part.ps1"
foreach($audioUnit in @('CSSoundNameConverter','AudSpeakerWrap','AudMicWrap')){
 $additional+=' "'+"$p\Petari\src\Game\AudioLib\$audioUnit.cpp"+'"'
}
$mixSource=Get-Content "$p\Petari\src\Game\Speaker\SpkMixingBuffer.cpp" -Raw
Set-Content "$n\src\generated\SpkMixingBuffer.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$mixSource)
$additional+=' "'+"$n\src\generated\SpkMixingBuffer.cpp"+'"'
$spkSource=Get-Content "$p\Petari\src\Game\Speaker\SpkSystem.cpp" -Raw
Set-Content "$n\src\generated\SpkSystem.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$spkSource)
$additional+=' "'+"$n\src\generated\SpkSystem.cpp"+'"'
foreach($speakerUnit in @('SpkSound','SpkSpeakerCtrl')){
 $additional+=' "'+"$p\Petari\src\Game\Speaker\$speakerUnit.cpp"+'"'
}
& "$n\generate-native-speaker-data.ps1"
$additional+=' "'+"$n\src\generated\SpkData.cpp"+'" "'+"$n\src\generated\SpkTableConstructor.cpp"+'"'
& "$n\generate-native-speaker-wave.ps1"
$additional+=' "'+"$n\src\generated\SpkWave.cpp"+'"'
& "$n\generate-pointer-sound-routing.ps1"
$additional+=' "'+"$n\src\generated\PointerSoundRouting.cpp"+'"'
$additional+=' "'+"$n\src\native_keyboard_kpad.cpp"+'" "'+"$n\src\native_kpad_input.cpp"+'" "'+"$n\src\native_wpad_port.cpp"+'" -luser32'
# This candidate layers a broad native system/camera set over the actor-link
# response. Drop translation units already present there to avoid duplicate symbols.
$knownUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach($m in [regex]::Matches($s,'"([^"\r\n]+\.(?:cpp|obj))"')){[void]$knownUnits.Add([IO.Path]::GetFileNameWithoutExtension($m.Groups[1].Value))}
$duplicateAdditionalUnits=[System.Collections.Generic.List[string]]::new()
foreach($m in [regex]::Matches($additional,'"([^"\r\n]+\.(?:cpp|obj))"')){
 $unit=[IO.Path]::GetFileNameWithoutExtension($m.Groups[1].Value)
 if(!$knownUnits.Add($unit)){$additional=$additional.Replace($m.Value,'');$duplicateAdditionalUnits.Add($unit)}
}
$s='-x c++ '+$s
$s=[regex]::Replace($s,'"[^"\r\n]*\\CollisionCodeMetadata\.(cpp|obj)"','')
$s=$s.Replace(' -o ',$additional+' -o ')
$s=$s.Replace("$n\build\actor-runtime-link-probe.dll","$out\runtime-link-probe.dll")
# Reset language for precompiled object inputs (C++ mode must not parse COFF bytes).
$s=[regex]::Replace($s,'("[^"\r\n]*\.obj")','-x none $1 -x c++')
Set-Content "$out\runtime-link.rsp" $s
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
if($missingCandidateObjects.Count){Write-Warning ('Candidate compile objects absent; omitted for explicit-provider/link diagnostics: '+(($missingCandidateObjects|Select-Object -Unique)-join ', '))}
if($duplicateAdditionalUnits.Count){Write-Host ('De-duplicated candidate translation units: '+$duplicateAdditionalUnits.Count)}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ "@$out\runtime-link.rsp" *> "$out\runtime-link.log"
$code=$LASTEXITCODE
Get-Content "$out\runtime-link.log" -Tail 12
exit $code
























































































































































































































































































































































































































































































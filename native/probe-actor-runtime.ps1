param([switch]$CandidateCamera,[string]$RunName='actor-runtime-probe',[string[]]$OnlyUnits=@())
# Native compatibility headers in native/include are checked in/generated already;
# these generators write into the protected include tree on Windows.
# Original full actor layout only; does not use the diagnostic LiveActor implementation.
$ErrorActionPreference='Continue'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\$RunName"
$cameraInclude=@()
if($CandidateCamera){
    & "$n\generate-camera-pointer-candidate.ps1"
    $out="$n\build\camera-pointer-candidate\actor-runtime-probe"
    $cameraInclude=@("-I$n\build\camera-pointer-candidate\include")
}
New-Item -ItemType Directory -Force $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\$RunName-cache"
$paths=@('nw4r\math\math_types','nw4r\math\math_triangular','nw4r\ut\ut_LinkList','nw4r\ut\ut_CharWriter','nw4r\ut\ut_TagProcessorBase','nw4r\ut\ut_TextWriterBase','nw4r\lyt\lyt_animation','nw4r\lyt\lyt_arcResourceAccessor','nw4r\lyt\lyt_common','nw4r\lyt\lyt_drawInfo','nw4r\lyt\lyt_init','nw4r\lyt\lyt_material','nw4r\lyt\lyt_pane','nw4r\lyt\lyt_picture','nw4r\lyt\lyt_resourceAccessor','nw4r\lyt\lyt_texMap','nw4r\lyt\lyt_textBox','nw4r\lyt\lyt_window','nw4r\lyt\lyt_layout','nw4r\lyt\lyt_group','nw4r\lyt\lyt_bounding','System\MessageHolder','nw4r\ut\ut_Font','nw4r\ut\ut_ResFontBase','nw4r\ut\ut_CharStrmReader','System\GameSystemFontHolder','System\HeapMemoryWatcher','Util\MemoryUtil','JSystem\JKernel\JKRExpHeap','JSystem\JKernel\JKRSolidHeap','JSystem\JKernel\JKRUnitHeap','JSystem\JKernel\JKRHeapDispatch','System\FunctionAsyncExecutor','Util\SystemUtil','System\ResourceHolderManager','Util\LayoutUtil','Screen\CustomTagProcessor','Screen\ReplaceTagProcessor','Screen\MessageTagSkipTagProcessor','Screen\LayoutPaneCtrl','Screen\PaneEffectKeeper','Screen\StarPointerTarget','Screen\LayoutActor','Screen\LayoutActorFlag','Screen\LayoutManager','Screen\LayoutCoreUtil','System\LayoutHolder','Animation\LayoutAnmPlayer','Screen\TimeLimitLayout','Scene\ScenarioSelectScene','Scene\PlayTimerScene','Util\ValueControl','System\GalaxyNameSortTable','LiveActor\LiveActorFlag','Map\WaterInfo','Map\StageSwitch','Map\SwitchWatcher','Map\SwitchWatcherHolder','Map\ActorAppearSwitchListener','NameObj\NameObjListExecutor','NameObj\NameObjExecuteHolder','System\GameSystemSceneController','System\NerveExecutor','LiveActor\Spine','LiveActor\Nerve','LiveActor\ActorStateKeeper','LiveActor\ActorStateBase','Util\NerveUtil','Util\JMapIdInfo','Util\SceneUtil','Util\FileUtil','System\FileLoader','System\FileLoaderThread','System\FileRipper','System\OSThreadWrapper','System\FileHolder','System\ArchiveHolder','Scene\PlacementStateChecker','Scene\StageDataHolder','Scene\StageResourceLoader','Scene\PlacementInfoOrdered','Scene\StageFileLoader','System\GalaxyStatusAccessor','System\ScenarioDataParser','Util\MathUtilScalar','Scene\Scene','Scene\SceneNameObjListExecutor','Scene\SceneNameObjMovementController','Scene\SceneFunction','Scene\SceneFactory','Scene\GameScene','Scene\IntermissionScene','Scene\LogoScene','NameObj\NameObjCategoryList','System\DrawBufferHolder','System\DrawBufferGroup','System\DrawBufferExecuter','System\DrawBuffer','Util\SwitchEventFunctorListener','Util\ObjUtil','Util\JMapUtil','Util\LiveActorUtil','Util\ActorSensorUtil','LiveActor\LiveActor','LiveActor\AllLiveActorGroup','LiveActor\LiveActorGroup','NameObj\NameObjGroup','NameObj\NameObjFinder','NameObj\NameObjAdaptor','NameObj\MovementOnOffGroupHolder','LiveActor\ClippingDirector','LiveActor\ClippingActorHolder','LiveActor\ClippingActorInfo','LiveActor\ClippingGroupHolder','LiveActor\ClippingJudge','LiveActor\ViewGroupCtrl','LiveActor\ActorLightCtrl','LiveActor\ActorPadAndCameraCtrl','LiveActor\ModelManager','LiveActor\ActorAnimKeeper','Animation\BvaPlayer','LiveActor\DisplayListMaker','System\ResourceHolder','System\ResourceInfo','LiveActor\MaterialCtrl','Animation\MaterialAnmBuffer','Animation\BckCtrl','Animation\AnmPlayer','Animation\BtpPlayer','Animation\BtkPlayer','Animation\BrkPlayer','Animation\BpkPlayer','JSystem\J3DGraphAnimator\J3DAnimation','JSystem\J3DGraphAnimator\J3DModelData','JSystem\J3DGraphLoader\J3DModelLoader','JSystem\J3DGraphLoader\J3DAnmLoader','JSystem\J3DGraphBase\J3DMaterial','JSystem\J3DGraphAnimator\J3DMaterialAttach','JSystem\J3DGraphAnimator\J3DMaterialAnm','JSystem\J3DGraphAnimator\J3DJoint','JSystem\J3DGraphAnimator\J3DJointTree','JSystem\J3DGraphLoader\J3DJointFactory','JSystem\J3DGraphBase\J3DDrawBuffer','JSystem\J3DGraphBase\J3DSys','JSystem\J3DGraphBase\J3DStruct','JSystem\J3DGraphAnimator\J3DMtxBuffer','JSystem\J3DGraphBase\J3DTransformPort','JSystem\J3DGraphBase\J3DShape','JSystem\J3DGraphBase\J3DShapeDraw','JSystem\J3DGraphBase\J3DShapeMtx','JSystem\J3DGraphAnimator\J3DShapeTable','JSystem\J3DGraphLoader\J3DShapeFactory','JSystem\J3DGraphLoader\J3DMaterialFactory','JSystem\J3DGraphLoader\J3DMaterialFactory_v21','JSystem\J3DGraphBase\J3DMatBlock','JSystem\J3DGraphBase\J3DTevs','JSystem\J3DGraphBase\J3DPacket','JSystem\J3DGraphBase\J3DVertex','JSystem\J3DGraphAnimator\J3DModel','JSystem\J3DGraphAnimator\J3DSkinDeform','JSystem\J3DGraphAnimator\J3DCluster','JSystem\JMath\JMATrigonometric','JSystem\JMath\JMath','JSystem\JUtility\JUTNameTab','JSystem\J3DGraphBase\J3DGD','JSystem\JKernel\JKRFileLoader','JSystem\JKernel\JKRMemArchive','JSystem\JKernel\JKRArchivePri','JSystem\JKernel\JKRArchivePub','JSystem\JKernel\JKRFileFinder','JSystem\JKernel\JKRDvdFile','JSystem\JKernel\JKRDvdRipper','JSystem\JKernel\JKRDvdArchive','JSystem\JKernel\JKRDisposer','JSystem\JSupport\JSUList','JSystem\JKernel\JKRThread','JSystem\JKernel\JKRDecomp','System\RenderMode','JSystem\J3DGraphLoader\J3DModelLoaderCalcSize','Animation\XanimePlayer','Animation\XanimeCore','Animation\XanimeResource')
$paths += @('System\GameSystemFunction','System\GameSystemObjHolder','System\GameDataFunction','System\GameSequenceFunction','System\Language','System\AudSystemWrapper','Screen\ScenarioSelectLayout','Screen\CinemaFrame','Util\MessageUtil','NameObj\NameObjFactory','Scene\SceneDataInitializer','Util\StringUtil','Util\SequenceUtil','Util\SoundUtil','Scene\ScenePlayingResult','System\GameDataConst','NPC\TalkDirector')
$paths += @('LiveActor\ActiveActorList','LiveActor\AnimationRandomPlayer')
$paths += @('Gravity\ConeGravity','Gravity\CubeGravity','Gravity\DiskGravity','Gravity\DiskTorusGravity','Gravity\GlobalGravityObj','Gravity\GraviryFollower','Gravity\GravityCreator','Gravity\GravityInfo','Gravity\ParallelGravity','Gravity\PlanetGravity','Gravity\PlanetGravityManager','Gravity\PointGravity','Gravity\SegmentGravity','Gravity\WireGravity','Map\GravityDust')
$paths += @('Screen\MovieSubtitles','Screen\MovieSubtitlesDataTable','Screen\MoviePlayerSimple','Screen\THPSimplePlayerWrapper','Screen\THPDraw')
$paths += @('System\GameSystem','System\GameSystemErrorWatcher','System\GameSystemResetAndPowerProcess','System\GameSystemStationedArchiveLoader','System\SaveDataHandleSequence','System\GameSequenceDirector','System\GameSequenceProgress','System\LuigiLeftSupplier','System\WPad','System\WPadHolder','System\WPadButton','System\WPadPointer','System\WPadRumble','System\WPadLeaveWatcher','Util\StarPointerUtil','Util\DrawUtil','Util\LightUtil','Util\PlayerUtil','System\PlacedHiddenStarScenarioTable','Screen\ScreenAlphaCapture')
$paths += @('Util\GamePadUtil')
$paths += @('System\SaveDataHandler','System\NANDErrorSequence','System\SaveDataBannerCreator','System\SaveDataFileAccessor','Screen\SysInfoWindow','Screen\SaveIcon')
$paths += @('Screen\MoviePlayingSequence','Map\ScenarioSelectStar','Screen\GamePauseSequence','Scene\GameScenePauseControl','Map\PlanetMapCreator','Map\PlanetMap','Map\PlanetMapWithoutHighModel','Map\LavaSunPlanet','MapObj\TypicalDoor','MapObj\QuestionBoxGalleryObj','MapObj\FlexibleSphere','MapObj\SimpleClipPartsObj','MapObj\JetTurtle','MapObj\ManholeCover','MapObj\RotateMoveObj','MapObj\BeamGoRoundPlanet','MapObj\BumpAppearPlanet','MapObj\HatchWaterPlanet','MapObj\LavaJamboSunPlanet','MapObj\MarblePlanet','MapObj\PeachCastleGardenPlanet','MapObj\SandCapsuleInsidePlanet','MapObj\SnowCapsulePlanet','MapObj\ReverseGravityRoomPlanet','MapObj\WormEatenPlanet','System\PauseButtonCheckerInGame','MapObj\LavaGeyser','MapObj\LavaProminence','MapObj\LavaProminenceTriple','MapObj\LavaFloater','MapObj\LavaHomeSeesaw','MapObj\LavaHomeVolcanoFlow','MapObj\LavaBreakColumn','Map\LavaShellTower','MapObj\LavaGalaxyParts','MapObj\LavaSteam','MapObj\LavaStrangeRock','MapObj\LavaBallRisingPlanetLava','Map\Halo','Map\OceanHomeMapCtrl','MapObj\ExterminationChecker','Screen\LensFlare','Screen\PauseMenu','Screen\StaffRoll','Screen\StarPointerLayout','AudioLib\AudParams','AudioLib\AudBgmMgr','AudioLib\AudBgmKeeper','AudioLib\AudBgmRhythmStrategy','AudioLib\AudBgmVolumeController','GameAudio\AudBgmConductor','MapObj\LavaHomeSeesawRotator')
$paths += @('Map\Air','MapObj\AirBubble','MapObj\AirBubbleGenerator','MapObj\AstroCore','MapObj\AstroDome','MapObj\AstroDomeAsteroid','MapObj\AstroDomeBlueStar','MapObj\AstroDomeComet','MapObj\AstroDomeDemoAstroGalaxy','Demo\AstroDomeDemoStarter','MapObj\AstroDomeGalaxySelector','MapObj\AstroDomeSky','MapObj\AstroMapBoard','MapObj\AstroMapObj','MapObj\AstroOverlookObj')
$paths += @('MapObj\MapObjActor','MapObj\MapObjActorInitInfo','MapObj\AstroDomeCameraController','MapObj\AstroMapObjFunction','Screen\GalaxyNamePlate','Screen\GalaxyNamePlateDrawer','Screen\GalaxyConfirmLayout','Screen\GalaxySelectBackButton','Screen\GalaxySelectInfo','MapObj\MiniatureGalaxyHolder','Map\SphereSelector','MapObj\SpinDriverShootPath','MapObj\SpinDriverPathDrawer','Demo\AstroDemoFunction','MapObj\GCaptureRibbon')
$paths += @('MapObj\AirBubbleHolder','MapObj\SimpleMapObj','Screen\GalaxyMapController','MapObj\GCapture')
$paths += @('MapObj\MorphItemObjNeo')
$paths += @('Map\HitInfo','Map\BezierRail','Map\KoopaBattleMapStair','Util\MultiEventCamera')
$paths += @('NPC\PowerStarEventKeeper','Map\CollisionCategorizedKeeper')
# Include the original NPC actor implementations as first-class campaign
# providers. The link-only game integration reaches NPC constructors through
# PlanetMapCreator/NameObjFactory; compiling these original units closes that
# real construction path rather than replacing it with a host-side stub.
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\NPC" -File -Filter '*.cpp' | ForEach-Object { 'NPC\' + $_.BaseName })
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\AreaObj" -File -Filter '*.cpp' | ForEach-Object { 'AreaObj\' + $_.BaseName })
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\Enemy" -File -Filter '*.cpp' | ForEach-Object { 'Enemy\' + $_.BaseName })
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\Boss" -File -Filter '*.cpp' | ForEach-Object { 'Boss\' + $_.BaseName })
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\MapObj" -File -Filter '*.cpp' | ForEach-Object { 'MapObj\' + $_.BaseName })
$paths += @('Ride\Trapeze','Map\RailPart')
$paths += @('Ride\PlantStalk','Ride\PlantLeaf')
$paths += @('Ride\SurfRayTutorial')
$paths += @('Ride\TamakoroTutorial','Ride\SphereAccelSensorController','Ride\SpherePadController','Ride\SphereController','GameAudio\AudTamakoroBgmCtrl')
$paths += @('Ride\SledRopePoint')
$paths += @('Map\KoopaBattleMapPlanet','Map\KoopaBattleMapDamagePlate')
$paths += @('LiveActor\SpotMarkLight','Effect\SpinPullParticleCallBack')
$paths += @('Util\OctahedronBezierSurface','Util\BezierSurface')
$paths += @('Map\CollisionCode','Map\KCollision')
$paths += @('Screen\IconComet')
$paths += @('Util\QuaternionSdkHelpers')
$paths += @('Player\RushEndInfo')
$paths += @('Map\SpaceInner','LiveActor\MirrorReflectionModel')
$paths += @('Map\FileSelectCameraController','Map\FileSelectSky','Map\FileSelectItem','Map\FileSelectIconID','Map\FileSelectEffect','Map\FileSelectModel')
$paths += @('Screen\FileSelectInfo','Screen\FileSelectButton','Screen\FileSelectNumber','Screen\BrosButton','Screen\Manual2P','Screen\MiiConfirmIcon','Screen\MiiSelect','Screen\MiiSelectIcon')
$paths += @('LiveActor\VolumeModelDrawer','LiveActor\SimpleJ3DModelDrawer')
$paths += @('Map\RailGraph','Map\RailGraphNode','Map\RailGraphEdge','Map\RailGraphIter','Util\RailGraphUtil')
$paths += @('Util\ActorStateUtil')
$paths += @('Map\GroundChecker','Util\GeometryBindUtil','Util\PostureHolder')
$paths += @('Demo\DemoCtrlBase','Demo\DemoPositionController')
$paths += @('Demo\ReturnDemoRailMove')
$paths += @('Demo\DemoParamCommonDataTable')
$paths += @('Scene\SceneObjHolder')
$paths += @('Util\MapPartsUtil','Util\TriangleFilter','Util\DirectDrawUtil')
$paths += @('Screen\GalaxyMap','Screen\GalaxyMapBackground','Screen\GalaxyMapSelectButton','Screen\PowerStarList','Screen\BackButton','Util\SpringValue')
$paths += @('Screen\GalaxyMapCometIcon','Screen\GalaxyMapDomeIcon','Screen\GalaxyMapGalaxyDetail','Screen\GalaxyMapGalaxyPlain','Screen\GalaxyMapIcon','Screen\GalaxyMapMarioIcon','Screen\GalaxyMapTicoIcon','Screen\GalaxyMapTitle','Map\RaceManager','Screen\OdhConverter','Screen\odh')
$paths += @('Player\MarioAccess')
$paths += @('Player\MarioActor')
$paths += @('Player\TornadoMario','Player\FireMarioBall')
$paths += @('Player\JetTurtleShadow','Player\MarineSnow')
$paths += @('Util\JointUtil','Util\JointRumbler','Player\MarioActorSpecialDraw')
$paths += @('Player\MarioActorBlackHole','Player\MarioActorClap','Player\MarioActorDefensiveMsg','Player\MarioActorDraw','Player\MarioActorEye','Player\MarioActorGameOver','Player\MarioActorGravity','Player\MarioActorHand','Player\MarioActorInit','Player\MarioActorMatrix','Player\MarioActorMorph','Player\MarioActorOffensiveMsg','Player\MarioActorPad','Player\MarioActorParts','Player\MarioActorPunch','Player\MarioActorRush','Player\MarioActorRushMsg','Player\MarioActorSensor','Player\MarioActorShadow','Player\MarioActorWipe')
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\Player" -File -Filter 'Mario*.cpp' | ForEach-Object { 'Player\' + $_.BaseName })
$paths += @('Util\ModelUtil','Player\MarioActorTakeMsg','Util\DemoUtil','Demo\DemoDirector','Demo\DemoStartRequestUtil','Demo\DemoFunction','Demo\DemoExecutor','Demo\DemoExecutorFunction','Demo\DemoStartRequestHolder','Demo\DemoCastGroupHolder','Demo\DemoSimpleCastHolder','Demo\DemoSubPartKeeper','Demo\DemoTimeKeeper')
$paths += @('Util\CameraUtil','Util\ActorCameraUtil','Util\ActorSwitchUtil','Util\TalkUtil','Demo\DemoCameraFunction')
$paths += @('Demo\DemoActionKeeper','Demo\DemoCameraKeeper','Demo\DemoCastGroup','Demo\DemoPlayerKeeper','Demo\DemoSoundKeeper','Demo\DemoTalkAnimCtrl','Demo\DemoWipeKeeper')
$paths += @(Get-ChildItem -LiteralPath "$p\Petari\src\Game\Camera" -Filter '*.cpp' | ForEach-Object { 'Camera\' + $_.BaseName })
$paths += @('Player\MarioActorCamera','Player\MarioCollision','Util\MapUtil','Util\NPCUtil','NPC\NPCActor','NPC\NPCFunction','NPC\NPCSupportRail','NPC\TalkBalloon','NPC\TalkMessageCtrl','NPC\TalkMessageInfo','NPC\TalkNodeCtrl','NPC\TalkState','Screen\StarPointerDirector','Scene\GameSceneScenarioOpeningCameraState','AreaObj\CameraRepulsiveArea')
$paths += @('NameObj\NameObjArchiveListCollector','Scene\MultiSceneActor','Scene\MultiSceneEffectKeeper')
$paths += @('Player\GhostPlayer','Demo\DemoKoopaJrShip','Ride\Creeper','Ride\Fluff','Ride\FluffWind','Ride\Tamakoro','Ride\JumpBranch','Ride\Plant','Ride\Pole','Ride\SurfRay','Ride\SwingRope','Ride\SpaceCocoon','Map\WaterRoad','Ride\SlingShooter','Ride\BigBubble','Map\Butterfly','Map\SphereSelectorHandle','Demo\PrologueDirector','Map\KoopaBattleMapPlate','Map\KoopaBattleMapCoinPlate','Effect\AstroEffectObj','Map\Sky','Map\FishGroup','Map\SeaGull','Map\WaterPlant','Effect\SimpleEffectObj','Effect\RandomEffectObj','Effect\EffectObjGravityDust','Map\GroupSwitchWatcher','Map\RaceRail','Map\TimerSwitch','Map\SwitchSynchronizer','Screen\MovieStarter','Demo\DemoCastSubGroup','Demo\StarReturnDemoStarter','Demo\GrandStarReturnDemoStarter','Demo\ScenarioStarter','Map\FileSelector')
$paths += @('Util\MathUtil','Util\MtxUtil','Util\ActorMovementUtil','Util\RailUtil','Player\DrawAdaptor','Player\ModelHolder','Player\MatrixControl')
$paths += @('Util\GravityUtil')
$paths += @('Gravity\GlobalGravityObj')
$paths += @('AudioLib\AudEffector','AudioLib\AudMeNameConverter','AudioLib\AudMicWrap','AudioLib\AudRemixMgr','AudioLib\AudRemixSequencer','AudioLib\AudSpeakerWrap','AudioLib\CSSoundNameConverter','GameAudio\AudSeKeeper','GameAudio\AudTalkSoundData','GameAudio\AudEffectDirector','RhythmLib\AudMeObject','JSystem\JAudio2\JAUSoundMgr')
$paths += @('AudioLib\AudBgm','AudioLib\AudBgmSetting','AudioLib\AudFader','AudioLib\AudAudience','AudioLib\AudSeStrategy','AudioLib\AudLimitedSound','AudioLib\AudSystemVolumeController','AudioLib\AudTrackController','AudioLib\AudSoundObject','AudioLib\AudSoundObject_Takezawa','AudioLib\AudSoundObject_Kawamura','AudioLib\AudSoundObject_Gohara','GameAudio\AudStageBgmWrap','GameAudio\AudStageBgmTable','RhythmLib\AudRhythmWrap','RhythmLib\AudRhythmMeSystem','RhythmLib\AudBgmTempoAdjuster','RhythmLib\AudChordInfo','RhythmLib\AudMePlayer','JSystem\JAudio2\JASAiCtrl','JSystem\JAudio2\JASDSPInterface','JSystem\JAudio2\JAUInitializer','JSystem\JAudio2\JAUStreamFileTable','JSystem\JAudio2\JAIStreamDataMgr','JSystem\JAudio2\JASVoiceBank','JSystem\JAudio2\JAUSeqDataBlockMgr','JSystem\JAudio2\JAUSeqCollection','JSystem\JAudio2\JASResArcLoader','JSystem\JAudio2\JAUStdSoundInfo','JSystem\JAudio2\JASAramStream','JSystem\JAudio2\JAIAudible','JSystem\JAudio2\JAIAudience','JSystem\JAudio2\JAISeq','JSystem\JAudio2\JAISeqMgr','JSystem\JAudio2\JAIStream','JSystem\JAudio2\JAIStreamMgr','JSystem\JAudio2\JAISoundInfo','JSystem\JAudio2\JAISoundStarter','JSystem\JSupport\JSUInputStream','JSystem\JSupport\JSUFileStream','JSystem\JSupport\JSUMemoryStream','Screen\TitleSequenceProduct')
$paths += @('Map\CollisionParts','Util\ParabolicPath','Util\FootPrint','Screen\GalaxyInfoLayoutSetter')
$paths += @('System\GameDataHolder','System\GameDataGalaxyStorage','System\GameDataPlayerStatus','System\GameDataTemporaryInGalaxy','System\GameEventFlagChecker','System\GameEventValueChecker','System\GameEventFlag','System\GameEventFlagStorage','System\GameEventFlagTable','System\ScenarioProgressTestRun','System\SpinDriverPathStorage','System\StarPieceAlmsStorage','System\BinaryDataChunkHolder','System\UserFile','System\ConfigDataHolder','System\ConfigDataMisc','System\ConfigDataMii','System\SysConfigFile','System\GalaxyCometScheduler','System\GalaxyCometState','System\StageResultSequenceChecker','System\GalaxyMoveArgument','System\StorySequenceExecutor','System\FindingLuigiEventScheduler','System\StationedFileInfo','System\NANDManager','System\NANDManagerThread','Map\FileSelectFunc','Map\SunshadeMapParts','Map\SunshadeMapHolder','LiveActor\RailRider','NWC24\LuigiMailDirector','NWC24\NWC24Messenger','Screen\LogoFader','Screen\IsbnManager','Screen\ScreenPreserver','Screen\HomeButtonLayout','NameObj\ModelChangableObjFactory','JSystem\J2DGraph\J2DPicture','JSystem\J2DGraph\J2DPane','JSystem\J2DGraph\J2DScreen','JSystem\J2DGraph\J2DManage','JSystem\JMath\random','JSystem\JSupport\JSUOutputStream','Util\BitArray')
$paths += @('System\GameSystemException','System\GameSystemFrameControl','System\GameSystemDimmingWatcher','System\HomeButtonStateNotifier','System\GameSystemErrorWatcher','System\GameSystemStationedArchiveLoader','System\StationedArchiveLoader','System\WPadInfoChecker','Util\TriggerChecker','Screen\ErrorMessageWindow','Screen\BatteryInfo','Screen\LuigiLetter','Screen\PlayerMissLeft')
$paths += @('System\StarPointerOnOffController','System\WPadAcceleration','System\WPadHVSwing','System\WPadStick','Screen\StarPointerController','Screen\StarPointerGuidance','Screen\StarPointerBlur','Screen\StarPointerCommandStream','Demo\DemoPadRumbler')
$paths += @('Screen\MeterLayout','Screen\BombTimerLayout','Screen\SubMeterLayout','Screen\PlayerLeft','Screen\SuddenDeathMeter','Screen\FullnessMeter','Screen\MarioMeter','Screen\PlayerMissLeft')
$paths += @('Screen\PlayerActionGuidance')
$paths += @('Screen\ProloguePictureBook','Screen\PrologueLetter')
$paths += @('Screen\SurfingGuidance','Screen\PeachLetter')
$paths += @('Screen\ScenarioTitle')
$paths += @('Screen\PictureBookLayout')
$paths += @('Screen\PictureBookCloseButton')
$paths += @('Screen\StageResultInformer')
# Reuse the checked-in host-adapted providers; only regenerate the adapters edited in this probe.
& "$n\generate-native-system-adapters.ps1"
& "$n\generate-native-game-data-holder.ps1"
& "$n\generate-native-game-event-flag-checker.ps1"
& "$n\generate-native-story-sequence-executor.ps1"
& "$n\generate-native-boot-ui.ps1"
& "$n\generate-native-nwc24-messenger.ps1"
& "$n\generate-player-util-runtime.ps1"
& "$n\generate-galaxy-name-plate-runtime.ps1"
& "$n\generate-sphere-selector-runtime.ps1"
& "$n\generate-galaxy-map-controller-runtime.ps1"
& "$n\generate-galaxy-map-runtime.ps1"
& "$n\generate-galaxy-map-select-button-runtime.ps1"
& "$n\generate-native-odh-converter.ps1"
& "$n\generate-race-manager-runtime.ps1"
& "$n\generate-odh-codec-runtime.ps1"
& "$n\generate-mario-actor-host-runtime.ps1"
& "$n\generate-water-plant-host-compat.ps1"
& "$n\generate-hit-info-runtime.ps1"
& "$n\generate-collision-categorized-keeper-runtime.ps1"
& "$n\generate-rail-part-runtime.ps1"
& "$n\generate-lava-floater-runtime.ps1"
& "$n\generate-kinopio-astro-runtime.ps1"
& "$n\generate-npc-host-compat.ps1"
& "$n\generate-area-host-compat.ps1"
& "$n\generate-enemy-host-compat.ps1"
& "$n\generate-mapobj-host-compat.ps1"
& "$n\generate-map-ride-host-compat.ps1"
& "$n\generate-native-spin-driver-path-drawer.ps1"
& "$n\generate-kcollision-host-compat.ps1"
& "$n\generate-wpad-acceleration-host-compat.ps1"
& "$n\generate-sub-meter-layout-host.ps1"
& "$n\generate-gravity-util-host-compat.ps1"
& "$n\generate-audparams-host-compat.ps1"
& "$n\generate-audio-dsp-host.ps1"
& "$n\generate-native-j3d-anm-loader.ps1"
& "$n\generate-scene-object-holder-runtime.ps1"
$nativeSaveProviders=@(& (Join-Path $n 'native-save-providers.ps1'))
$nativeSaveSources=@{};foreach($provider in $nativeSaveProviders){$nativeSaveSources[$provider.Unit]=$provider.Source}
$results=@()
$seenPaths=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach($path in $paths){
if(!$seenPaths.Add($path)){continue}
$unit=Split-Path $path -Leaf;$source="$p\Petari\src\Game\$path.cpp"
if($OnlyUnits.Count -gt 0 -and $unit -notin $OnlyUnits){continue}
if($path.StartsWith('JSystem\') -or $path.StartsWith('nw4r\')){$source="$p\Petari\src\$path.cpp"}
if($unit -in @('LayoutHolder','MessageTagSkipTagProcessor','ut_CharWriter','CustomTagProcessor','math_triangular','lyt_arcResourceAccessor','lyt_material','lyt_texMap','lyt_textBox','MessageHolder','GameSystemFontHolder','HeapMemoryWatcher','MemoryUtil','JKRExpHeap','JKRSolidHeap','JKRDisposer','JKRUnitHeap','JKRHeapDispatch','FunctionAsyncExecutor','ResourceHolderManager','LayoutUtil','PaneEffectKeeper','StarPointerTarget','LayoutActor','LayoutCoreUtil','GalaxyNameSortTable','ScenarioDataParser','JKRDecomp','JKRThread','JKRArchivePub','JKRMemArchive','JKRDvdFile','JKRDvdRipper','JKRDvdArchive','FileRipper','OSThreadWrapper','FileHolder','ArchiveHolder','FileUtil','FileLoader','StageDataHolder','MathUtilScalar','ObjUtil','JMapUtil','LiveActorUtil','DisplayListMaker','ResourceHolder','ResourceInfo','AnmPlayer','J3DAnimation','XanimeResource','J3DModelLoader','J3DAnmLoader','J3DModelLoaderCalcSize','J3DJoint','J3DDrawBuffer','J3DMtxBuffer','J3DTransformPort','J3DShape','J3DShapeDraw','J3DShapeFactory','J3DMaterialFactory','J3DMaterialFactory_v21','J3DPacket','J3DModel','JMath','J3DGD','NameObjAdaptor','SwitchWatcherHolder','DrawBufferHolder','DrawBufferGroup','DrawBufferExecuter','SceneNameObjListExecutor','SceneNameObjMovementController','IntermissionScene','LogoScene')){$source="$n\src\generated\$unit.cpp"}
if($unit -eq 'StringUtil'){$source="$n\src\generated\StringUtil.cpp"}
if($unit -eq 'J3DAnmLoader'){$source="$n\src\generated\J3DAnmLoaderNative.cpp"}
if($unit -eq 'StarPointerUtil'){$source="$n\src\generated\StarPointerUtil.cpp"}
if($unit -eq 'SubMeterLayout'){$source="$n\src\generated\SubMeterLayoutHost.cpp"}
if($unit -in @('SeaGull','FileSelector','SurfRay','SwingRope','RaceRail','EffectObjGravityDust')){$source="$n\src\generated\MapRideCompat\$unit.cpp"}
if($unit -eq 'SpinDriverPathDrawer'){$source="$n\src\generated\SpinDriverPathDrawer.cpp"}
if($unit -eq 'GameSystemStationedArchiveLoader'){$source="$n\src\generated\GameSystemStationedArchiveLoader.cpp"}
if($unit -eq 'MarioActor'){$source="$n\src\generated\MarioActor.cpp"}
if($unit -eq 'MarioActorSpecialDraw'){$source="$n\src\generated\MarioActorSpecialDraw.cpp"}
if($unit -eq 'DemoUtil'){$source="$n\src\generated\DemoUtilRuntime.cpp"}
if($unit -eq 'MapUtil'){$source="$n\src\generated\MapUtilRuntime.cpp"}
if($unit -eq 'MarioActorCamera'){$source="$n\src\generated\MarioActorCameraRuntime.cpp"}
if($unit -eq 'MarioCollision'){$source="$n\src\generated\MarioCollisionRuntime.cpp"}
if($unit -eq 'TalkBalloon'){$source="$n\src\generated\TalkBalloonRuntime.cpp"}
if($unit -eq 'TalkMessageCtrl'){$source="$n\src\generated\TalkMessageCtrlRuntime.cpp"}
if($unit -eq 'TalkState'){$source="$n\src\generated\TalkStateRuntime.cpp"}
if($unit -eq 'MultiSceneEffectKeeper'){$source="$n\src\generated\MultiSceneEffectKeeperRuntime.cpp"}
if($unit -eq 'GameDataHolder'){$source="$n\src\generated\GameDataHolderRuntime.cpp"}
if($unit -eq 'GameEventFlagChecker'){$source="$n\src\generated\GameEventFlagCheckerRuntime.cpp"}
if($unit -eq 'StorySequenceExecutor'){$source="$n\src\generated\StorySequenceExecutorRuntime.cpp"}
if($unit -eq 'LogoFader'){$source="$n\src\generated\LogoFaderRuntime.cpp"}
if($unit -eq 'IsbnManager'){$source="$n\src\generated\IsbnManagerRuntime.cpp"}
if($unit -eq 'NWC24Messenger'){$source="$n\src\generated\NWC24MessengerRuntime.cpp"}
if($unit -eq 'GalaxyNamePlate'){$source="$n\src\generated\GalaxyNamePlateRuntime.cpp"}
if($unit -eq 'SphereSelector'){$source="$n\src\generated\SphereSelectorRuntime.cpp"}
if($unit -eq 'GalaxyMapController'){$source="$n\src\generated\GalaxyMapControllerRuntime.cpp"}
if($unit -eq 'GalaxyMap'){$source="$n\src\generated\GalaxyMapRuntime.cpp"}
if($unit -eq 'GalaxyMapSelectButton'){$source="$n\src\generated\GalaxyMapSelectButtonRuntime.cpp"}
if($unit -eq 'OdhConverter'){$source="$n\src\generated\OdhConverter.cpp"}
if($unit -eq 'RaceManager'){$source="$n\src\generated\RaceManagerRuntime.cpp"}
if($unit -eq 'odh'){$source="$n\src\generated\ODHCodecRuntime.cpp"}
if($unit -in @('MarioActorEye','MarioActorGravity','MarioActorParts','MarioActorRushMsg','MarioActorSensor','Mario','MarioBee','MarioDamage','MarioEffect','MarioJump','MarioMove','MarioShadow','MarioSideStep','MarioStick','MarioSwim','MarioTeresa','MarioWall','MarioWarp')){$source="$n\src\generated\${unit}Runtime.cpp"}
if($unit -eq 'HitInfo'){$source="$n\src\generated\HitInfoRuntime.cpp"}
if($unit -eq 'CollisionCategorizedKeeper'){$source="$n\src\generated\CollisionCategorizedKeeperRuntime.cpp"}
if($unit -eq 'RailPart'){$source="$n\src\generated\RailPartRuntime.cpp"}
if($unit -eq 'LavaFloater'){$source="$n\src\generated\LavaFloaterRuntime.cpp"}
if($unit -eq 'KinopioAstro'){$source="$n\src\generated\KinopioAstroRuntime.cpp"}
if($unit -in @('Rosetta','RosettaReading','StrayTico','Syati','Tico','TicoRail')){$source="$n\src\generated\NPCCompat\$unit.cpp"}
if($unit -in @('AreaObj','AreaObjContainer','ChangeBgmCube')){$source="$n\src\generated\AreaObjCompat\$unit.cpp"}
if($unit -in @('BallBeamer','BegomanBase','CannonShellBase','HammerHeadPackun','Jellyfish','JellyfishElectric','KirairaChain','KoopaJrShip','Meramera','Metbo','Mogucchi','OtaRock','Unizo')){$source="$n\src\generated\EnemyCompat\$unit.cpp"}
if($unit -in @('BossKameckStateBattle','DinoPackunTailNode','Dodoryu','TripodBossAccesser','TripodBossKillerGeneraterCircle')){$source="$n\src\generated\EnemyCompat\$unit.cpp"}
if($unit -in @('AssemblyBlock','BrightObj','ClipAreaDrop','ClipAreaMovable','CocoNut','CrystalCage','CrystalCageMoving','DriftWood','EarthenPipe','ElectricBall','ElectricRailHolder','FirePressureRadiate','GeneralMapParts','GreenCaterpillarBig','IceVolcanoUpDownPlane','MapPartsBreaker','MapPartsRailGuideDrawer','MapPartsRailPosture','MapPartsRailRotator','MapPartsRotator','MarioLauncherAttractor','MiniatureGalaxy','Note','OceanWaveFloater','PhantomShipBoxFloater','PlantGroup','PowerStar','PunchBox','QuestionCoin','Rock','RockCreator','Sandstorm','Shellfish','SnowFloor','SnowFloorTile','SpringJetWater','SpringWaterFloaterSpot','TrampleStar','TreasureBoxCracked','WaterfallCaveCover','WaterPressureBullet')){$source="$n\src\generated\MapObjCompat\$unit.cpp"}
if($unit -eq 'AudParams'){$source="$n\src\generated\AudParamsRuntime.cpp"}
if($unit -eq 'SceneObjHolder'){$source="$n\src\generated\SceneObjHolderRuntime.cpp"}
if($unit -eq 'MathUtil'){$source="$n\src\generated\MathUtilRuntime.cpp"}
if($unit -eq 'MtxUtil'){$source="$n\src\generated\MtxUtilRuntime.cpp"}
if($unit -eq 'ActorMovementUtil'){$source="$n\src\generated\ActorMovementUtilRuntime.cpp"}
if($unit -eq 'NPCActor'){$source="$n\src\generated\NPCActorRuntime.cpp"}
if($unit -eq 'NPCUtil'){$source="$n\src\generated\NPCUtilRuntime.cpp"}
if($unit -eq 'StarPointerDirector'){$source="$n\src\generated\StarPointerDirectorRuntime.cpp"}
if($unit -eq 'DemoDirector'){$source="$n\src\generated\DemoDirectorRuntime.cpp"}
if($unit -eq 'CameraManGame'){$source="$n\src\generated\CameraManGameRuntime.cpp"}
if($unit -eq 'CamTranslatorAnim'){$source="$n\src\generated\CamTranslatorAnimRuntime.cpp"}
if($unit -eq 'CameraUtil'){$source="$n\src\generated\CameraUtilRuntime.cpp"}
if($unit -eq 'DemoCameraFunction'){$source="$n\src\generated\DemoCameraFunction.cpp"}
if($unit -eq 'DemoExecutor'){$source="$n\src\generated\DemoExecutorRuntime.cpp"}
if($unit -eq 'DemoStartRequestHolder'){$source="$n\src\generated\DemoStartRequestHolderRuntime.cpp"}
if($unit -eq 'WaterPlant'){$source="$n\src\generated\WaterPlantRuntime.cpp"}
if($unit -eq 'THPSimplePlayerWrapper'){$source="$n\src\generated\THPSimplePlayerWrapper.cpp"}
if($unit -eq 'THPDraw'){$source="$p\Petari\src\Game\Screen\THPDraw.c"}
if($unit -eq 'QuaternionSdkHelpers'){$source="$n\src\generated\QuaternionSdkHelpers.cpp"}
if($nativeSaveSources.ContainsKey($unit)){$source=$nativeSaveSources[$unit]}
if(!(Test-Path -LiteralPath $source)){throw "Missing original actor source: $source"}; $unitArgs=@()
$unitInclude=@()
if($unit -eq 'JASAiCtrl'){$source="$n\src\generated\AudioAiCtrlHost.cpp";$unitArgs+=@('-include','cstdarg','-D__va_list=va_list','-DTRUE=1','-DFALSE=0');$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'THPSimplePlayerWrapper'){$unitArgs+=@('-DTRUE=1','-DFALSE=0');$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'THPDraw'){$unitArgs+=@('-x','c++')}
if($unit -eq 'KCollision'){$source="$n\src\generated\KCollisionQueriesHost.cpp"}
if($unit -eq 'QuaternionSdkHelpers'){$source="$n\src\generated\QuaternionSdkHelpers.cpp"}
if($unit -eq 'JASDSPInterface'){$source="$n\src\generated\AudioDSPInterfaceHost.cpp";$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'JASAramStream'){$source="$n\src\generated\JASAramStream.cpp"}
if($unit -eq 'JASResArcLoader'){$source="$n\src\generated\JASResArcLoader.cpp"}
if($unit -eq 'JSUMemoryStream'){$source="$n\src\generated\NativeMemoryStreams.cpp"}
if($unit -eq 'JAIStream'){$source="$n\src\generated\JAIStream.cpp"}
if($unit -eq 'JAUInitializer'){$source="$n\src\generated\JAUInitializer.cpp"}
if($unit -eq 'JAUStreamFileTable'){$source="$n\src\generated\JAUStreamFileTable.cpp"}
if($unit -eq 'PlayerEvent'){$unitInclude+=@('-I',"$n\build\original-include")}
if($unit -in @('GameSceneScenarioOpeningCameraState','DemoKoopaJrShip','PrologueDirector','ProloguePictureBook','PrologueLetter','SurfingGuidance','PeachLetter','ScenarioTitle','PictureBookLayout','PictureBookCloseButton','StageResultInformer','FileSelectCameraController','FileSelectSky','FileSelectItem','FileSelectIconID','FileSelectEffect','FileSelectModel','FileSelectInfo','FileSelectButton','FileSelectNumber','BrosButton','Manual2P','MiiConfirmIcon','MiiSelect','MiiSelectIcon','DrawAdaptor','ModelHolder','MatrixControl','IconComet','ErrorMessageWindow','BatteryInfo')){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -eq 'FileSelectItem'){$unitArgs+=@('-include',"$n\include-runtime\file_select_host_compat.hpp")}
if($unit -eq 'GameSystemException'){$unitArgs+=@('-include','cstdarg','-D__va_list=va_list')}
if($unit -in @('AudStageBgmWrap','AudStageBgmTable')){$unitArgs+=@('-Wno-c++11-narrowing')}
if($unit -eq 'AudBgmSetting'){$unitArgs+=@('-Wno-c++11-narrowing')}
if($unit -eq 'AudRhythmMeSystem'){$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'Butterfly'){$unitArgs+=@('-include',"$n\include-runtime\butterfly_host_compat.hpp")}
if($unit -eq 'CameraAnim'){$unitArgs+=@('-include',"$p\Petari\libs\JSystem\include\JSystem\JMath\JMath.hpp")}
if($unit -eq 'WPadAcceleration'){$unitArgs+=@('-include',"$p\Petari\libs\JSystem\include\JSystem\JMath\JMath.hpp")}
if($unit -eq 'J2DPane'){$unitArgs+=@('-include',"$n\include\Native\AngleConversion.hpp")}
if($unit -eq 'WPadAcceleration'){$source="$n\src\generated\WPadCompat\WPadAcceleration.cpp"}
if($unit -eq 'GravityUtil'){$source="$n\src\generated\GravityCompat\GravityUtil.cpp"}
if($unit -in @('ClipAreaDrop','ClipAreaMovable')){$unitArgs+=@('-include',"$n\include-runtime\mapobj_host_compat.hpp")}
if($unit -in @('Rock','RockCreator','WaterfallCaveCover','PlantStalk','PlantLeaf')){$unitArgs+=@('-include',"$p\Petari\include\math_types.hpp")}
if($unit -in @('GameSystemErrorWatcher','GameSystemStationedArchiveLoader','SaveDataHandleSequence','GameSequenceProgress','StorySequenceExecutor','GalaxyCometState','LogoFader','HomeButtonLayout','NWC24Messenger','TitleSequenceProduct','SaveDataHandler','NANDErrorSequence','SaveDataBannerCreator','MapObjActor','AstroDomeCameraController','GalaxyNamePlate','GalaxyConfirmLayout','GalaxySelectBackButton','GalaxySelectInfo','SphereSelector','GCapture','GalaxyMapController','GalaxyMap','GalaxyMapSelectButton','PowerStarList','GalaxyMapGalaxyDetail','GalaxyMapTitle','GamePauseSequence','GameScenePauseControl','LensFlare','MoviePlayerSimple')){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -in @('HammerHeadPackun','Mogucchi','MogucchiHill','Petari','Pukupuku','TrampleStar','FireMarioBall')){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -in @('Dodoryu','DodoryuDemo','DodoryuStateLv1','DodoryuStateLv2','DodoryuStateWait','SkeletalFishBaby','SkeletalFishBoss','SkeletalFishGuard')){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -in @('SaveDataHandler','SaveDataBannerCreator','AudEffector','AudRemixMgr','AudMeObject')){$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'PlayerActionGuidance'){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -eq 'SurfRayTutorial'){$unitArgs+=@('-include',"$n\include-runtime\native_no_weak.h")}
if($unit -eq 'MarineSnow'){$unitArgs+=@('-Wno-c++11-narrowing');$unitInclude+=@('-include',"$n\include\Native\HeapPlacement.hpp")}
if($unit -eq 'GameSystem'){$unitArgs+='-Dmain=GameSystemMain'}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @cameraInclude @unitInclude @unitArgs -DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER -std=c++17 -O2 -c -ferror-limit=12 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\RVLFaceLib\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" -include "$n\include-runtime\nw4r\ut\Color.h" $source -o "$out\$unit.obj" *> "$out\$unit.log"
$results += [pscustomobject]@{Unit=$unit;ExitCode=$LASTEXITCODE;Log="$out\$unit.log"}
}
$results | ConvertTo-Json | Set-Content "$out\results.json"
$results | Format-Table -AutoSize
foreach($failure in $results | Where-Object ExitCode -ne 0){Get-Content $failure.Log -Tail 12}
if(@($results | Where-Object ExitCode -ne 0).Count){exit 1}
































































































































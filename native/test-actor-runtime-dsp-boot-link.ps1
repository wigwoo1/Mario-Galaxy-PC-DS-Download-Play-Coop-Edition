param([string]$ResponsePath,[switch]$DiagnosticSkipArcFinderFree,[switch]$DiagnosticPressA,[string]$BckCtrlObject,[string]$ResourceAccessorObject,[string]$ParticleResourceHolderObject,[string]$TextureObjectLodKernelObject,[ValidateRange(1,100000)][int]$DiagnosticCaptureFrame=240,[string]$ZigExePath,[switch]$DiagnosticHiddenWindow,[switch]$DiagnosticHardware)

$ErrorActionPreference='Stop'
$native=$PSScriptRoot
$root=Split-Path $native -Parent
& (Join-Path $native 'generate-jpa-emitter-header.ps1')
& (Join-Path $native 'generate-native-sound-id-header.ps1')
$zig=if($ZigExePath){$ZigExePath}else{Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe'}
if(!(Test-Path -LiteralPath $zig)){throw "Zig compiler not found: $zig"}
if(!$ResponsePath){
 $ResponsePath=Get-ChildItem (Join-Path $native 'build') -Filter 'actor-runtime-link-*.rsp' |
  Where-Object Name -NotLike '*with-dspboot*' |
  Sort-Object LastWriteTime -Descending |
  Select-Object -First 1 -ExpandProperty FullName
}
if(!$ResponsePath -or !(Test-Path -LiteralPath $ResponsePath)){throw 'Pass a completed actor-runtime link response file or run the actor-link audit first.'}

# Validate the original input before replacing the current runtime manifest or
# spending time compiling. This script's generated response is not valid input.
$inputResponse=Get-Content -LiteralPath $ResponsePath -Raw
$soundIdRefreshUnits=@(
 @{Unit='AudBgm';Source=(Join-Path $root 'Petari\src\Game\AudioLib\AudBgm.cpp')},
 @{Unit='AudBgmSetting';Source=(Join-Path $native 'src\generated\AudBgmSetting.cpp')},
 @{Unit='AudSeStrategy';Source=(Join-Path $root 'Petari\src\Game\AudioLib\AudSeStrategy.cpp')},
 @{Unit='JAUSoundMgr';Source=(Join-Path $root 'Petari\src\JSystem\JAudio2\JAUSoundMgr.cpp')},
 @{Unit='JAUStreamFileTable';Source=(Join-Path $native 'src\generated\JAUStreamFileTable.cpp')},
 @{Unit='JAUSeqCollection';Source=(Join-Path $native 'src\generated\JAUSeqCollection.cpp')},
 @{Unit='JAUStdSoundInfo';Source=(Join-Path $native 'src\generated\JAUStdSoundInfo.cpp')}
)
foreach($provider in $soundIdRefreshUnits){
 if([regex]::Matches($inputResponse,'"[^"\r\n]*\\'+$provider.Unit+'\.obj"').Count-ne 1){
  throw "Use an original actor-runtime-link-*.rsp from native/build, not the generated *with-dspboot.rsp. Expected one original $($provider.Unit).obj in: $ResponsePath"
 }
}

& (Join-Path $native 'generate-npc-item-table.ps1')
& (Join-Path $native 'generate-bas-resource.ps1')
$tmp=Join-Path $env:TEMP 'galaxy-dspboot-link-verification'
$runtimeManifestPath=Join-Path $tmp 'latest-native-runtime.json'
if(Test-Path -LiteralPath $runtimeManifestPath){Remove-Item -LiteralPath $runtimeManifestPath}

$cache=Join-Path $tmp 'zig-cache'
New-Item -ItemType Directory -Force -Path $tmp,$cache | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=$cache
& (Join-Path $native 'build-rfl-original.ps1') -DiagnosticBootTrace
if($LASTEXITCODE){throw 'Current native FaceLib storage/ABI providers failed to compile.'}
$objects=@(
 @{Source=(Join-Path $native 'src\dsp_boot_hle.cpp');Name='dsp_boot_hle.obj'},
 @{Source=(Join-Path $native 'src\dsp_rom_boot_hle.cpp');Name='dsp_rom_boot_hle.obj'},
 @{Source=(Join-Path $native 'src\dsp_audio_command_hle.cpp');Name='dsp_audio_command_hle.obj'},
 @{Source=(Join-Path $native 'src\remix_sequence_data.cpp');Name='remix_sequence_data.obj'},
 @{Source=(Join-Path $native 'src\generated\DspUcodeImage.cpp');Name='DspUcodeImage.obj'},
 @{Source=(Join-Path $native 'src\native_game_entry_host.cpp');Name='native_game_entry_host.obj'}
)
$dspBootTraceSource=Join-Path $tmp 'dsp_boot_hle-boot-traced.cpp'
$dspBootTraceText=Get-Content -LiteralPath (Join-Path $native 'src\dsp_boot_hle.cpp') -Raw
$dspBootTraceText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $dspBootTraceText
foreach($stage in @(
 @('    DspInitWork();','DspBoot work init begin','DspBoot work init returned'),
 @('    DSPInit();','DspBoot DSP init begin','DspBoot DSP init returned'),
 @('    const auto* image = Native::originalDspUcodeImage();','DspBoot image lookup begin','DspBoot image lookup complete'),
 @('    for (std::size_t i = 0; i < sizeof(upload) / sizeof(upload[0]); ++i) {','DspBoot upload loop begin','DspBoot upload loop end'),
 @('    if (!receive(kDspInit) || !receive(kZeldaHandshake)) return;','DspBoot ucode handshake begin','DspBoot ucode handshake returned'),
 @('    Dsp_Running_Start();','DspBoot running transition begin','DspBoot running transition returned')
)){
 $needle=$stage[0]
 if(!$dspBootTraceText.Contains($needle)){continue}
 $dspBootTraceText=$dspBootTraceText.Replace($needle,'    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("'+$stage[2]+'");')
}
$oldReceive='bool receive(u32 expected) {' + "`n" + '    return DSPCheckMailFromDSP() != 0 && DSPReadMailFromDSP() == expected;' + "`n" + '}'
$newReceive=@'
bool receive(u32 expected) {
    galaxyNativeBootTrace("DspBoot receive mailbox check begin");
    const auto available = DSPCheckMailFromDSP();
    { char line[120]; std::snprintf(line,sizeof(line),"DspBoot receive mailbox check returned available=%u",available); galaxyNativeBootTrace(line); }
    if (!available) return false;
    galaxyNativeBootTrace("DspBoot receive mailbox read begin");
    const auto actual = DSPReadMailFromDSP();
    { char line[160]; std::snprintf(line,sizeof(line),"DspBoot receive mailbox read returned actual=%08X expected=%08X",actual,expected); galaxyNativeBootTrace(line); }
    return actual == expected;
}
'@
if($dspBootTraceText.Contains($oldReceive)){$dspBootTraceText=$dspBootTraceText.Replace($oldReceive,$newReceive.TrimEnd())}
$dspBootTraceText=$dspBootTraceText.Replace('bool send(u32 word) {','bool send(u32 word) {' + "`n" + '    galaxyNativeBootTrace("DspBoot mail send begin");')
$oldRomHandshake='    if (!nativeDspBeginRomTask(image, imageBytes) || !receive(kRomReady))' + "`n" + '        return;'
$newRomHandshake=@'
    galaxyNativeBootTrace("DspBoot native ROM task begin");
    const bool nativeRomTaskStarted = nativeDspBeginRomTask(image, imageBytes);
    { char line[120]; std::snprintf(line,sizeof(line),"DspBoot native ROM task returned started=%d",int(nativeRomTaskStarted)); galaxyNativeBootTrace(line); }
    if (!nativeRomTaskStarted) return;
    if (!receive(kRomReady)) {
        galaxyNativeBootTrace("DspBoot ROM ready mail absent");
        return;
    }
    galaxyNativeBootTrace("DspBoot ROM ready mail accepted");
'@
if($dspBootTraceText.Contains($oldRomHandshake)){$dspBootTraceText=$dspBootTraceText.Replace($oldRomHandshake,$newRomHandshake.TrimEnd())}else{throw 'DspBoot ROM-task handshake trace replacement anchor missing.'}
[IO.File]::WriteAllText($dspBootTraceSource,$dspBootTraceText,[Text.UTF8Encoding]::new($false))
$objects[0].Source=$dspBootTraceSource
$common=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',
 "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include",
 '-include','revolution/types.h','-c')
$common+="-I$root\Petari\libs\nw4r\include"
foreach($item in $objects){
 $item.Path=Join-Path $tmp $item.Name
 & $zig c++ @common $item.Source '-o' $item.Path
 if($LASTEXITCODE){throw "DSP boot link provider failed to compile: $($item.Source)"}
}
$entryObject=Join-Path $tmp 'GameSystemEntry.obj'
$entryCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',
 "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include",
 "-I$root\Petari\libs\nw4r\include", '-include',"$native\include-runtime\native_no_weak.h",
 '-include','revolution/types.h','-include',"$native\include-runtime\revolution\os.h")
& $zig c++ @entryCommon -c (Join-Path $native 'src\generated\GameSystemEntry.cpp') '-o' $entryObject
if($LASTEXITCODE){throw 'Updated native GameSystemEntry provider failed to compile.'}
$heapObject=Join-Path $tmp 'HeapMemoryWatcher.obj'
$heapCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_BOOT_TRACE','-target','x86_64-windows-gnu',
 "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include",
 "-I$root\Petari\libs\nw4r\include", '-include',"$native\include-runtime\native_no_weak.h",
 '-include','revolution/types.h','-include',"$native\include-runtime\revolution\os.h")
& $zig c++ @heapCommon -c (Join-Path $native 'src\generated\HeapMemoryWatcher.cpp') '-o' $heapObject
if($LASTEXITCODE){throw 'Boot-traced native HeapMemoryWatcher provider failed to compile.'}
$gameSystemObject=Join-Path $tmp 'GameSystem.obj'
$gameSystemSource=Join-Path $tmp 'GameSystem-boot-traced.cpp'
$d3dGameHostTemp=Join-Path $tmp 'native_d3d_game_host.hpp'
$gameSystemText=Get-Content -LiteralPath (Join-Path $native 'src\generated\GameSystem.cpp') -Raw
$gameSystemText="#include <stdexcept>`n#include <string>`n"+$gameSystemText
$gameSystemText='#include "native_framebuffer_probe.hpp"'+"`n"+$gameSystemText
$sceneDrawAnchor='    mSceneController->drawScene();'
if(!$gameSystemText.Contains($sceneDrawAnchor)){throw 'Scene-ready frame probe anchor missing.'}
$gameSystemText=$gameSystemText.Replace($sceneDrawAnchor,'    if((!std::getenv("GALAXY_FRAMEBUFFER_PROBE_AFTER_PROFILE_START") || std::strcmp(mSceneController->mCurrSceneControlInfo.mStage,"FileSelect")!=0) && mSceneController->mSceneInitializeState==SceneInitializeState_End && mSceneController->getCurrentSceneForExecute()==mSceneController->mScene && std::strcmp(mSceneController->mCurrSceneControlInfo.mScene,"Game")==0) NativeFramebufferProbe::armSceneFrame();'+"`n"+$sceneDrawAnchor)
foreach($overlayCall in @(
 @('    mSequenceDirector->draw();','sequence'),
 @('    mObjHolder->drawStarPointer();','star-pointer'),
 @('    mObjHolder->drawBeforeEndRender();','before-end-render'),
 @('        mSystemWipeHolder->draw();','system-wipe'),
 @('    mErrorWatcher->draw();','error-watcher'),
 @('    mHomeButtonLayout->draw();','home-button'),
 @('    SingletonHolder< GameSystemResetAndPowerProcess >::get()->draw();','reset-power')
)){
 if(!$gameSystemText.Contains($overlayCall[0])){throw "System overlay diagnostic anchor missing: $($overlayCall[1])"}
 $gameSystemText=$gameSystemText.Replace($overlayCall[0],'    try {'+$overlayCall[0]+' } catch(const std::exception& error) { throw std::runtime_error(std::string("System overlay '+$overlayCall[1]+': ")+error.what()); }')
}
$d3dGameHostInclude=$d3dGameHostTemp.Replace('\','/')
$originalEntryStart=$gameSystemText.IndexOf('extern "C" void nativeOriginalGameMain(void) {')
if($originalEntryStart -lt 0){throw 'Generated GameSystem entrypoint block was not found.'}
$originalEntryEnd=$gameSystemText.IndexOf("`n`nGameSystem::GameSystem()",$originalEntryStart)
if($originalEntryEnd -lt 0){throw 'Generated GameSystem constructor anchor was not found.'}
$gameSystemText=$gameSystemText.Remove($originalEntryStart,$originalEntryEnd-$originalEntryStart)
$gameSystemText=$gameSystemText.Replace('#include "native_d3d_game_host.hpp"','#include "'+$d3dGameHostInclude+'"')
$gameSystemText=$gameSystemText.Replace('NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight);','NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, true, true);')
$gameSystemText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + '#include "'+$d3dGameHostInclude+'"' + "`n" + $gameSystemText
$traceStages=@(
 @('    JKRAram::create(0xE00000, 0xFFFFFFFF, 8, 7, 3);', 'GameSystem::init ARAM begin', 'GameSystem::init ARAM complete'),
 @('    mObjHolder = new GameSystemObjHolder();', 'GameSystem::init ObjHolder allocation begin', 'GameSystem::init ObjHolder allocation complete'),
 @('    mFontHolder = new GameSystemFontHolder();', 'GameSystem::init FontHolder allocation begin', 'GameSystem::init FontHolder allocation complete'),
 @('    mFontHolder->createFontFromEmbeddedData();', 'GameSystem::init embedded font begin', 'GameSystem::init embedded font complete'),
 @('    initNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));', 'GameSystem::init audio nerve begin', 'GameSystem::init audio nerve complete'),
 @('    mSequenceDirector = new GameSequenceDirector();', 'GameSystem::init sequence director begin', 'GameSystem::init sequence director complete'),
 @('    initGX();', 'GameSystem::init GX begin', 'GameSystem::init GX complete'),
 @('    DrawSyncManager::start(0x300, 15);', 'GameSystem::init draw sync begin', 'GameSystem::init draw sync complete'),
 @('    mSceneController = new GameSystemSceneController();', 'GameSystem::init scene controller begin', 'GameSystem::init scene controller complete'),
 @('    mErrorWatcher = new GameSystemErrorWatcher();', 'GameSystem::init error watcher begin', 'GameSystem::init error watcher complete'),
 @('    mFrameControl = new GameSystemFrameControl();', 'GameSystem::init frame control begin', 'GameSystem::init frame control complete'),
 @('    SingletonHolder< GameSystemResetAndPowerProcess >::init();', 'GameSystem::init reset singleton begin', 'GameSystem::init reset singleton complete'),
 @('    SingletonHolder< GameSystemResetAndPowerProcess >::get()->initWithoutIter();', 'GameSystem::init reset init begin', 'GameSystem::init reset init complete'),
 @('    mStationedArchiveLoader = new GameSystemStationedArchiveLoader();', 'GameSystem::init archive loader begin', 'GameSystem::init archive loader complete'),
 @('    mHomeButtonLayout = new HomeButtonLayout();', 'GameSystem::init home button begin', 'GameSystem::init home button complete'),
 @('    mHomeButtonStateNotifier = new HomeButtonStateNotifier();', 'GameSystem::init home notifier begin', 'GameSystem::init home notifier complete'),
 @('    mDimmingWatcher = new GameSystemDimmingWatcher();', 'GameSystem::init dimming watcher begin', 'GameSystem::init dimming watcher complete'),
 @('    mFontHolder->createFontFromEmbeddedData();', 'GameSystem::init embedded font begin', 'GameSystem::init embedded font complete'),
 @('    initNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));', 'GameSystem::init initial nerve begin', 'GameSystem::init initial nerve complete'),
 @('    mSequenceDirector = new GameSequenceDirector();', 'GameSystem::init SequenceDirector allocation begin', 'GameSystem::init SequenceDirector allocation complete'),
 @('    initGX();', 'GameSystem::init GX begin', 'GameSystem::init GX complete'),
 @('    DrawSyncManager::start(0x300, 15);', 'GameSystem::init draw sync begin', 'GameSystem::init draw sync complete'),
 @('    mSceneController = new GameSystemSceneController();', 'GameSystem::init SceneController allocation begin', 'GameSystem::init SceneController allocation complete'),
 @('    mObjHolder->init();', 'GameSystem::init ObjHolder init begin', 'GameSystem::init ObjHolder init complete')
)
foreach($stage in $traceStages){
 $needle=$stage[0]
 if(!$gameSystemText.Contains($needle)){throw "GameSystem boot trace anchor missing: $needle"}
 $replacement='    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("'+$stage[2]+'");'
 $gameSystemText=$gameSystemText.Replace($needle,$replacement)
}
$frameLoop='void GameSystem::frameLoop() {'
if(!$gameSystemText.Contains($frameLoop)){throw 'GameSystem first-frame trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($frameLoop,$frameLoop + "`n" + '    static bool firstNativeFrame = true;' + "`n" + '    if (firstNativeFrame) galaxyNativeBootTrace("GameSystem first frame entered");')
foreach($stage in @(
 @('    MainLoopFramework::sManager->beginRender();','GameSystem first frame render began'),
 @('    draw();','GameSystem first frame draw complete'),
 @('    MainLoopFramework::sManager->endRender();','GameSystem first frame render ended'),
 @('    update();','GameSystem first frame update complete'),
 @('    calcAnim();','GameSystem first frame animation complete'),
 @('    mObjHolder->captureIfAllowForScreenPreserver();','GameSystem first frame capture complete'),
 @('    MainLoopFramework::sManager->endFrame();','GameSystem first frame end complete'),
 @('    MainLoopFramework::sManager->waitForRetrace();','GameSystem first frame retrace complete')
)){$needle=$stage[0];if(!$gameSystemText.Contains($needle)){throw "GameSystem frame stage trace anchor missing: $needle"};$gameSystemText=$gameSystemText.Replace($needle,$needle + "`n" + '    if (firstNativeFrame) galaxyNativeBootTrace("'+$stage[1]+'");')}
$gameSystemText=$gameSystemText.Replace('    if (firstNativeFrame) galaxyNativeBootTrace("GameSystem first frame retrace complete");','    if (firstNativeFrame) { galaxyNativeBootTrace("GameSystem first frame retrace complete"); firstNativeFrame = false; }')
$inputInclude='#include <Native/GameInput.hpp>'
if($DiagnosticPressA){
 if(!$gameSystemText.Contains($inputInclude)){throw 'GameSystem diagnostic input include anchor missing.'}
 $gameSystemText=$gameSystemText.Replace($inputInclude,$inputInclude + "`n" + '#include <Native/DiagnosticMenuInput.hpp>' + "`n" + '#include <Native/KpadInput.hpp>' + "`n" + '#include <Native/KeyboardKpad.hpp>' + "`n" + '#include <Native/WpadHost.hpp>' + "`n" + '#include <cstdlib>')
 $inputAnchor='    nativePumpGameInputFrame();'
 if(!$gameSystemText.Contains($inputAnchor)){throw 'GameSystem diagnostic input-pump anchor missing.'}
 $inputPulse=@'
    static unsigned diagnosticPadFrame = 0;
    ++diagnosticPadFrame;
    const auto diagnosticInput = NativeDiagnosticInput::next(std::getenv("GALAXY_DIAGNOSTIC_PRESS_AB"),std::getenv("GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT"),std::getenv("GALAXY_DIAGNOSTIC_CREATE_PROFILE"),std::getenv("GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT"),std::getenv("GALAXY_DIAGNOSTIC_SELECT_START_BUTTON"));
    const bool diagnosticPress = diagnosticInput.jump;
    const auto sample = nativeMapKeyboard(diagnosticInput);
    nativeSetHostWpadConnected(true);
    if(!nativeSubmitKpadState(0, sample, double(diagnosticPadFrame)/60.0))
        throw std::runtime_error("Diagnostic controller state queue rejected input");
    nativeDispatchWpadCallbacks();
    if(diagnosticPress) galaxyNativeBootTrace("Diagnostic injected WPAD A press");
'@
 $gameSystemText=$gameSystemText.Replace($inputAnchor,$inputPulse.TrimEnd())
}
$gameSystemUpdate='void GameSystem::update() {'
if(!$gameSystemText.Contains($gameSystemUpdate)){throw 'GameSystem update trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($gameSystemUpdate,$gameSystemUpdate + "`n" + '    galaxyNativeBootTrace("GameSystem update entered");')
foreach($stage in @(
 @('    SingletonHolder< GameSystemResetAndPowerProcess >::get()->movement();','reset-process movement'),
 @('    mSceneController->checkRequestAndChangeScene();','scene change check'),
 @('    mObjHolder->update();','object-holder update'),
 @('    mHomeButtonLayout->movement();','home-button movement'),
 @('        mErrorWatcher->movement();','error-watcher movement'),
 @('    mDimmingWatcher->update();','dimming-watcher update'),
 @('    updateNerve();','nerve update')
)){$needle=$stage[0];if(!$gameSystemText.Contains($needle)){throw "GameSystem update trace stage missing: $needle"};$gameSystemText=$gameSystemText.Replace($needle,'    galaxyNativeBootTrace("GameSystem update '+$stage[1]+' begin");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("GameSystem update '+$stage[1]+' returned");')}
$sceneCheckReturned='    galaxyNativeBootTrace("GameSystem update scene change check returned");'
if(!$gameSystemText.Contains($sceneCheckReturned)){throw 'GameSystem scene-request state trace anchor missing.'}
$sceneCheckState='    { char line[240]; std::snprintf(line,sizeof(line),"SceneController state current=%s next=%s scene=%p state=%u request=%d nerve=%p nextNerve=%p step=%d",mSceneController->mCurrSceneControlInfo.mScene,mSceneController->mNextSceneControlInfo.mScene,static_cast<void*>(mSceneController->mScene),unsigned(mSceneController->mSceneInitializeState),int(mSceneController->isExistRequest()),static_cast<const void*>(mSceneController->mSpine->mCurrNerve),static_cast<const void*>(mSceneController->mSpine->mNextNerve),int(mSceneController->mSpine->mStep)); galaxyNativeBootTrace(line); }'
$gameSystemText=$gameSystemText.Replace($sceneCheckReturned,$sceneCheckReturned + "`n" + $sceneCheckState)
$initializeAudio='void GameSystem::exeInitializeAudio() {'
if(!$gameSystemText.Contains($initializeAudio)){throw 'GameSystem audio-nerve trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($initializeAudio,$initializeAudio + "`n" + '    galaxyNativeBootTrace("GameSystem initial audio nerve entered");')
$audioAsyncCall='        MR::startFunctionAsyncExecute(MR::Functor(mObjHolder, &GameSystemObjHolder::createAudioSystem), 14, INIT_AUDIO_KEY);'
if(!$gameSystemText.Contains($audioAsyncCall)){throw 'GameSystem async-audio trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($audioAsyncCall,'        galaxyNativeBootTrace("GameSystem async-audio submit begin");' + "`n" + $audioAsyncCall + "`n" + '        galaxyNativeBootTrace("GameSystem async-audio submit returned");')
$audioUpdate='    updateSceneController();'
$audioFunctionStart=$gameSystemText.IndexOf($initializeAudio)
$audioFunctionEnd=$gameSystemText.IndexOf("`n}",$audioFunctionStart)
if($audioFunctionEnd -lt 0){throw 'GameSystem audio-nerve boundary missing.'}
$audioFunctionText=$gameSystemText.Substring($audioFunctionStart,$audioFunctionEnd-$audioFunctionStart)
if(!$audioFunctionText.Contains($audioUpdate)){throw 'GameSystem audio-nerve scene-update trace anchor missing.'}
$audioFunctionText=$audioFunctionText.Replace($audioUpdate,'    galaxyNativeBootTrace("GameSystem initial audio scene-controller begin");' + "`n" + $audioUpdate + "`n" + '    galaxyNativeBootTrace("GameSystem initial audio scene-controller returned");')
$audioFunctionText=$audioFunctionText.Replace('        MR::waitForEndFunctionAsyncExecute(INIT_AUDIO_KEY);','        galaxyNativeBootTrace("GameSystem audio-gate wait-for-end begin");' + "`n" + '        MR::waitForEndFunctionAsyncExecute(INIT_AUDIO_KEY);' + "`n" + '        galaxyNativeBootTrace("GameSystem audio-gate wait-for-end returned");')
$audioGate='    if (MR::isEndFunctionAsyncExecute(INIT_AUDIO_KEY) && mObjHolder->mAudioSystem->isLoadDoneWaveDataAtSystemInit()) {'
if(!$audioFunctionText.Contains($audioGate)){throw 'GameSystem audio-ready gate trace anchor missing.'}
$audioFunctionText=$audioFunctionText.Replace($audioGate,'    const bool diagnosticAudioAsyncDone=MR::isEndFunctionAsyncExecute(INIT_AUDIO_KEY); const bool diagnosticAudioWavesDone=mObjHolder->mAudioSystem->isLoadDoneWaveDataAtSystemInit(); { char line[140]; std::snprintf(line,sizeof(line),"GameSystem audio gate async=%d waves=%d",int(diagnosticAudioAsyncDone),int(diagnosticAudioWavesDone)); galaxyNativeBootTrace(line); }' + "`n" + '    if (diagnosticAudioAsyncDone && diagnosticAudioWavesDone) {')
$gameSystemText=$gameSystemText.Substring(0,$audioFunctionStart)+$audioFunctionText+$gameSystemText.Substring($audioFunctionEnd)
$logoInitialize='void GameSystem::exeInitializeLogoScene() {'
if(!$gameSystemText.Contains($logoInitialize)){throw 'GameSystem logo-nerve trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($logoInitialize,$logoInitialize + "`n" + '    galaxyNativeBootTrace("GameSystem logo-scene nerve entered");')
$logoRequest='            MR::requestChangeScene("Logo");'
if(!$gameSystemText.Contains($logoRequest)){throw 'GameSystem Logo request trace anchor missing.'}
$gameSystemText=$gameSystemText.Replace($logoRequest,'            galaxyNativeBootTrace("GameSystem Logo request begin");' + "`n" + $logoRequest + "`n" + '            galaxyNativeBootTrace("GameSystem Logo request returned");')
foreach($stage in @(
 @('    mObjHolder->updateAudioSystem();','audio-system update'),
 @('        mSequenceDirector->update();','sequence-director update'),
 @('            mSystemWipeHolder->movement();','wipe movement'),
 @('        mSceneController->updateScene();','scene update'),
 @('        mSceneController->updateSceneDuringResetProcessing();','reset scene update')
)){$needle=$stage[0];if(!$gameSystemText.Contains($needle)){throw "GameSystem scene-controller trace stage missing: $needle"};$gameSystemText=$gameSystemText.Replace($needle,'        galaxyNativeBootTrace("GameSystem scene-controller '+$stage[1]+' begin");' + "`n" + $needle + "`n" + '        galaxyNativeBootTrace("GameSystem scene-controller '+$stage[1]+' returned");')}
$d3dGameHostSource=Join-Path $root 'native\src\native_d3d_game_host.hpp'
$frameProbeText=Get-Content -LiteralPath (Join-Path $native 'src\native_d3d_render_stream.hpp') -Raw
$frameProbeText='#include "native_framebuffer_probe.hpp"'+"`n"+$frameProbeText
$frameProbeText=$frameProbeText.Replace('  Native::D3dDepthPeek depth(efb);','  NativeFramebufferProbe framebufferProbe(efb);'+"`n"+'  Native::D3dDepthPeek depth(efb);')
$frameProbeText=$frameProbeText.Replace('   switch(op.kind){','   framebufferProbe.before(op,op.kind==NativeRenderOperation::Kind::Draw?&prepared[i].vertices:nullptr);'+"`n"+'   switch(op.kind){')
$frameProbeEnd="   }`n  }`n }`n void execute("
if(!$frameProbeText.Contains($frameProbeEnd)){throw 'Framebuffer operation observer anchor missing.'}
$frameProbeText=$frameProbeText.Replace($frameProbeEnd,"   }`n   framebufferProbe.after(op);`n  }`n }`n void execute(")
$frameProbeText='#include "'+((Join-Path $native 'src\native_projection_failure_probe.hpp').Replace('\','/'))+'"'+"`n"+$frameProbeText
$projectionAnchor='   item.vertices=projectModelTriangles(op.vertices,op.triangles,op.state,{1,1,1,1},rasterColor);'
if(!$frameProbeText.Contains($projectionAnchor)){throw 'Projection failure observation anchor missing'}
$frameProbeText=$frameProbeText.Replace($projectionAnchor,'   try{'+$projectionAnchor.Trim()+'}catch(const std::exception& error){captureNativeProjectionFailure(op,rasterColor,error.what());throw;}')
[IO.File]::WriteAllText((Join-Path $tmp 'native_d3d_render_stream.hpp'),$frameProbeText)
$d3dCommandSubmitterTemp=Join-Path $tmp 'native_d3d_command_submitter.hpp'
$d3dCommandSubmitterText=Get-Content -LiteralPath (Join-Path $root 'native\src\native_d3d_command_submitter.hpp') -Raw
$d3dCommandSubmitterText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $d3dCommandSubmitterText
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace(' void submit(const std::uint8_t* data,std::size_t size,bool finish)override{',' void submit(const std::uint8_t* data,std::size_t size,bool finish)override{' + "`n" + '  char submitTrace[180]; std::snprintf(submitTrace, sizeof(submitTrace), "D3D submit entered data=%p size=%zu finish=%d", static_cast<const void*>(data), size, int(finish)); galaxyNativeBootTrace(submitTrace);')
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace('  candidate.decode(data,size,resources);','  galaxyNativeBootTrace("D3D submit decode begin");' + "`n" + '  candidate.decode(data,size,resources);' + "`n" + '  { unsigned kinds[4]{}; for(const auto& op:candidate.operations()) ++kinds[static_cast<unsigned>(op.kind)]; unsigned bp52=0; for(std::size_t i=0;i+4<size;++i) if(data[i]==0x61&&data[i+1]==0x52) ++bp52; char line[220]; std::snprintf(line,sizeof(line),"D3D decoded ops draw=%u copy=%u token=%u finish=%u raw-BP52=%u",kinds[0],kinds[1],kinds[2],kinds[3],bp52); galaxyNativeBootTrace(line); }' + "`n" + '  galaxyNativeBootTrace("D3D submit decode complete");')
$drawTrace=@'
  { unsigned drawIndex=0; for(const auto& op:candidate.operations()) if(op.kind==NativeRenderOperation::Kind::Draw){ if(drawIndex<16){ char line[300]; if(op.vertices.empty()) std::snprintf(line,sizeof(line),"D3D draw[%u] empty triangles=%zu",drawIndex,op.triangles.size()); else { float minX=op.vertices[0].position[0],maxX=minX,minY=op.vertices[0].position[1],maxY=minY,minZ=op.vertices[0].position[2],maxZ=minZ; for(const auto& v:op.vertices){minX=std::min(minX,v.position[0]);maxX=std::max(maxX,v.position[0]);minY=std::min(minY,v.position[1]);maxY=std::max(maxY,v.position[1]);minZ=std::min(minZ,v.position[2]);maxZ=std::max(maxZ,v.position[2]);} std::snprintf(line,sizeof(line),"D3D draw[%u] vertices=%zu triangles=%zu matrix=%u object0=(%.2f,%.2f,%.2f) bounds=(%.2f..%.2f,%.2f..%.2f,%.2f..%.2f)",drawIndex,op.vertices.size(),op.triangles.size(),op.vertices[0].matrix,op.vertices[0].objectPosition[0],op.vertices[0].objectPosition[1],op.vertices[0].objectPosition[2],minX,maxX,minY,maxY,minZ,maxZ); } galaxyNativeBootTrace(line); } ++drawIndex; } }
'@
 $d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace('  galaxyNativeBootTrace("D3D submit decode complete");',$drawTrace.TrimEnd() + "`n" + '  galaxyNativeBootTrace("D3D submit decode complete");')
$copyLambda='  if(!copyHandler)copyHandler=[this](const Native::GxCommandState::CopyOperation& operation){'
if(!$d3dCommandSubmitterText.Contains($copyLambda)){throw 'D3D copy-handler trace anchor missing.'}
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace($copyLambda,$copyLambda + "`n" + '   { char line[240]; std::snprintf(line,sizeof(line),"D3D copy handler display=%d mode=%08X width=%u height=%u stride=%u address=%08X",int(operation.display()),operation.registers[0x52],operation.width(),operation.height(),operation.strideBytes(),operation.destinationAddress()); galaxyNativeBootTrace(line); }')
$copyBranch='   if(operation.display()){'
if(!$d3dCommandSubmitterText.Contains($copyBranch)){throw 'D3D display-copy handler trace anchor missing.'}
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace($copyBranch,$copyBranch + "`n" + '    { char line[200]; std::snprintf(line,sizeof(line),"D3D display copy width=%u height=%u stride=%u address=%08X",operation.width(),operation.height(),operation.strideBytes(),operation.destinationAddress()); galaxyNativeBootTrace(line); }')
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace('  auto batch=renderer.prepare(candidate.operations(),resources,copyHandler);','  galaxyNativeBootTrace("D3D submit prepare begin");' + "`n" + '  auto batch=renderer.prepare(candidate.operations(),resources,copyHandler);' + "`n" + '  galaxyNativeBootTrace("D3D submit prepare complete");')
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace('   renderer.executePrepared(batch,efb,&abortRequested);','   galaxyNativeBootTrace("D3D submit execute begin");' + "`n" + '   renderer.executePrepared(batch,efb,&abortRequested);' + "`n" + '   galaxyNativeBootTrace("D3D submit execute complete");')
$d3dCommandSubmitterText=$d3dCommandSubmitterText.Replace('   galaxyNativeBootTrace("D3D submit execute complete");','   galaxyNativeBootTrace("D3D submit execute complete");' + "`n" + '   { static bool captured=false; if(!captured){ for(const auto& op:batch.operations) if(op.kind==NativeRenderOperation::Kind::Draw){ captured=true; { const auto tev=op.state.tevColor(0); const auto c0=op.state.tevColor(1); const auto st=op.state.tevStage(0); char q[240]; std::snprintf(q,sizeof(q),"FIRST GX draw verts=%zu prev=%d,%d,%d,%d C0=%d,%d,%d,%d stage=%u inputs=%u,%u,%u,%u blend=%08X z=%08X",op.vertices.size(),tev[0],tev[1],tev[2],tev[3],c0[0],c0[1],c0[2],c0[3],op.state.tevStageCount(),st.color.inputs[0],st.color.inputs[1],st.color.inputs[2],st.color.inputs[3],op.state.bp(0x41),op.state.bp(0x40)); galaxyNativeBootTrace(q); } galaxyNativeBootTrace("EFB readback about to run"); std::vector<std::uint8_t> pixels; try{pixels=efb.readColor();}catch(const std::exception& e){galaxyNativeBootTrace(e.what()); break;} unsigned colors=0; std::uint32_t samples[12]{}; unsigned used=0; for(std::size_t i=0;i+3<pixels.size();i+=4){const std::uint32_t c=(std::uint32_t(pixels[i])<<24)|(std::uint32_t(pixels[i+1])<<16)|(std::uint32_t(pixels[i+2])<<8)|pixels[i+3]; bool seen=false; for(unsigned j=0;j<used;++j) seen|=samples[j]==c; if(!seen){++colors;if(used<12)samples[used++]=c;}} char line[220]; const auto center=(std::size_t(efb.getHeight()/2)*efb.getWidth()+efb.getWidth()/2)*4; std::snprintf(line,sizeof(line),"D3D first-draw EFB pixels=%zu unique-colors-at-least=%u center=%02X%02X%02X%02X",pixels.size()/4,colors,unsigned(pixels[center]),unsigned(pixels[center+1]),unsigned(pixels[center+2]),unsigned(pixels[center+3])); galaxyNativeBootTrace(line); break; } } }')
[IO.File]::WriteAllText($d3dCommandSubmitterTemp,$d3dCommandSubmitterText,[Text.UTF8Encoding]::new($false))
$d3dGameHostText=Get-Content -LiteralPath $d3dGameHostSource -Raw
$d3dGameHostText='#include <cstdio>' + "`n" + '#include <JSystem/JKernel/JKRExpHeap.hpp>' + "`n" + '#include "Game/Util.hpp"' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + 'static void galaxyNativeHeapTrace(const char* stage) { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[280]; std::snprintf(line,sizeof(line),"%s gddr=%p start=%p end=%p size=%u",stage,static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + $d3dGameHostText
$d3dCommandSubmitterInclude=$d3dCommandSubmitterTemp.Replace('\','/')
$d3dGameHostText=$d3dGameHostText.Replace('#include "native_d3d_command_submitter.hpp"','#include "'+$d3dCommandSubmitterInclude+'"')
$d3dGameHostText=$d3dGameHostText.Replace('            const auto instance = GetModuleHandleW(nullptr);','            galaxyNativeBootTrace("D3D host window class registration begin");' + "`n" + '            const auto instance = GetModuleHandleW(nullptr);')
$d3dGameHostText=$d3dGameHostText.Replace('            RECT bounds{0, 0, 960, 720};','            galaxyNativeBootTrace("D3D host window class registered");' + "`n" + '            RECT bounds{0, 0, 960, 720};')
$d3dGameHostText=$d3dGameHostText.Replace('            window = CreateWindowExW(','            galaxyNativeBootTrace("D3D host CreateWindow begin");' + "`n" + '            window = CreateWindowExW(')
$d3dGameHostText=$d3dGameHostText.Replace('            if (!window) throw std::runtime_error("Cannot create native game window");','            galaxyNativeBootTrace("D3D host CreateWindow returned");' + "`n" + '            if (!window) throw std::runtime_error("Cannot create native game window");')
$d3dGameHostText=$d3dGameHostText.Replace('            HRESULT result = D3D11CreateDevice(','            galaxyNativeBootTrace("D3D11CreateDevice begin");' + "`n" + '            HRESULT result = D3D11CreateDevice(')
$deviceResultAnchor='            if (FAILED(result) && !warp)'
if(!$d3dGameHostText.Contains($deviceResultAnchor)){throw 'Initial D3D driver observation anchor missing'}
$d3dGameHostText=$d3dGameHostText.Replace($deviceResultAnchor,'            {char line[160];std::snprintf(line,sizeof(line),"D3D initial device requested=%s HRESULT=%08X",warp?"WARP":"HARDWARE",unsigned(result));galaxyNativeBootTrace(line);}'+"`n"+$deviceResultAnchor)
$d3dGameHostText=$d3dGameHostText.Replace('            if (FAILED(result)) {','            galaxyNativeBootTrace("D3D11CreateDevice returned");' + "`n" + '            if (FAILED(result)) {')
$d3dGameHostText=$d3dGameHostText.Replace('            if (visible) { ShowWindow(window, SW_SHOW); UpdateWindow(window); }','            galaxyNativeBootTrace("D3D host device ready");' + "`n" + '            if (visible) { ShowWindow(window, SW_SHOW); UpdateWindow(window); }')
$d3dGameHostText=$d3dGameHostText.Replace('    NativeD3dGameHost(unsigned width, unsigned height, bool visible = true, bool warp = false)','    NativeD3dGameHost(unsigned width, unsigned height, bool visible = true, bool warp = false)')
$d3dGameHostText=$d3dGameHostText.Replace('        const float black[4] = {0.f, 0.f, 0.f, 1.f};','        galaxyNativeBootTrace("D3D host members constructed");' + "`n" + '        const float black[4] = {0.f, 0.f, 0.f, 1.f};')
$d3dGameHostText=$d3dGameHostText.Replace('        efbTarget.clear(black, 1.f);','        efbTarget.clear(black, 1.f);' + "`n" + '        galaxyNativeBootTrace("D3D host EFB cleared");')
$d3dGameHostText=$d3dGameHostText.Replace('        host = new NativeD3dGameHost(width, height, visible, warp);','        galaxyNativeBootTrace("D3D initialize entered");' + "`n" + '        galaxyNativeBootTrace("D3D host allocation begin");' + "`n" + '        host = new NativeD3dGameHost(width, height, visible, warp);' + "`n" + '        galaxyNativeBootTrace("D3D initialize returned");' + "`n" + '        galaxyNativeBootTrace("D3D host allocation complete");')
$firstFrameBmpPath=(Join-Path $tmp ('retail-frame-'+$DiagnosticCaptureFrame+'-xfb.bmp')).Replace('\','\\')
$firstFrameCapture=@'
static unsigned galaxyRetailXfbCount = 0;
framePresenter.renderYuyv(bytes, operation.width(), operation.height(), operation.strideBytes());
if (++galaxyRetailXfbCount == __GALAXY_CAPTURE_FRAME__) {
    
    RECT client{};
    if (GetClientRect(bootstrap.window, &client) && client.right > client.left && client.bottom > client.top) {
        const unsigned width = static_cast<unsigned>(client.right - client.left);
        const unsigned height = static_cast<unsigned>(client.bottom - client.top);
        const unsigned rowBytes = ((width * 3u + 3u) / 4u) * 4u;
        const auto rgba = framePresenter.readbackBackbuffer();
        const auto decoded = NativeD3dXfbPresenter::decodeYuyv(bytes, operation.width(), operation.height(), operation.strideBytes());
        const auto xfbCenter = (std::size_t(operation.height() / 2) * operation.width() + operation.width() / 2) * 4;
        const auto windowCenter = (std::size_t(height / 2) * width + width / 2) * 4;
        char pixelTrace[240];
        std::snprintf(pixelTrace, sizeof(pixelTrace), "XFB capture frame=%u source-YUYV=%02X%02X%02X%02X decoded-center=%02X%02X%02X backbuffer-center=%02X%02X%02X", galaxyRetailXfbCount, unsigned(bytes[0]), unsigned(bytes[1]), unsigned(bytes[2]), unsigned(bytes[3]), unsigned(decoded[xfbCenter]), unsigned(decoded[xfbCenter + 1]), unsigned(decoded[xfbCenter + 2]), unsigned(rgba[windowCenter]), unsigned(rgba[windowCenter + 1]), unsigned(rgba[windowCenter + 2]));
        galaxyNativeBootTrace(pixelTrace);
        std::vector<unsigned char> bgr(static_cast<std::size_t>(rowBytes) * height, 0);
        for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x) {
            const auto source = (static_cast<std::size_t>(y) * width + x) * 4;
            const auto target = static_cast<std::size_t>(height - 1 - y) * rowBytes + x * 3;
            bgr[target] = rgba[source + 2]; bgr[target + 1] = rgba[source + 1]; bgr[target + 2] = rgba[source];
        }
        BITMAPFILEHEADER fileHeader{}; BITMAPINFOHEADER imageHeader{};
        fileHeader.bfType = 0x4D42; fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(imageHeader);
        fileHeader.bfSize = fileHeader.bfOffBits + static_cast<DWORD>(bgr.size());
        imageHeader.biSize = sizeof(imageHeader); imageHeader.biWidth = static_cast<LONG>(width);
        imageHeader.biHeight = static_cast<LONG>(height); imageHeader.biPlanes = 1;
        imageHeader.biBitCount = 24; imageHeader.biCompression = BI_RGB;
        imageHeader.biSizeImage = static_cast<DWORD>(bgr.size());
        HANDLE image = CreateFileW(L"__GALAXY_FIRST_FRAME_BMP__", GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (image != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(image, &fileHeader, sizeof(fileHeader), &written, nullptr);
            WriteFile(image, &imageHeader, sizeof(imageHeader), &written, nullptr);
            WriteFile(image, bgr.data(), static_cast<DWORD>(bgr.size()), &written, nullptr);
            CloseHandle(image);
            galaxyNativeBootTrace("Retail XFB at display-copy __GALAXY_CAPTURE_FRAME__ captured from D3D back buffer");
        }
    }
}
framePresenter.presentFrame();
'@
$firstFrameCapture=$firstFrameCapture.Replace('__GALAXY_FIRST_FRAME_BMP__',$firstFrameBmpPath).Replace('__GALAXY_CAPTURE_FRAME__',[string]$DiagnosticCaptureFrame)
if(!$d3dGameHostText.Contains('framePresenter.presentYuyv(bytes, operation.width(), operation.height(), operation.strideBytes());')){throw 'Retail XFB capture anchor missing.'}
$d3dGameHostText=$d3dGameHostText.Replace('framePresenter.presentYuyv(bytes, operation.width(), operation.height(), operation.strideBytes());',$firstFrameCapture.Trim())
$d3dGameHostText=[regex]::Replace($d3dGameHostText,'galaxyNativeBootTrace\("(D3D[^"]*)"\);','galaxyNativeHeapTrace("$1");')
[IO.File]::WriteAllText($d3dGameHostTemp,$d3dGameHostText,[Text.UTF8Encoding]::new($false))
if($DiagnosticHiddenWindow){$gameSystemText=$gameSystemText.Replace('NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, true, true);','NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, false, true);')}
if($DiagnosticHardware){$gameSystemText=$gameSystemText.Replace("nativeMode->efbHeight, true, true);","nativeMode->efbHeight, true, false);").Replace("nativeMode->efbHeight, false, true);","nativeMode->efbHeight, false, false);")}
[IO.File]::WriteAllText($gameSystemSource,$gameSystemText,[Text.UTF8Encoding]::new($false))
$gameSystemCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-DGALAXY_BOOT_TRACE',
 '-Dmain=GameSystemMain','-target','x86_64-windows-gnu', "-I$native\include-runtime", "-I$native\include",
 "-I$native\src", "-I$root\Petari\include", "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include",
 "-I$root\Petari\libs\RVLFaceLib\include", "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h',
 '-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h", '-include',"$native\include-runtime\nw4r\ut\Color.h", '-c')
& $zig c++ @gameSystemCommon $gameSystemSource '-o' $gameSystemObject
if($LASTEXITCODE){throw 'Boot-traced Petari GameSystem provider failed to compile.'}
$mainLoopObject=Join-Path $tmp 'MainLoopFramework.obj'
$mainLoopSource=Join-Path $tmp 'MainLoopFramework-boot-traced.cpp'
$mainLoopText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\System\MainLoopFramework.cpp') -Raw
$mainLoopText=& (Join-Path $native 'adapt-clear-depth-texture.ps1') -SourceText $mainLoopText
$mainLoopText=$mainLoopText.Replace('#include <runtime.h>','#include <revolution/os.h>')
$mainLoopText=[regex]::Replace($mainLoopText,'(?<![\w.])(\d+)f\b','$1.0f')
$mainLoopText='#include <cstdio>' + "`n" + '#include <cstdarg>' + "`n" + 'using __va_list = va_list;' + "`n" + '#ifndef TRUE' + "`n" + '#define TRUE 1' + "`n" + '#define FALSE 0' + "`n" + '#endif' + "`n" + 'static inline unsigned long long __cvt_dbl_usll(double v) { return static_cast<unsigned long long>(v); }' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $mainLoopText
$mainLoopFunctionStarts=@(
 @('void MainLoopFramework::endGX() {','MainLoop endGX entered'),
 @('void MainLoopFramework::drawendXfb_single() {','MainLoop drawendXfb_single entered'),
 @('void MainLoopFramework::copyXfb_triple() {','MainLoop copyXfb_triple entered'),
 @('void MainLoopFramework::endRender() {','MainLoop endRender entered'),
 @('void MainLoopFramework::endFrame() {','MainLoop endFrame entered')
)
foreach($item in $mainLoopFunctionStarts){$signature=$item[0];if(!$mainLoopText.Contains($signature)){throw "MainLoop trace function missing: $signature"};$insert="`n"+'    galaxyNativeBootTrace("'+$item[1]+'");';$mainLoopText=$mainLoopText.Replace($signature,$signature+$insert)}
$renderTraceAnchors=@(
 @('void MainLoopFramework::endGX() {','    GXFlush();','MainLoop endGX GXFlush begin','MainLoop endGX GXFlush returned'),
 @('void MainLoopFramework::drawendXfb_single() {','        prepareCopyDisp();','MainLoop XFB prepare-copy begin','MainLoop XFB prepare-copy returned'),
 @('void MainLoopFramework::drawendXfb_single() {','        ::waitDrawDoneAndSetAlarm();','MainLoop XFB wait-draw-done begin','MainLoop XFB wait-draw-done returned'),
 @('void MainLoopFramework::drawendXfb_single() {','        GXFlush();','MainLoop XFB GXFlush begin','MainLoop XFB GXFlush returned'),
 @('void MainLoopFramework::copyXfb_triple() {','        GXCopyDisp(pXfbMgr->getDrawingXfb(), GX_FALSE);','MainLoop XFB GXCopyDisp begin','MainLoop XFB GXCopyDisp returned'),
 @('void MainLoopFramework::copyXfb_triple() {','        GXPixModeSync();','MainLoop XFB PixModeSync begin','MainLoop XFB PixModeSync returned'),
 @('void MainLoopFramework::endRender() {','    endGX();','MainLoop endRender endGX call begin','MainLoop endRender endGX call returned'),
 @('void MainLoopFramework::endRender() {','            drawendXfb_single();','MainLoop endRender XFB call begin','MainLoop endRender XFB call returned'),
 @('void MainLoopFramework::endFrame() {','            ::waitDrawDoneAndSetAlarm();','MainLoop endFrame waitDrawDone begin','MainLoop endFrame waitDrawDone returned'),
 @('void MainLoopFramework::endFrame() {','            GXFlush();','MainLoop endFrame GXFlush begin','MainLoop endFrame GXFlush returned'),
 @('void MainLoopFramework::endRender() {','    calcCombinationRatio();','MainLoop combination-ratio begin','MainLoop endRender calcCombinationRatio returned')
)
foreach($anchor in $renderTraceAnchors){$signature=$anchor[0];$needle=$anchor[1];$start=$mainLoopText.IndexOf($signature);if($start -lt 0){throw "MainLoop trace function missing: $signature"};$at=$mainLoopText.IndexOf($needle,$start);if($at -lt 0){throw "MainLoop trace anchor missing: $needle"};$before='    galaxyNativeBootTrace("'+$anchor[2]+'");' + "`n";$after="`n"+'    galaxyNativeBootTrace("'+$anchor[3]+'");';$mainLoopText=$mainLoopText.Substring(0,$at)+$before+$needle+$after+$mainLoopText.Substring($at+$needle.Length)}
$mainLoopText += "`nvolatile PPCWGPipe gxfifo;`n"
[IO.File]::WriteAllText($mainLoopSource,$mainLoopText,[Text.UTF8Encoding]::new($false))
& $zig c++ @gameSystemCommon $mainLoopSource '-o' $mainLoopObject
if($LASTEXITCODE){throw 'Boot-traced Petari MainLoopFramework provider failed to compile.'}
$objHolderObject=Join-Path $tmp 'GameSystemObjHolder.obj'
$objHolderSource=Join-Path $tmp 'GameSystemObjHolder-boot-traced.cpp'
$objHolderText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\System\GameSystemObjHolder.cpp') -Raw
$objHolderText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + '#include "'+$d3dGameHostInclude+'"' + "`n" + $objHolderText
$nativeHostInit='NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight);'
$nativeHostInitTrace='{ JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"before NativeD3dGameHost init gddr=%p start=%p end=%p size=%u",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + 'NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, true, true);' + "`n" + '{ JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"after NativeD3dGameHost init gddr=%p start=%p end=%p size=%u",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }'
if(!$objHolderText.Contains($nativeHostInit)){throw 'Native D3D host initialization trace anchor missing.'}
$objHolderText=$objHolderText.Replace($nativeHostInit,$nativeHostInitTrace)
if($DiagnosticHiddenWindow){$objHolderText=$objHolderText.Replace('NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, true, true);','NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight, false, true);')}
$messageAnchors=@(
 @('void GameSystemObjHolder::initMessageResource() {','GameSystemObjHolder message resource begin'),
 @('    MR::getMountedArchiveAndHeap("ErrorMessageArchive.arc", &pArchive, &pHeap);','GameSystemObjHolder message archive mounted'),
 @('    const char* languagePrefix = MR::getCurrentLanguagePrefix();','GameSystemObjHolder language selected'),
 @('    void* pSystemMessageArchive = MR::decompressFileFromArchive(pArchive, systemMessagePath, nullptr, 0);','GameSystemObjHolder system message decompressed'),
 @('    MR::createAndAddArchive(pSystemMessageArchive, pHeap, "/Memory/SystemMessage.arc");','GameSystemObjHolder system message archive registered'),
 @('    void* pErrorMessageWindowArchive = MR::decompressFileFromArchive(pArchive, "/LayoutData/ErrorMessageWindow.arc", pHeap, 32);','GameSystemObjHolder error layout decompressed'),
 @('    MR::createAndAddArchive(pErrorMessageWindowArchive, pHeap, "/Memory/ErrorMessageWindow.arc");','GameSystemObjHolder error layout archive registered'),
 @('    MR::createAndAddLayoutHolderRawData("/Memory/ErrorMessageWindow.arc");','GameSystemObjHolder error layout registered'),
 @('    mMessageHolder = new MessageHolder();','GameSystemObjHolder message holder allocated'),
 @('    mMessageHolder->initSystemData();','GameSystemObjHolder system message data initialized')
)
$objHolderText=$objHolderText.Replace('#include "Game/System/GameSystemObjHolder.hpp"','#include "Game/System/GameSystemObjHolder.hpp"' + "`n" + '#include <cstdio>')
foreach($anchor in $messageAnchors){$needle=$anchor[0];if(!$objHolderText.Contains($needle)){throw "Message resource trace anchor missing: $needle"};$replacement=$needle + "`n" + '    galaxyNativeBootTrace("'+$anchor[1]+'");' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[220]; std::snprintf(line,sizeof(line),"stage=%s heap=%p start=%p end=%p size=%u","'+$anchor[1]+'",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }';$objHolderText=$objHolderText.Replace($needle,$replacement)}
$archiveLookup='    MR::getMountedArchiveAndHeap("ErrorMessageArchive.arc", &pArchive, &pHeap);'
$archiveDiag=$archiveLookup + "`n" + '    char nativeArchiveStatus[128];' + "`n" + '    std::snprintf(nativeArchiveStatus, sizeof(nativeArchiveStatus), "ErrorMessageArchive ptr=%p heap=%p", static_cast<void*>(pArchive), static_cast<void*>(pHeap));' + "`n" + '    galaxyNativeBootTrace(nativeArchiveStatus);' + "`n" + '    if (!pArchive || !pHeap) throw std::runtime_error("ErrorMessageArchive was not found in the mounted WBFS");'
$objHolderText=$objHolderText.Replace($archiveLookup,$archiveDiag)
$systemDecompress='    void* pSystemMessageArchive = MR::decompressFileFromArchive(pArchive, systemMessagePath, nullptr, 0);'
$systemDecompressDiag='    void* nativeSystemMessageEntry = pArchive->getResource(systemMessagePath);' + "`n" + '    char nativeSystemMessageStatus[160];' + "`n" + '    const auto nativeSystemMessageBytes = nativeSystemMessageEntry ? pArchive->getResSize(nativeSystemMessageEntry) : 0;' + "`n" + '    auto* nativeMessageBytes = static_cast<const unsigned char*>(nativeSystemMessageEntry);' + "`n" + '    std::snprintf(nativeSystemMessageStatus, sizeof(nativeSystemMessageStatus), "System message entry=%p bytes=%u head=%02X%02X%02X%02X size=%u", nativeSystemMessageEntry, static_cast<unsigned>(nativeSystemMessageBytes), nativeMessageBytes ? nativeMessageBytes[0] : 0, nativeMessageBytes ? nativeMessageBytes[1] : 0, nativeMessageBytes ? nativeMessageBytes[2] : 0, nativeMessageBytes ? nativeMessageBytes[3] : 0, nativeMessageBytes ? ((nativeMessageBytes[4] << 24) | (nativeMessageBytes[5] << 16) | (nativeMessageBytes[6] << 8) | nativeMessageBytes[7]) : 0);' + "`n" + '    galaxyNativeBootTrace(nativeSystemMessageStatus);' + "`n" + '    if (!nativeSystemMessageEntry || !nativeSystemMessageBytes) throw std::runtime_error("System.arc message entry is missing from ErrorMessageArchive");' + "`n" + $systemDecompress
$objHolderText=$objHolderText.Replace($systemDecompress,$systemDecompressDiag)
$errorLayoutDecompress='    void* pErrorMessageWindowArchive = MR::decompressFileFromArchive(pArchive, "/LayoutData/ErrorMessageWindow.arc", pHeap, 32);'
$errorLayoutDecompressDiag=@'
    void* nativeErrorLayoutEntry = pArchive->getResource("/LayoutData/ErrorMessageWindow.arc");
    const auto nativeErrorLayoutBytes = nativeErrorLayoutEntry ? pArchive->getResSize(nativeErrorLayoutEntry) : 0;
    auto* nativeErrorLayoutData = static_cast<const unsigned char*>(nativeErrorLayoutEntry);
    char nativeErrorLayoutStatus[240];
    std::snprintf(nativeErrorLayoutStatus, sizeof(nativeErrorLayoutStatus),
                  "Error layout entry=%p bytes=%u magic=%02X%02X%02X%02X declared=%u heap=%p start=%p end=%p",
                  nativeErrorLayoutEntry, static_cast<unsigned>(nativeErrorLayoutBytes),
                  nativeErrorLayoutData ? nativeErrorLayoutData[0] : 0,
                  nativeErrorLayoutData ? nativeErrorLayoutData[1] : 0,
                  nativeErrorLayoutData ? nativeErrorLayoutData[2] : 0,
                  nativeErrorLayoutData ? nativeErrorLayoutData[3] : 0,
                  nativeErrorLayoutData && nativeErrorLayoutBytes >= 8 ?
                    ((nativeErrorLayoutData[4] << 24) | (nativeErrorLayoutData[5] << 16) |
                     (nativeErrorLayoutData[6] << 8) | nativeErrorLayoutData[7]) : 0,
                  static_cast<void*>(pHeap), pHeap ? static_cast<void*>(pHeap->mStart) : nullptr,
                  pHeap ? static_cast<void*>(pHeap->mEnd) : nullptr);
    galaxyNativeBootTrace(nativeErrorLayoutStatus);
    void* pErrorMessageWindowArchive = MR::decompressFileFromArchive(pArchive, "/LayoutData/ErrorMessageWindow.arc", pHeap, 32);
'@
if(!$objHolderText.Contains($errorLayoutDecompress)){throw 'Error layout decompression trace anchor missing.'}
$objHolderText=$objHolderText.Replace($errorLayoutDecompress,$errorLayoutDecompressDiag.TrimEnd())
$objHolderInitNeedle='void GameSystemObjHolder::init() {'
if(!$objHolderText.Contains($objHolderInitNeedle)){throw 'GameSystemObjHolder::init trace anchor missing.'}
$objHolderText=$objHolderText.Replace($objHolderInitNeedle,$objHolderInitNeedle + "`n" + @'
    galaxyNativeBootTrace("GameSystemObjHolder::init begin");
    galaxyNativeBootTrace("GameSystemObjHolder audio init begin");
'@)
foreach($step in @(@('    initAudio();','GameSystemObjHolder audio init complete','    initRenderMode();','GameSystemObjHolder render mode init complete'),
 @('    initResourceHolderManager();','GameSystemObjHolder resource holder init complete','    initMessageResource();','GameSystemObjHolder message resource init complete'),
 @('    initFunctionAsyncExecutor();','GameSystemObjHolder async executor init complete','    initGameController();','GameSystemObjHolder game controller init complete'),
 @('    initDisplay();','GameSystemObjHolder display init complete'))){
 $call=$step[0]; if(!$objHolderText.Contains($call)){throw "GameSystemObjHolder init anchor missing: $call"}
 $objHolderText=$objHolderText.Replace($call,$call + "`n" + '    galaxyNativeBootTrace("'+$step[1]+'");')
 if($step.Count -ge 4){$next=$step[2]; if(!$objHolderText.Contains($next)){throw "GameSystemObjHolder init anchor missing: $next"}; $objHolderText=$objHolderText.Replace($next,'    galaxyNativeBootTrace("'+$step[3]+'");' + "`n" + $next)}
}
$objHolderText=$objHolderText.Replace('    initGameController();','    galaxyNativeBootTrace("GameSystemObjHolder game controller init begin");' + "`n" + '    initGameController();')
$objHolderText=$objHolderText.Replace('    initDisplay();','    galaxyNativeBootTrace("GameSystemObjHolder display init begin");' + "`n" + '    initDisplay();')
$objHolderText=$objHolderText.Replace('    initResourceHolderManager();','    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"before ResourceHolderManager init gddr=%p start=%p end=%p size=%u",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + '    initResourceHolderManager();')
$heapStages=@(@('    initAudio();','after-audio'),@('    initRenderMode();','after-render-mode'),@('    initResourceHolderManager();','after-resource-holder'),@('    initMessageResource();','after-message-resource'),@('    initFunctionAsyncExecutor();','after-async-executor'),@('    initGameController();','after-game-controller'))
foreach($stage in $heapStages){$call=$stage[0];if(!$objHolderText.Contains($call)){throw "heap stage anchor missing: $call"};$probe='    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[220]; std::snprintf(line,sizeof(line),"stage=%s heap=%p start=%p end=%p size=%u","'+$stage[1]+'",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }';$objHolderText=$objHolderText.Replace($call,$call + "`n" + $probe)}
$displayAnchors=@(
 @('    s32 size = MR::getRequiredExternalFrameBufferSize();','GameSystemObjHolder framebuffer size calculated'),
 @('    JKRHeap* pStationedHeap = MR::getStationedHeapGDDR3();','GameSystemObjHolder display heap selected'),
 @('    _C = new (pStationedHeap, 32) u8[size];','GameSystemObjHolder display buffer C allocated'),
 @('    _10 = new (pStationedHeap, 32) u8[size];','GameSystemObjHolder display buffer 10 allocated'),
 @('    _14 = new (pStationedHeap, 32) u8[size];','GameSystemObjHolder display buffer 14 allocated'),
 @('    pManager = MainLoopFramework::createManager(nullptr, _C, _10, _14, true);','GameSystemObjHolder display manager created')
)
foreach($anchor in $displayAnchors){$needle=$anchor[0];if(!$objHolderText.Contains($needle)){throw "Display init trace anchor missing: $needle"};$objHolderText=$objHolderText.Replace($needle,$needle + "`n" + '    galaxyNativeBootTrace("'+$anchor[1]+'");')}
$objHolderText=$objHolderText.Replace('    JKRHeap* pStationedHeap = MR::getStationedHeapGDDR3();','    JKRHeap* pStationedHeap = MR::getStationedHeapGDDR3();' + "`n" + '    char nativeDisplayHeapStatus[240]; std::snprintf(nativeDisplayHeapStatus, sizeof(nativeDisplayHeapStatus), "display heap=%p start=%p end=%p size=%u", static_cast<void*>(pStationedHeap), pStationedHeap ? static_cast<void*>(pStationedHeap->mStart) : nullptr, pStationedHeap ? static_cast<void*>(pStationedHeap->mEnd) : nullptr, pStationedHeap ? pStationedHeap->mSize : 0); galaxyNativeBootTrace(nativeDisplayHeapStatus);')
$objHolderText=$objHolderText.Replace('    _C = new (pStationedHeap, 32) u8[size];','    galaxyNativeBootTrace("GameSystemObjHolder display buffer C allocation begin");' + "`n" + '    _C = new (pStationedHeap, 32) u8[size];' + "`n" + '    char nativeDisplayBufferStatus[160]; std::snprintf(nativeDisplayBufferStatus, sizeof(nativeDisplayBufferStatus), "display buffer C=%p size=%d", static_cast<void*>(_C), size); galaxyNativeBootTrace(nativeDisplayBufferStatus);')
if($DiagnosticHardware){$objHolderText=$objHolderText.Replace("nativeMode->efbHeight, true, true);","nativeMode->efbHeight, true, false);").Replace("nativeMode->efbHeight, false, true);","nativeMode->efbHeight, false, false);")}
[IO.File]::WriteAllText($objHolderSource,$objHolderText,[Text.UTF8Encoding]::new($false))
$objHolderCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$native\src", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @objHolderCommon $objHolderSource '-o' $objHolderObject
if($LASTEXITCODE){throw 'Boot-traced Petari GameSystemObjHolder provider failed to compile.'}
$asyncObject=Join-Path $tmp 'FunctionAsyncExecutor.obj'
$asyncSource=Join-Path $tmp 'FunctionAsyncExecutor-boot-traced.cpp'
$asyncText=Get-Content -LiteralPath (Join-Path $native 'src\generated\FunctionAsyncExecutor.cpp') -Raw
$asyncText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $asyncText
$asyncText=$asyncText.Replace('        JKRHeap::free(mFunc, nullptr);','        galaxyNativeBootTrace("FunctionAsyncExecInfo functor heap-free begin");' + "`n" + '        JKRHeap::free(mFunc, nullptr);' + "`n" + '        galaxyNativeBootTrace("FunctionAsyncExecInfo functor heap-free returned");')
$asyncInvoke='    (*mFunc)();'
if(!$asyncText.Contains($asyncInvoke)){throw 'Async callback writer anchor missing.'}
$asyncText=$asyncText.Replace($asyncInvoke,'    { char line[200]; std::snprintf(line,sizeof(line),"Async writer callback begin info=%p writer=%p name=%s",static_cast<void*>(this),static_cast<void*>(__GDCurrentDL),mName?mName:"<null>"); galaxyNativeBootTrace(line); }'+"`n"+$asyncInvoke+"`n"+'    { char line[200]; std::snprintf(line,sizeof(line),"Async writer callback returned info=%p writer=%p name=%s",static_cast<void*>(this),static_cast<void*>(__GDCurrentDL),mName?mName:"<null>"); galaxyNativeBootTrace(line); }')
$asyncRun='void* FunctionAsyncExecutorThread::run() {'
if(!$asyncText.Contains($asyncRun)){throw 'FunctionAsyncExecutor worker trace anchor missing.'}
$asyncText=$asyncText.Replace($asyncRun,$asyncRun + "`n" + '    galaxyNativeBootTrace("FunctionAsyncExecutor worker entered");')
$asyncText=$asyncText.Replace('        OSInitFastCast();','        OSInitFastCast();' + "`n" + '        galaxyNativeBootTrace("FunctionAsyncExecutor worker fast-cast initialized");')
$asyncText=$asyncText.Replace('        OSReceiveMessage(&mQueue, &msg, OS_MESSAGE_BLOCK);','        galaxyNativeBootTrace("FunctionAsyncExecutor worker message wait");' + "`n" + '        OSReceiveMessage(&mQueue, &msg, OS_MESSAGE_BLOCK);' + "`n" + '        galaxyNativeBootTrace("FunctionAsyncExecutor worker message received");')
$asyncText=$asyncText.Replace('        info->execute();','        { char line[180]; std::snprintf(line,sizeof(line),"FunctionAsyncExecutor callback begin name=%s",info->mName?info->mName:"<null>"); galaxyNativeBootTrace(line); }' + "`n" + '        info->execute();' + "`n" + '        galaxyNativeBootTrace("FunctionAsyncExecutor callback returned");')
$asyncText=$asyncText.Replace('    OSReceiveMessage(&info->mQueue, &info->mMessage, OS_MESSAGE_BLOCK);','    { char line[180]; std::snprintf(line,sizeof(line),"FunctionAsyncExecutor wait receive begin info=%p name=%s",static_cast<void*>(info),info->mName?info->mName:"<null>"); galaxyNativeBootTrace(line); }' + "`n" + '    OSReceiveMessage(&info->mQueue, &info->mMessage, OS_MESSAGE_BLOCK);' + "`n" + '    galaxyNativeBootTrace("FunctionAsyncExecutor wait receive returned");')
$asyncText=$asyncText.Replace('    delete info;','    galaxyNativeBootTrace("FunctionAsyncExecutor info delete begin");' + "`n" + '    delete info;' + "`n" + '    galaxyNativeBootTrace("FunctionAsyncExecutor info delete returned");')
$asyncCtor='FunctionAsyncExecutor::FunctionAsyncExecutor() : mMainThreadExec(nullptr), mHolders(), _410(nullptr), _414(nullptr) {'
if(!$asyncText.Contains($asyncCtor)){throw 'FunctionAsyncExecutor constructor trace anchor missing.'}
$asyncText=$asyncText.Replace($asyncCtor,$asyncCtor + "`n" + '    galaxyNativeBootTrace("FunctionAsyncExecutor ctor begin");')
$asyncAnchors=@(
 @('        FunctionAsyncExecutorThread* thread = new FunctionAsyncExecutorThread(MR::getStationedHeapNapa());','FunctionAsyncExecutor worker create begin'),
 @('        mThreads[i] = thread;','FunctionAsyncExecutor worker object created'),
 @('        OSResumeThread(thread->mThread);','FunctionAsyncExecutor worker resumed'),
 @('    mMainThreadExec = new FunctionAsyncExecutorOnMainThread(OSGetCurrentThread());','FunctionAsyncExecutor main queue create begin'),
 @('    _410 = JKRUnitHeap::create(sizeof(FunctionAsyncExecInfo), sizeof(JKRUnitHeap) + 256 * sizeof(FunctionAsyncExecInfo) + 32 + 2 * alignof(FunctionAsyncExecInfo), alignof(FunctionAsyncExecInfo), MR::getCurrentHeap(), false);','FunctionAsyncExecutor main queue complete'),
 @('    _414 = JKRExpHeap::create(0x2800, MR::getCurrentHeap(), false);','FunctionAsyncExecutor unit heap complete')
)
foreach($anchor in $asyncAnchors){$needle=$anchor[0];if(!$asyncText.Contains($needle)){throw "FunctionAsyncExecutor trace anchor missing: $needle"};$asyncText=$asyncText.Replace($needle,'    galaxyNativeBootTrace("'+$anchor[1]+'");' + "`n" + $needle)}
foreach($stage in @(
 @('FunctionAsyncExecInfo* info = createAndAddExecInfo(rFunc, priority, pName);','async task allocation begin'),
 @('FunctionAsyncExecutorThread* thread = getSuspendThread();','async worker selection begin'),
 @('    OSSendMessage(&thread->mQueue, info, OS_MESSAGE_NOBLOCK);','async task enqueue begin'),
 @('    MR::FunctorBase* func = rFunc.clone(_414);','async functor clone begin'),
 @('    FunctionAsyncExecInfo* info = new (_410, 0) FunctionAsyncExecInfo(func, priority, pName);','async task record allocation begin'),
 @('    OSLockMutex(&MR::MutexHolder< 2 >::sMutex);','async task-list lock begin'),
 @('    mHolders.push_back(info);','async task-list insertion begin')
)){$needle=$stage[0];if(!$asyncText.Contains($needle)){throw "FunctionAsyncExecutor start trace stage missing: $needle"};$asyncText=$asyncText.Replace($needle,'    galaxyNativeBootTrace("FunctionAsyncExecutor '+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("FunctionAsyncExecutor '+$stage[1].Replace('begin','returned')+'");')}
$asyncCloneTrace='    galaxyNativeBootTrace("FunctionAsyncExecutor async functor clone begin");'
if(!$asyncText.Contains($asyncCloneTrace)){throw 'FunctionAsyncExecutor clone-heap trace anchor missing.'}
$asyncText=$asyncText.Replace($asyncCloneTrace,'    { char line[280]; void* functor=const_cast<MR::FunctorBase*>(&rFunc); void* vtable=*reinterpret_cast<void**>(functor); std::snprintf(line,sizeof(line),"Async clone inspect name=%s functor=%p vtable=%p firstEntry=%p",pName?pName:"<null>",functor,vtable,*reinterpret_cast<void**>(vtable)); galaxyNativeBootTrace(line); }' + "`n" + '    { char line[240]; std::snprintf(line,sizeof(line),"Async clone heap=%p start=%p end=%p size=%u free=%d current=%p",static_cast<void*>(_414),_414?static_cast<void*>(_414->mStart):nullptr,_414?static_cast<void*>(_414->mEnd):nullptr,_414?_414->mSize:0,_414?_414->getTotalFreeSize():-1,static_cast<void*>(JKRHeap::sCurrentHeap)); galaxyNativeBootTrace(line); }' + "`n" + $asyncCloneTrace)
$recordAlloc='    FunctionAsyncExecInfo* info = new (_410, 0) FunctionAsyncExecInfo(func, priority, pName);'
if(!$asyncText.Contains($recordAlloc)){throw 'FunctionAsyncExecutor record-heap snapshot anchor missing.'}
$recordHeapSnapshot=@'
    { char line[280]; std::snprintf(line,sizeof(line),"Async record heap=%p start=%p end=%p units=%p bitmap=%p unitSize=%u count=%u free=%u current=%p root=%p",static_cast<void*>(_410),static_cast<void*>(_410?_410->mStart:nullptr),static_cast<void*>(_410?_410->mEnd:nullptr),static_cast<void*>(_410?_410->mUnits:nullptr),static_cast<void*>(_410?_410->mBat:nullptr),_410?_410->mUnitSize:0,_410?_410->mUnitCount:0,_410?_410->mTotalFreeSize:0,static_cast<void*>(JKRHeap::sCurrentHeap),static_cast<void*>(JKRHeap::sRootHeap)); galaxyNativeBootTrace(line); }
'@
$asyncText=$asyncText.Replace($recordAlloc,$recordHeapSnapshot + "`n" + $recordAlloc)
[IO.File]::WriteAllText($asyncSource,$asyncText,[Text.UTF8Encoding]::new($false))
$asyncCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @asyncCommon $asyncSource '-o' $asyncObject
if($LASTEXITCODE){throw 'Boot-traced Petari FunctionAsyncExecutor provider failed to compile.'}
$functorInclude=Join-Path $tmp 'functor-trace-include'
$functorHeader=Join-Path $functorInclude 'Game\Util\Functor.hpp'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $functorHeader) | Out-Null
$functorText=Get-Content -LiteralPath (Join-Path $native 'include\Game\Util\Functor.hpp') -Raw
$functorText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $functorText
$functorClone='FunctorBase* clone(JKRHeap* heap)const override{'
if(!$functorText.Contains($functorClone)){throw 'Native functor clone trace anchor missing.'}
$functorText=$functorText.Replace($functorClone, $functorClone + ' galaxyNativeBootTrace("Runtime functor clone dispatch");')
[IO.File]::WriteAllText($functorHeader,$functorText,[Text.UTF8Encoding]::new($false))
$heapPlacementInclude=Join-Path $tmp 'heap-placement-trace-include'
$heapPlacementHeader=Join-Path $heapPlacementInclude 'Native\HeapPlacement.hpp'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $heapPlacementHeader) | Out-Null
$heapPlacementText=Get-Content -LiteralPath (Join-Path $native 'include\Native\HeapPlacement.hpp') -Raw
$heapPlacementText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $heapPlacementText
$heapPlacementText=$heapPlacementText.Replace('    void* memory = JKRHeap::alloc(static_cast<u32>(size), alignment, heap);','    { char line[180]; std::snprintf(line,sizeof(line),"Functor placement-new enter size=%zu heap=%p align=%d",size,static_cast<void*>(heap),alignment); galaxyNativeBootTrace(line); }' + "`n" + '    void* memory = JKRHeap::alloc(static_cast<u32>(size), alignment, heap);' + "`n" + '    { char line[180]; std::snprintf(line,sizeof(line),"Functor placement-new alloc returned memory=%p",memory); galaxyNativeBootTrace(line); }')
[IO.File]::WriteAllText($heapPlacementHeader,$heapPlacementText,[Text.UTF8Encoding]::new($false))
$heapFunctorInclude=Join-Path $tmp 'native-functor-trace-include'
$heapFunctorHeader=Join-Path $heapFunctorInclude 'Native\HeapFunctor.hpp'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $heapFunctorHeader) | Out-Null
$heapFunctorText=Get-Content -LiteralPath (Join-Path $native 'include\Native\HeapFunctor.hpp') -Raw
$heapFunctorText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $heapFunctorText
$heapFunctorText=$heapFunctorText.Replace('FunctorBase* clone(JKRHeap* heap)const override{return new(heap) NativeHeapCallback(callback);}', 'FunctorBase* clone(JKRHeap* heap)const override{galaxyNativeBootTrace("NativeHeapCallback clone dispatch"); return new(heap) NativeHeapCallback(callback);}')
$heapFunctorText=$heapFunctorText.Replace('void* operator new(std::size_t size,JKRHeap* heap){', 'void* operator new(std::size_t size,JKRHeap* heap){galaxyNativeBootTrace("NativeHeapCallback placement-new dispatch");')
[IO.File]::WriteAllText($heapFunctorHeader,$heapFunctorText,[Text.UTF8Encoding]::new($false))
$gameSystemFunctorCommon=@("-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", '-include',$heapPlacementHeader) + $gameSystemCommon
& $zig c++ @gameSystemFunctorCommon $gameSystemSource '-o' $gameSystemObject
if($LASTEXITCODE){throw 'Boot-traced GameSystem functor-vtable provider failed to compile.'}
$asyncFunctorObject=Join-Path $tmp 'FunctionAsyncExecutor-functor-traced.obj'
$asyncFunctorCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-include',$heapPlacementHeader,'-c')
& $zig c++ @asyncFunctorCommon $asyncSource '-o' $asyncFunctorObject
if($LASTEXITCODE){throw 'Boot-traced FunctionAsyncExecutor functor-vtable provider failed to compile.'}
$asyncObject=$asyncFunctorObject
$stationedLoaderObject=Join-Path $tmp 'GameSystemStationedArchiveLoader-functor-traced.obj'
$stationedLoaderSource=Join-Path $tmp 'GameSystemStationedArchiveLoader-phase-traced.cpp'
$stationedLoaderText=Get-Content -LiteralPath (Join-Path $native 'src\generated\GameSystemStationedArchiveLoader.cpp') -Raw
$stationedLoaderText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $stationedLoaderText
foreach($phase in @('LoadAudio1stWaveData','LoadStationedArchiveOthers','LoadStationedArchivePlayer','InitializeGameData','End','Suspended')){
 $anchor='void GameSystemStationedArchiveLoader::exe'+$phase+'() {'
 if(!$stationedLoaderText.Contains($anchor)){throw "Missing stationed loader phase: $phase"}
 $marker='    if (getNerveStep() % 120 == 0) { char line[160]; std::snprintf(line, sizeof(line), "Stationed loader phase='+$phase+' step=%d", getNerveStep()); galaxyNativeBootTrace(line); }'
 $stationedLoaderText=$stationedLoaderText.Replace($anchor,$anchor+"`n"+$marker)
}
[IO.File]::WriteAllText($stationedLoaderSource,$stationedLoaderText,[Text.UTF8Encoding]::new($false))
$stationedNoWeakHeader=Join-Path $tmp 'StationedArchiveNoWeak.hpp'
[IO.File]::WriteAllText($stationedNoWeakHeader, "#include <revolution/types.h>`n#undef ATTRIBUTE_WEAK`n#define ATTRIBUTE_WEAK`n",[Text.UTF8Encoding]::new($false))
$stationedLoaderCommon=@("-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", '-include',$heapPlacementHeader) + $gameSystemCommon + @('-include',$stationedNoWeakHeader)
& $zig c++ @stationedLoaderCommon $stationedLoaderSource '-o' $stationedLoaderObject
if($LASTEXITCODE){throw 'Boot-traced GameSystemStationedArchiveLoader provider failed to compile.'}
$resourceHolderObject=Join-Path $tmp 'ResourceHolder-texture-traced.obj'
$resourceHolderSource=Join-Path $native 'src\generated\ResourceHolder.cpp'
$resourceHolderCommon=@("-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", '-include',$heapPlacementHeader) + $gameSystemCommon
& $zig c++ @resourceHolderCommon $resourceHolderSource '-o' $resourceHolderObject
if($LASTEXITCODE){throw 'Boot-traced ResourceHolder provider failed to compile.'}
$nativeBckSource=Join-Path $native 'src\native_bck_animation.cpp'
$nativeBckText=Get-Content -LiteralPath $nativeBckSource -Raw
$nativeBckText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $nativeBckText
$bcaReturn='    return createBcaAnimation(data,std::size_t(size));'
if(!$nativeBckText.Contains($bcaReturn)){throw 'BCA loader trace anchor missing.'}
$bcaTrace='    auto* loaded = static_cast<J3DAnmTransformFull*>(createBcaAnimation(data,std::size_t(size)));' + "`n" + '    if (loaded) { const auto* table=loaded->mAnmTable; char line[320]; std::snprintf(line,sizeof(line),"BCA loaded bytes=%d anim=%p frames=%d joints=%u table=%p scale=%p/%u rot=%p/%u trans=%p/%u",int(size),static_cast<void*>(loaded),int(loaded->mFrameMax),unsigned(loaded->field_0x1e),static_cast<const void*>(table),static_cast<void*>(loaded->mScaleData),unsigned(table[0].mScaleMaxFrame),static_cast<void*>(loaded->mRotData),unsigned(table[0].mRotationMaxFrame),static_cast<void*>(loaded->mTransData),unsigned(table[0].mTranslateMaxFrame)); galaxyNativeBootTrace(line); }' + "`n" + '    return loaded;'
$nativeBckText=$nativeBckText.Replace($bcaReturn,$bcaTrace)
$nativeBckTraceSource=Join-Path $tmp 'native_bck_animation-boot-traced.cpp'
$nativeBckTraceObject=Join-Path $tmp 'native_bck_animation-boot-traced.obj'
[IO.File]::WriteAllText($nativeBckTraceSource,$nativeBckText,[Text.UTF8Encoding]::new($false))
$nativeBckTraceCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',"-I$(Join-Path $native 'src')",'-I'+$native,'-I'+(Join-Path $native 'include'),'-I'+(Join-Path $native 'include-runtime'),'-I'+(Join-Path $root 'Petari\include'),'-I'+(Join-Path $root 'Petari\libs\JSystem\include'),'-I'+(Join-Path $root 'Petari\libs\RVL_SDK\include'),'-I'+(Join-Path $root 'Petari\libs\nw4r\include'),'-include','revolution/types.h','-include','revolution/mtx.h','-c')
& $zig c++ @nativeBckTraceCommon $nativeBckTraceSource '-o' $nativeBckTraceObject
if($LASTEXITCODE){throw 'Boot-traced native BCK/BCA decoder failed to compile.'}
$materialAnmObject=Join-Path $tmp 'MaterialAnmBuffer-table-traced.obj'
$materialAnmSource=Join-Path $tmp 'MaterialAnmBuffer-table-traced.cpp'
$materialAnmText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\Animation\MaterialAnmBuffer.cpp') -Raw
$materialAnmText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $materialAnmText
$materialCtorAnchor='    searchUpdateMaterialID(pResourceHolder, pModelData);'
if(!$materialAnmText.Contains($materialCtorAnchor)){throw 'Material-animation constructor trace anchor missing.'}
$materialAnmText=$materialAnmText.Replace($materialCtorAnchor,'    galaxyNativeBootTrace("MaterialAnm constructor search begin");' + "`n" + $materialCtorAnchor + "`n" + '    galaxyNativeBootTrace("MaterialAnm constructor search returned");')
$materialDiffAnchor='void MaterialAnmBuffer::setDiffFlag(const ResourceHolder* pResourceHolder) {'
if(!$materialAnmText.Contains($materialDiffAnchor)){throw 'Material-animation diff-flag trace anchor missing.'}
$materialAnmText=$materialAnmText.Replace($materialDiffAnchor,$materialDiffAnchor + "`n" + '    galaxyNativeBootTrace("MaterialAnm setDiffFlag entered");')
$bpkFlagAnchor='MR::onDiffFlagBpk(_4, pAnm, pBpkName);'
if(!$materialAnmText.Contains($bpkFlagAnchor)){throw 'BPK diff-flag trace anchor missing.'}
$materialAnmText=$materialAnmText.Replace($bpkFlagAnchor,'galaxyNativeBootTrace("BPK diff-flag begin");' + "`n" + $bpkFlagAnchor + "`n" + 'galaxyNativeBootTrace("BPK diff-flag returned");')
$attachAnchor='void MaterialAnmBuffer::attachMaterialAnmBuffer(J3DModelData* pModelData, bool useDiffFlag) {'
if(!$materialAnmText.Contains($attachAnchor)){throw 'Material-animation attach trace anchor missing.'}
$materialAnmText=$materialAnmText.Replace($attachAnchor,$attachAnchor + "`n" + '    galaxyNativeBootTrace("MaterialAnm attach entered");')
$attachAssignment='pModelData->getMaterialNodePointer(i)->mMaterialAnm = &_0[anmIndex];'
if(!$materialAnmText.Contains($attachAssignment)){throw 'Material-animation attachment assignment trace anchor missing.'}
$attachTrace='{ char line[240]; std::snprintf(line,sizeof(line),"MaterialAnm attach before index=%u count=%u flags=%08X anm=%u buffer=%p",unsigned(i),unsigned(pModelData->getMaterialNum()),unsigned(_4? _4[i]:0),unsigned(anmIndex),static_cast<void*>(&_0[anmIndex])); galaxyNativeBootTrace(line); }' + "`n" + $attachAssignment + "`n" + 'galaxyNativeBootTrace("MaterialAnm attach assignment returned");'
$materialAnmText=$materialAnmText.Replace($attachAssignment,$attachTrace)
$materialAnmAnchor='    const ResTable* pBpkTable = pResourceHolder->mBpkResTable;'
if(!$materialAnmText.Contains($materialAnmAnchor)){throw 'Material animation table trace anchor missing.'}
$materialAnmText=$materialAnmText.Replace($materialAnmAnchor,$materialAnmAnchor + "`n" + '    { char line[320]; std::snprintf(line,sizeof(line),"MaterialAnm tables bpk=%p/%u btp=%p/%u btk=%p/%u brk=%p/%u",static_cast<const void*>(pBpkTable),unsigned(pBpkTable->mCount),static_cast<const void*>(pResourceHolder->mBtpResTable),unsigned(pResourceHolder->mBtpResTable->mCount),static_cast<const void*>(pResourceHolder->mBtkResTable),unsigned(pResourceHolder->mBtkResTable->mCount),static_cast<const void*>(pResourceHolder->mBrkResTable),unsigned(pResourceHolder->mBrkResTable->mCount)); galaxyNativeBootTrace(line); }')
$bpkSearchAnchor='pAnm->searchUpdateMaterialID(pModelData);'
$bpkSearchIndex=$materialAnmText.IndexOf($bpkSearchAnchor)
if($bpkSearchIndex -lt 0){throw 'BPK material-name search trace anchor missing.'}
$bpkSearchTrace='galaxyNativeBootTrace("BPK material-name lookup begin");' + "`n" + $bpkSearchAnchor + "`n" + 'galaxyNativeBootTrace("BPK material-name lookup returned");'
$materialAnmText=$materialAnmText.Remove($bpkSearchIndex,$bpkSearchAnchor.Length).Insert($bpkSearchIndex,$bpkSearchTrace)
foreach($trace in @(
 @('for (u32 i = 0; i < pBpkTable->mCount; i++) {','pBpkTable','BPK'),
 @('for (u32 i = 0; i < pBtpTable->mCount; i++) {','pBtpTable','BTP'),
 @('for (u32 i = 0; i < pBtkTable->mCount; i++) {','pBtkTable','BTK'),
 @('for (u32 i = 0; i < pBrkTable->mCount; i++) {','pBrkTable','BRK')
)){
 $needle=$trace[0]
 if(!$materialAnmText.Contains($needle)){continue}
 $log=$needle + "`n" + '        { const auto* table='+$trace[1]+'; void* object=table->getRes(i); char line[224]; std::snprintf(line,sizeof(line),"MaterialAnm '+$trace[2]+' item=%u name=%s resource=%p",unsigned(i),table->getResName(i)?table->getResName(i):"<null>",object); galaxyNativeBootTrace(line); }'
 $materialAnmText=$materialAnmText.Replace($needle,$log)
}
[IO.File]::WriteAllText($materialAnmSource,$materialAnmText,[Text.UTF8Encoding]::new($false))
$materialAnmCommon=@("-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", '-include',$heapPlacementHeader) + $gameSystemCommon
& $zig c++ @materialAnmCommon $materialAnmSource '-o' $materialAnmObject
if($LASTEXITCODE){throw 'Boot-traced MaterialAnmBuffer provider failed to compile.'}
$sceneControllerOriginal=Join-Path $root 'Petari\src\Game\System\GameSystemSceneController.cpp'
$sceneControllerSource=Join-Path $tmp 'GameSystemSceneController-substage-traced.cpp'
$sceneControllerText=Get-Content -LiteralPath $sceneControllerOriginal -Raw
$sceneControllerText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $sceneControllerText
$sceneInit='void GameSystemSceneController::initializeScene() {'
if(!$sceneControllerText.Contains($sceneInit)){throw 'Scene initialization substage trace anchor missing.'}
$sceneControllerText=$sceneControllerText.Replace($sceneInit,$sceneInit + "`n" + '    galaxyNativeBootTrace("GameSystemSceneController initializeScene entered");')
foreach($stage in @(
 @('SingletonHolder< HeapMemoryWatcher >::get()->createFileCacheHeapOnGameHeap(0x1040400);','file cache heap ready'),
 @('SingletonHolder< HeapMemoryWatcher >::get()->createSceneHeapOnGameHeap();','scene heap ready'),
 @('    updateSceneControlInfo();','scene control info ready'),
 @('    MR::resetWPad();','WPad reset complete'),
 @('    mScene = MR::createScene(mCurrSceneControlInfo.mScene);','scene factory returned'),
 @('    mScene->initNameObjListExecutor();','scene list executor ready'),
 @('    mScene->initSceneObjHolder();','scene object holder ready'),
 @('    mScene->init();','scene init returned'),
 @('    SceneFunction::allocateDrawBufferActorList();','draw buffer actors allocated'),
 @('    MR::clearFileLoaderRequestFileInfo(mIsResetProcessing);','loader request table cleared'),
 @('    SingletonHolder< HeapMemoryWatcher >::get()->checkRestMemory();','rest memory checked')
)){$needle=$stage[0];if(!$sceneControllerText.Contains($needle)){throw "Scene initialization substage anchor missing: $needle"};$sceneControllerText=$sceneControllerText.Replace($needle,'    galaxyNativeBootTrace("GameSystemSceneController '+$stage[1]+' begin");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("GameSystemSceneController '+$stage[1]+' returned");')}
[IO.File]::WriteAllText($sceneControllerSource,$sceneControllerText,[Text.UTF8Encoding]::new($false))
$sceneControllerObject=Join-Path $tmp 'GameSystemSceneController-functor-traced.obj'
$sceneControllerCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include',"$native\include-runtime\native_no_weak.h",'-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-include',$heapPlacementHeader,'-c')
& $zig c++ @sceneControllerCommon $sceneControllerSource '-o' $sceneControllerObject
if($LASTEXITCODE){throw 'Boot-traced Petari scene-controller provider failed to compile.'}
$logoSourceOriginal=Join-Path $root 'Petari\src\Game\Scene\LogoScene.cpp'
$logoSource=Join-Path $tmp 'LogoScene-substage-traced.cpp'
$logoText=Get-Content -LiteralPath $logoSourceOriginal -Raw
$logoText="#include <Game/System/GameSystem.hpp>`n#include <Game/System/GameSequenceDirector.hpp>`n#include <Game/System/SaveDataHandleSequence.hpp>`n#include <Game/System/SaveDataHandler.hpp>`n#include <Game/System/NANDManager.hpp>`n#include <Game/Util/SingletonHolder.hpp>`n"+$logoText
$logoText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $logoText
$logoText=$logoText.Replace('void LogoScene::init() {','void LogoScene::init() {' + "`n" + '    galaxyNativeBootTrace("LogoScene init entered");')
$logoText=$logoText.Replace('void LogoScene::initLayout() {','void LogoScene::initLayout() {' + "`n" + '    galaxyNativeBootTrace("LogoScene initLayout entered");')
$logoDrawTrace=@'
void LogoScene::draw() const {
    static unsigned diagnosticDrawCount = 0;
    if (++diagnosticDrawCount % 120 == 0) {
        char line[512];
        std::snprintf(line, sizeof(line), "Logo state draws=%u step=%d strap-fadein=%d strap-display=%d fader-rate=%.3f fader-step=%d fade-end=%d wait-system=%d mount-data=%d deactive=%d system-done=%d", diagnosticDrawCount, getNerveStep(), int(isNerve(GET_NERVE_ANON(LogoSceneStrapFadein))), int(isNerve(GET_NERVE_ANON(LogoSceneStrapDisplay))), mLogoFader ? mLogoFader->mRate : -1.0f, mLogoFader ? mLogoFader->getNerveStep() : -1, mLogoFader ? int(mLogoFader->isFadeEnd()) : -1, int(isNerve(GET_NERVE_ANON(LogoSceneWaitReadDoneSystemArchive))), int(isNerve(GET_NERVE_ANON(LogoSceneMountGameData))), int(isNerve(GET_NERVE_ANON(LogoSceneDeactive))), int(GameSystemFunction::isDoneLoadSystemArchive()));
        galaxyNativeBootTrace(line);
        auto* sequence = SingletonHolder<GameSystem>::get()->mSequenceDirector->mSaveDataHandleSequence;
        auto* handler = sequence ? sequence->mSaveDataHandler : nullptr;
        auto* request = handler ? handler->mNANDRequestInfo : nullptr;
        std::snprintf(line, sizeof(line), "Save caller step=%d active=%d handler-step=%d handler-done=%d nand-state=%d nand-result=%d nand-type=%d", sequence ? sequence->getNerveStep() : -1, sequence ? int(sequence->isActive()) : -1, handler ? handler->getNerveStep() : -1, handler ? int(handler->isDone()) : -1, request ? request->_40 : -1, request ? request->mResult : -999, request ? request->mType : -1);
        galaxyNativeBootTrace(line);
    }
'@
if(!$logoText.Contains('void LogoScene::draw() const {')){throw 'Logo draw state trace anchor missing.'}
$logoText=$logoText.Replace('void LogoScene::draw() const {',$logoDrawTrace.TrimEnd())
foreach($stage in @(
 @('    SceneFunction::createHioBasicNode(this);','HIO node create'),
 @('    SceneFunction::initForNameObj();','name object scene init'),
 @('    MR::createSceneObj(SceneObj_CameraContext);','camera context scene object create'),
 @('    MR::createSceneObj(SceneObj_NameObjGroup);','name object group scene object create'),
 @('    mStrapLayout->kill();','strap layout kill'),
 @('    mLogoFader->initWithoutIter();','logo fader init'),
 @('    mLogoFader->setBlank();','logo fader blank'),
 @('    mLogoFader->appear();','logo fader appear'),
 @('    MR::connectToSceneLayout(mLogoFader);','logo fader scene connect')
)){$needle=$stage[0];if(!$logoText.Contains($needle)){throw "Logo scene trace anchor missing: $needle"};$logoText=$logoText.Replace($needle,'    galaxyNativeBootTrace("LogoScene '+$stage[1]+' begin");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("LogoScene '+$stage[1]+' returned");')}
[IO.File]::WriteAllText($logoSource,$logoText,[Text.UTF8Encoding]::new($false))
$logoObject=Join-Path $tmp 'LogoScene-substage-traced.obj'
& $zig c++ @sceneControllerCommon $logoSource '-o' $logoObject
if($LASTEXITCODE){throw 'Boot-traced Petari LogoScene provider failed to compile.'}
$sceneFunctionSourceOriginal=Join-Path $root 'Petari\src\Game\Scene\SceneFunction.cpp'
$sceneFunctionSource=Join-Path $tmp 'SceneFunction-substage-traced.cpp'
$sceneFunctionText=Get-Content -LiteralPath $sceneFunctionSourceOriginal -Raw
$sceneFunctionText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $sceneFunctionText
foreach($stage in @(
 @('MR::createSceneObj(SceneObj_NameObjExecuteHolder);','name object execute holder'),
 @('MR::createSceneObj(SceneObj_StopSceneController);','stop scene controller'),
 @('MR::createSceneObj(SceneObj_SceneNameObjMovementController);','scene name object movement controller')
)){$needle=$stage[0];if(!$sceneFunctionText.Contains($needle)){throw "SceneFunction trace anchor missing: $needle"};$sceneFunctionText=$sceneFunctionText.Replace($needle,'    galaxyNativeBootTrace("SceneFunction '+$stage[1]+' begin");' + "`n" + '    '+$needle + "`n" + '    galaxyNativeBootTrace("SceneFunction '+$stage[1]+' returned");')}
[IO.File]::WriteAllText($sceneFunctionSource,$sceneFunctionText,[Text.UTF8Encoding]::new($false))
$sceneFunctionObject=Join-Path $tmp 'SceneFunction-substage-traced.obj'
& $zig c++ @sceneControllerCommon $sceneFunctionSource '-o' $sceneFunctionObject
if($LASTEXITCODE){throw 'Boot-traced Petari scene-function provider failed to compile.'}
$collisionSourceOriginal=Join-Path $native 'src\native_collision_scene.cpp'
$collisionSource=Join-Path $tmp 'native_collision_scene-substage-traced.cpp'
$collisionText=Get-Content -LiteralPath $collisionSourceOriginal -Raw
$collisionText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $collisionText
$collisionText=$collisionText.Replace('NameObj* SceneObjHolder::newEachObj(int id){','NameObj* SceneObjHolder::newEachObj(int id){' + "`n" + ' { char line[128]; std::snprintf(line,sizeof(line),"native scene-object factory id=%d",id); galaxyNativeBootTrace(line); }')
$collisionText=$collisionText.Replace('SceneObjHolder* getSceneObjHolder(){','SceneObjHolder* getSceneObjHolder(){' + "`n" + ' galaxyNativeBootTrace(active ? "native scene-object holder uses active collision scene" : "native scene-object holder uses game scene fallback");')
$collisionText=$collisionText.Replace('void connectToScene(NameObj* o,int movement,int animation,int buffer,int draw){','void connectToScene(NameObj* o,int movement,int animation,int buffer,int draw){' + "`n" + ' galaxyNativeBootTrace(active ? "native NameObj connect uses collision schedule" : "native NameObj connect uses game executor");')
$collisionText=$collisionText.Replace(' if(!active){' + "`n" + '  MR::registerNameObjToExecuteHolder(o,movement,animation,buffer,draw);',' if(!active){' + "`n" + '  galaxyNativeBootTrace("native NameObj connect register actor begin");' + "`n" + '  MR::registerNameObjToExecuteHolder(o,movement,animation,buffer,draw);' + "`n" + '  galaxyNativeBootTrace("native NameObj connect register actor returned");')
foreach($stage in @(
 @('if(id==SceneObj_NameObjExecuteHolder)return new NameObjExecuteHolder(4096);','SceneObj_NameObjExecuteHolder','NameObjExecuteHolder','auto* p=new NameObjExecuteHolder(4096);'),
 @('if(id==SceneObj_NameObjGroup)return new NameObjGroup("IgnorePauseNameObj",16);','SceneObj_NameObjGroup','NameObjGroup','auto* p=new NameObjGroup("IgnorePauseNameObj",16);'),
 @('if(id==SceneObj_StopSceneController)return new StopSceneController();','SceneObj_StopSceneController','StopSceneController','auto* p=new StopSceneController();'),
 @('if(id==SceneObj_SceneNameObjMovementController)return new SceneNameObjMovementController();','SceneObj_SceneNameObjMovementController','SceneNameObjMovementController','auto* p=new SceneNameObjMovementController();')
)){$needle=$stage[0];if(!$collisionText.Contains($needle)){throw "Native collision factory trace anchor missing: $needle"};$replacement='if(id=='+$stage[1]+'){ galaxyNativeBootTrace("'+$stage[2]+' construction begin"); '+$stage[3]+' galaxyNativeBootTrace("'+$stage[2]+' construction returned"); return p; }';$collisionText=$collisionText.Replace($needle,$replacement)}
[IO.File]::WriteAllText($collisionSource,$collisionText,[Text.UTF8Encoding]::new($false))
$stopSourceOriginal=Join-Path $root 'Petari\src\Game\Scene\StopSceneController.cpp'
$stopSource=Join-Path $tmp 'StopSceneController-substage-traced.cpp'
$stopText=Get-Content -LiteralPath $stopSourceOriginal -Raw
$stopText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $stopText
$stopText=$stopText.Replace('StopSceneController::StopSceneController() : NameObj("StopSceneController"), mDelayRequestArray(), mFrame() {','StopSceneController::StopSceneController() : NameObj("StopSceneController"), mDelayRequestArray(), mFrame() {' + "`n" + '    galaxyNativeBootTrace("StopSceneController constructor body entered");')
$stopText=$stopText.Replace('    mDelayRequestArray = new NameObjGroup(','    galaxyNativeBootTrace("StopSceneController NameObjGroup allocation begin");' + "`n" + '    mDelayRequestArray = new NameObjGroup(')
$stopText=$stopText.Replace('        StopSceneDelayRequest* delayRequest = new StopSceneDelayRequest();','        galaxyNativeBootTrace("StopSceneController delay request allocation begin");' + "`n" + '        StopSceneDelayRequest* delayRequest = new StopSceneDelayRequest();' + "`n" + '        galaxyNativeBootTrace("StopSceneController delay request allocation returned");')
$stopText=$stopText.Replace('    MR::connectToScene(this, MR::MovementType_StopSceneDelayRequest, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);','    galaxyNativeBootTrace("StopSceneDelayRequest scene connection begin");' + "`n" + '    MR::connectToScene(this, MR::MovementType_StopSceneDelayRequest, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);' + "`n" + '    galaxyNativeBootTrace("StopSceneDelayRequest scene connection returned");')
$stopText=$stopText.Replace('        delayRequest->initWithoutIter();','        delayRequest->initWithoutIter();' + "`n" + '        galaxyNativeBootTrace("StopSceneController delay request init returned");')
$stopText=$stopText.Replace('        mDelayRequestArray->registerObj(delayRequest);','        mDelayRequestArray->registerObj(delayRequest);' + "`n" + '        galaxyNativeBootTrace("StopSceneController delay request registered");')
$stopText=$stopText.Replace('    }' + "`n" + '}' + "`n" + "`n" + 'void StopSceneDelayRequest::movement() {','    }' + "`n" + '    galaxyNativeBootTrace("StopSceneController constructor body returned");' + "`n" + '}' + "`n" + "`n" + 'void StopSceneDelayRequest::movement() {')
[IO.File]::WriteAllText($stopSource,$stopText,[Text.UTF8Encoding]::new($false))
$oldStopSource='"'+$root+'\Petari\src\Game\Scene\StopSceneController.cpp"'
$nameRegisterSourceOriginal=Join-Path $root 'Petari\src\Game\NameObj\NameObjRegister.cpp'
$nameRegisterSource=Join-Path $tmp 'NameObjRegister-substage-traced.cpp'
$nameRegisterText=Get-Content -LiteralPath $nameRegisterSourceOriginal -Raw
$nameRegisterText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $nameRegisterText
$nameRegisterText=$nameRegisterText.Replace('void NameObjRegister::add(NameObj* pObj) {','void NameObjRegister::add(NameObj* pObj) {' + "`n" + '    { char line[160]; std::snprintf(line,sizeof(line),"NameObjRegister add holder=%p object=%p",static_cast<void*>(mHolder),static_cast<void*>(pObj)); galaxyNativeBootTrace(line); }')
$nameRegisterText=$nameRegisterText.Replace('    mHolder->add(pObj);','    galaxyNativeBootTrace("NameObjRegister holder add begin");' + "`n" + '    mHolder->add(pObj);' + "`n" + '    galaxyNativeBootTrace("NameObjRegister holder add returned");')
[IO.File]::WriteAllText($nameRegisterSource,$nameRegisterText,[Text.UTF8Encoding]::new($false))
$oldNameRegisterSource='"'+$root+'\Petari\src\Game\NameObj\NameObjRegister.cpp"'
$nameExecSourceOriginal=Join-Path $root 'Petari\src\Game\NameObj\NameObjExecuteHolder.cpp'
$nameExecSource=Join-Path $tmp 'NameObjExecuteHolder-substage-traced.cpp'
$nameExecText=Get-Content -LiteralPath $nameExecSourceOriginal -Raw
$nameExecText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $nameExecText
$nameExecText=$nameExecText.Replace('void NameObjExecuteHolder::registerActor(NameObj* pObj, int movementType, int calcAnimType, int drawBufferType, int drawType) {','void NameObjExecuteHolder::registerActor(NameObj* pObj, int movementType, int calcAnimType, int drawBufferType, int drawType) {' + "`n" + '    { char line[180]; std::snprintf(line,sizeof(line),"NameObjExecuteHolder register index=%d max=%d actor=%p",mExecuteArraySize,mExecuteArrayMaxSize,static_cast<void*>(pObj)); galaxyNativeBootTrace(line); }')
$nameExecText=$nameExecText.Replace('    mExecuteArray[mExecuteArraySize].setConnectInfo(pObj, movementType, calcAnimType, drawBufferType, drawType);','    galaxyNativeBootTrace("NameObjExecuteHolder setConnectInfo begin");' + "`n" + '    mExecuteArray[mExecuteArraySize].setConnectInfo(pObj, movementType, calcAnimType, drawBufferType, drawType);' + "`n" + '    galaxyNativeBootTrace("NameObjExecuteHolder setConnectInfo returned");')
$nameExecText=$nameExecText.Replace('    mExecuteArraySize++;','    mExecuteArraySize++;' + "`n" + '    galaxyNativeBootTrace("NameObjExecuteHolder registerActor returned");')
[IO.File]::WriteAllText($nameExecSource,$nameExecText,[Text.UTF8Encoding]::new($false))
$nameExecObject=Join-Path $tmp 'NameObjExecuteHolder-substage-traced.obj'
& $zig c++ @sceneControllerCommon $nameExecSource '-o' $nameExecObject
if($LASTEXITCODE){throw 'Boot-traced Petari name-object executor provider failed to compile.'}
$oldNameExecObject='"'+$native+'\build\actor-runtime-probe\NameObjExecuteHolder.obj"'
$audioSource=Join-Path $tmp 'AudSystemWrapper-boot-traced.cpp'
$audioObject=Join-Path $tmp 'AudSystemWrapper-boot-traced.obj'
$audioText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\System\AudSystemWrapper.cpp') -Raw
$audioText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $audioText
foreach($anchor in @(
 @('    receiveResourceForInitialize();','audio resource receive begin','    if (mIsResetDone) {','audio resources received'),
 @('    mAudSystem = AudNewAudSystem(mSolidHeap, mSmrRes, mJaiSeqRes, mJaiChordRes, mJaiMeRes, mJaiRemixSeqRes);','AudNewAudSystem begin','    mDisableReset = false;','reset lock cleared'),
 @('    createSoundNameConverter();','sound-name converters begin','    AudMicWrap::setMicEnv();','audio mic environment set'),
 @('    mSmrRes = MR::receiveFile(sAudioResFile);','system audio resource received','    mJaiSeqRes = MR::receiveArchive(sJaiSeqArc);','sequence archive received'),
 @('    mJaiChordRes = MR::receiveArchive(sJaiChordArc);','chord archive received','    mJaiMeRes = MR::receiveArchive(sJaiMeArc);','ME archive received'),
 @('    mSpkRes = MR::receiveArchive(AudSpeakerWrap::getResName());','speaker archive received','    mJaiRemixSeqRes = MR::receiveArchive(sJaiRemixArc);','remix archive received')
)){
 $needle=$anchor[0]
 if(!$audioText.Contains($needle)){throw "Audio callback trace anchor missing: $needle"}
 $audioText=$audioText.Replace($needle,'    galaxyNativeBootTrace("AudSystemWrapper '+$anchor[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("AudSystemWrapper '+$anchor[2]+'");')
}
$speakerCall='    mAudSystem->setSpeakerResource(mSpkRes);'
$speakerTrace='    { char line[180]; std::snprintf(line,sizeof(line),"AudSystemWrapper setSpeakerResource begin wrapper=%p audio=%p speaker=%p",static_cast<void*>(this),static_cast<void*>(mAudSystem),static_cast<void*>(mSpkRes)); galaxyNativeBootTrace(line); }' + "`n" + $speakerCall + "`n" + '    galaxyNativeBootTrace("AudSystemWrapper setSpeakerResource returned");'
$audioText=$audioText.Replace($speakerCall,$speakerTrace)
[IO.File]::WriteAllText($audioSource,$audioText,[Text.UTF8Encoding]::new($false))
$audioCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\nw4r\include",
 '-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @audioCommon $audioSource '-o' $audioObject
if($LASTEXITCODE){throw 'Boot-traced Petari AudSystemWrapper provider failed to compile.'}
$micSource=Join-Path $tmp 'AudMicWrap-boot-traced.cpp'
$micObject=Join-Path $tmp 'AudMicWrap-boot-traced.obj'
$micText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\AudioLib\AudMicWrap.cpp') -Raw
$micText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $micText
$micText=$micText.Replace('    void setMicEnv() {','    void setMicEnv() {' + "`n" + '        galaxyNativeBootTrace("AudMicWrap setMicEnv entered");')
$micText=$micText.Replace('        AudSystem* pSystem = AudWrap::getSystem();','        AudSystem* pSystem = AudWrap::getSystem();' + "`n" + '        galaxyNativeBootTrace("AudMicWrap audio system lookup returned");')
$micText=$micText.Replace('        AudWrap::getSystem()->mAudience.updateSetting();','        galaxyNativeBootTrace("AudMicWrap setting update begin");' + "`n" + '        AudWrap::getSystem()->mAudience.updateSetting();' + "`n" + '        galaxyNativeBootTrace("AudMicWrap setting update returned");')
[IO.File]::WriteAllText($micSource,$micText,[Text.UTF8Encoding]::new($false))
& $zig c++ @audioCommon $micSource '-o' $micObject
if($LASTEXITCODE){throw 'Boot-traced AudMicWrap provider failed to compile.'}
$audKernelSource=Join-Path $tmp 'AudioSystemKernel-boot-traced.cpp'
$audKernelObject=Join-Path $tmp 'AudioSystemKernel-boot-traced.obj'
$audKernelText=Get-Content -LiteralPath (Join-Path $native 'src\generated\AudioSystemKernel.cpp') -Raw
$audKernelText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $audKernelText
$speakerMethod='void AudSystem::setSpeakerResource(JKRArchive* pArchive) {'
if(!$audKernelText.Contains($speakerMethod)){throw 'Generated AudSystem speaker-resource method anchor missing.'}
$audKernelText=$audKernelText.Replace($speakerMethod,$speakerMethod + "`n" + '    { char line[180]; std::snprintf(line,sizeof(line),"AudSystem::setSpeakerResource entered this=%p spkSystem=%p archive=%p",static_cast<void*>(this),static_cast<void*>(mSpkSystem),static_cast<void*>(pArchive)); galaxyNativeBootTrace(line); }')
$audKernelText=$audKernelText.Replace('    mSpkSystem->setResource(pArchive, 0, 1);','    mSpkSystem->setResource(pArchive, 0, 1);' + "`n" + '    galaxyNativeBootTrace("AudSystem::setSpeakerResource returned");')
$constructorSteps=@(
 @('    mLimitedSoundInfo = new (mSectionHeap->getHeap(), 0) AudLimitedSoundInfo[2];','limited sound records allocate'),
 @('    clearAllLimitedSound();','limited sound initialization'),
 @('        getSeMgr().setSeqDataMgr(seSeqDataMgr);','SE sequence manager bind'),
 @('        getSeqMgr().setSeqDataMgr(seqSeqDataMgr);','music sequence manager bind'),
 @('    setAudience(&mAudience);','audience initialization'),
 @('    JAUStreamStaticAramMgr* streamAramMgr = new JAUStreamStaticAramMgr();','stream ARAM manager allocate'),
 @('    streamAramMgr->alloc(AudParams::streamAramBlocks_perChunk, JASKernel::getAramHeap());','stream ARAM buffers allocate'),
 @('    mStreamMgr.setStreamAramMgr(mStreamAramMgr);','stream manager bind'),
 @('        mSoundInfo = newSoundInfo(mSectionHeap);','sound info allocate'),
 @('        mStreamMgr.setStreamDataMgr(streamDataMgr);','stream data manager bind'),
 @('    initCategoryArrangement();','category arrangement initialize'),
 @('    initVolumeSetting();','volume setting initialize'),
 @('    mSoundObjHolder = new (mSectionHeap->getHeap(), 0) AudSoundObjHolder(mSectionHeap->getHeap(), AudParams::numInspectableSoundObj);','sound object holder allocate'),
 @('    mSystemSeObject = new (mSectionHeap->getHeap(), 0) AudSoundObject(nullptr, 10, mSectionHeap->getHeap());','system SE object allocate'),
 @('    mAtmosphereSeObject = new (mSectionHeap->getHeap(), 0) AudSoundObject(nullptr, 8, mSectionHeap->getHeap());','atmosphere SE object allocate'),
 @('    mSceneMgr = new (mSectionHeap->getHeap(), 0) AudSceneMgr(mSectionHeap);','scene manager allocate'),
 @('    mRhythmMeSystem = new (mSectionHeap->getHeap(), 0) AudRhythmMeSystem(mSectionHeap->getHeap(), 3, true);','rhythm ME system allocate'),
 @('    mRemixMgr = new (mSectionHeap->getHeap(), 0) AudRemixMgr(mSectionHeap->getHeap());','remix manager allocate'),
 @('    mRemixMgr->init();','remix manager initialize'),
 @('    mSeStrategyMgr = new (mSectionHeap->getHeap(), 0) AudSeStrategyMgr();','SE strategy manager allocate'),
 @('    mSeMgr.setStrategyMgr(mSeStrategyMgr);','SE strategy manager bind'),
 @('    mAudEffector = new (mSectionHeap->getHeap(), 0) AudEffector();','audio effector allocate'),
 @('    mAudEffector->initFxLine(mSectionHeap->getHeap());','audio effect line initialize'),
 @('        mRhythmMeSystem->mChordInfo.init(pChordArchive);','chord archive initialize'),
 @('        setMeResource(pMeArchive, 1, 0);','ME archive initialize'),
 @('        mRemixMgr->setRemixSeqResource(remixResource, remixResourceSize > 0 ? u32(remixResourceSize) : 0);','remix sequence initialize'),
 @('    mSystemMeObject = new (mSectionHeap->getHeap(), 0) AudMeObject(nullptr, 8, mSectionHeap->getHeap());','system ME object allocate'),
 @('    mSpkSystem = new (mSectionHeap->getHeap(), 0) SpkSystem(mSectionHeap->getHeap());','speaker system allocate'),
 @('    mVolumeController = new (mSectionHeap->getHeap(), 0) AudSystemVolumeController(this);','volume controller allocate')
)
foreach($step in $constructorSteps){$needle=$step[0];if(!$audKernelText.Contains($needle)){throw "AudSystem constructor trace anchor missing: $needle"};$detail=$step[1];$prefix='    galaxyNativeBootTrace("AudSystem before '+$detail+'");';$suffix='    galaxyNativeBootTrace("AudSystem after '+$detail+'");';$audKernelText=$audKernelText.Replace($needle,$prefix+"`n"+$needle+"`n"+$suffix)}
$meTableLookup='    void* meTable = pArchive->getResource(a1);'
if(!$audKernelText.Contains($meTableLookup)){throw 'ME-table resource trace anchor missing.'}
$meTableTrace=@'
    { auto* b=static_cast<const unsigned char*>(meTable); char line[220]; std::snprintf(line,sizeof(line),"ME resource archive=%p index=%u ptr=%p header=%08X,%08X,%08X",static_cast<void*>(pArchive),unsigned(a1),meTable,b?(unsigned(b[0])<<24|unsigned(b[1])<<16|unsigned(b[2])<<8|b[3]):0,b?(unsigned(b[4])<<24|unsigned(b[5])<<16|unsigned(b[6])<<8|b[7]):0,b?(unsigned(b[8])<<24|unsigned(b[9])<<16|unsigned(b[10])<<8|b[11]):0); galaxyNativeBootTrace(line); }
'@
$audKernelText=$audKernelText.Replace($meTableLookup,$meTableLookup+"`n"+$meTableTrace.TrimEnd())
$setMeTableCall='    mRhythmMeSystem->mMeMgr->setMeTable(meTable);'
if(!$audKernelText.Contains($setMeTableCall)){throw 'ME-table setter trace anchor missing.'}
$audKernelText=$audKernelText.Replace($setMeTableCall,'    galaxyNativeBootTrace("ME table setter begin");' + "`n" + $setMeTableCall + "`n" + '    galaxyNativeBootTrace("ME table setter returned");')
$audKernelText=$audKernelText.Replace('        initJAS.initJASystem(pHeap);','        galaxyNativeBootTrace("AudNewAudSystem initJASystem begin");' + "`n" + '        initJAS.initJASystem(pHeap);' + "`n" + '        galaxyNativeBootTrace("AudNewAudSystem initJASystem returned");')
$audKernelText=$audKernelText.Replace('        initJAI.initJAInterface();','        galaxyNativeBootTrace("AudNewAudSystem initJAInterface begin");' + "`n" + '        initJAI.initJAInterface();' + "`n" + '        galaxyNativeBootTrace("AudNewAudSystem initJAInterface returned");')
foreach($stage in @(
 @('    int maxNumSeq = ::init_JAS_JAI_(pHeap);','AudNewAudSystem init JAS begin','    JAUSectionHeap* newHeap = JAUNewSectionHeap(true);','AudNewAudSystem new section heap begin'),
 @('        loader.load(pV);','AudNewAudSystem audio archive load begin','        newHeap->getHeap()->freeTail();','AudNewAudSystem audio archive load complete'),
 @('        newHeap->setSeqDataArchive(pSeqArchive);','AudNewAudSystem sequence archive assign begin','        maxSeqDataSize = AudParams::maxSeqDataSize;','AudNewAudSystem sequence archive assigned'),
 @('    AudSystem* system = AudNewAudSystem_(newHeap, pChordArchive, pMeArchive, pRemixArchive, 1);','AudNewAudSystem system construct begin','    if (maxSeqDataSize != 0) {','AudNewAudSystem system construct complete'),
 @('        newHeap->newDynamicSeqBlocks(maxSeqDataSize, maxNumSeq);','AudNewAudSystem sequence blocks begin','    currHeap->becomeCurrentHeap();','AudNewAudSystem sequence blocks complete'),
 @('    newHeap->finishBuild();','AudNewAudSystem finish heap begin','    return system;','AudNewAudSystem finish heap complete'),
 @('        initJAI.initJAInterface();','initJAI interface init begin','        AudDopplerAudible< 4 >::newMemPool(initJAI.mMaxNumSe);','initJAI interface init complete'),
 @('    newHeap->getHeap()->becomeCurrentHeap();','AudNewAudSystem current heap switch begin','    if (pV == nullptr) {','AudNewAudSystem current heap switch complete'),
 @('            JKRArchive* seqArc = JKRArchive::mount(sJaiSeqArc, JKRArchive::MOUNT_MODE_DVD, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);','AudNewAudSystem DVD sequence archive mount begin','            newHeap->setSeqDataArchive(seqArc);','AudNewAudSystem DVD sequence archive mount complete'),
 @('        pChordArchive = JKRArchive::mount(sJaiChordArc, JKRArchive::MOUNT_MODE_MEM, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);','AudNewAudSystem chord archive mount begin','    }','AudNewAudSystem chord archive mount complete'),
 @('        pMeArchive = JKRArchive::mount(sJaiMeArc, JKRArchive::MOUNT_MODE_MEM, newHeap->getHeap(), JKRArchive::MOUNT_DIRECTION_1);','AudNewAudSystem ME archive mount begin','    }','AudNewAudSystem ME archive mount complete')
)){
 $needle=$stage[0]
 if($audKernelText.Contains($needle)){$audKernelText=$audKernelText.Replace($needle,'    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("'+$stage[3]+'");')}
}
[IO.File]::WriteAllText($audKernelSource,$audKernelText,[Text.UTF8Encoding]::new($false))
& $zig c++ @audioCommon $audKernelSource '-o' $audKernelObject
if($LASTEXITCODE){throw 'Boot-traced native AudSystem provider failed to compile.'}
$sequenceBlockCall='        newHeap->newDynamicSeqBlocks(maxSeqDataSize, maxNumSeq);'
if(!$audKernelText.Contains($sequenceBlockCall)){throw 'AudNewAudSystem sequence-block call trace anchor missing.'}
$sequenceBlockCallTrace=@'
        { char line[240]; std::snprintf(line,sizeof(line),"AudNewAudSystem seq-block args size=%u count=%d heap=%p",maxSeqDataSize,maxNumSeq,static_cast<void*>(newHeap->getHeap())); galaxyNativeBootTrace(line); }
'@
$audKernelText=$audKernelText.Replace($sequenceBlockCall,$sequenceBlockCallTrace.TrimEnd()+"`n"+$sequenceBlockCall)
[IO.File]::WriteAllText($audKernelSource,$audKernelText,[Text.UTF8Encoding]::new($false))
& $zig c++ @audioCommon $audKernelSource '-o' $audKernelObject
if($LASTEXITCODE){throw 'Instrumented native AudSystem sequence-block provider failed to compile.'}
$sectionHeapSource=Join-Path $tmp 'AudioSectionHeapKernel-boot-traced.cpp'
$sectionHeapText=Get-Content -LiteralPath (Join-Path $native 'src\generated\AudioSectionHeapKernel.cpp') -Raw
$sectionHeapText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $sectionHeapText
$dynamicBlock='bool JAUSectionHeap::newDynamicSeqBlock(u32 size) {'
if(!$sectionHeapText.Contains($dynamicBlock)){throw 'Dynamic sequence-block entry trace anchor missing.'}
$sectionHeapText=$sectionHeapText.Replace($dynamicBlock,$dynamicBlock + "`n" + '    { char line[200]; std::snprintf(line,sizeof(line),"JAUSectionHeap newDynamicSeqBlock size=%u heap=%p start=%p end=%p free=%d",size,static_cast<void*>(mHeap),mHeap?static_cast<void*>(mHeap->getStart()):nullptr,mHeap?static_cast<void*>(mHeap->getEnd()):nullptr,mHeap?mHeap->getFreeSize():0); galaxyNativeBootTrace(line); }')
$dynamicBlocks='int JAUSectionHeap::newDynamicSeqBlocks(u32 size, int n) {'
if(!$sectionHeapText.Contains($dynamicBlocks)){throw 'Dynamic sequence-block batch trace anchor missing.'}
$sectionHeapText=$sectionHeapText.Replace($dynamicBlocks,$dynamicBlocks + "`n" + '    { char line[160]; std::snprintf(line,sizeof(line),"JAUSectionHeap newDynamicSeqBlocks size=%u count=%d",size,n); galaxyNativeBootTrace(line); }')
[IO.File]::WriteAllText($sectionHeapSource,$sectionHeapText,[Text.UTF8Encoding]::new($false))
$remixManagerObject=Join-Path $tmp 'AudRemixMgr-boot-compatible.obj'
& $zig c++ @audioCommon (Join-Path $native 'src\generated\AudRemixMgr.cpp') '-o' $remixManagerObject
if($LASTEXITCODE){throw 'Native Remix manager provider failed to compile.'}
$jauSource=Join-Path $tmp 'JAUInitializer-boot-traced.cpp'
$jauObject=Join-Path $tmp 'JAUInitializer-boot-traced.obj'
$jauText=Get-Content -LiteralPath (Join-Path $native 'src\generated\JAUInitializer.cpp') -Raw
$jauText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $jauText
foreach($stage in @(
 @('    JASKernel::setupRootHeap(pHeap, heapSize_);','JAS setup root heap begin','JAS setup root heap returned'),
 @('    JASKernel::setupAramHeap(audioMemory_, audioMemSize_);','JAS setup ARAM heap begin','JAS setup ARAM heap returned'),
 @('    JASTrack::newMemPool(field_0x1c);','JAS track mempool begin','JAS track mempool returned'),
 @('    JASDvd::createThread(dvdThreadPriority_, 0x80, 0x1000);','JAS DVD thread create begin','JAS DVD thread create returned'),
 @('    JASAudioThread::create(audioThreadPriority_);','JAS audio thread create begin','JAS audio thread create returned'),
 @('    JASDriver::setDSPLevel(dspLevel_);','JAS DSP level set begin','JAS DSP level set returned'),
 @('    JASAramStream::initSystem(aramBlockSize_, aramChannelNum_);','JAS ARAM stream init begin','JAS ARAM stream init returned'),
 @('    s32 r30 = JASDram->getFreeSize();','JAI mempool init begin','JAI mempool init returned'),
 @('    JAIStream::newMemPool(mMaxNumStream);','JAI stream pool begin','JAI stream pool returned'),
 @('    JAISeq::newMemPool(mMaxNumSeq);','JAI sequence pool begin','JAI sequence pool returned'),
 @('    JAISe::newMemPool(mMaxNumSe);','JAI sound-effect pool begin','JAI sound-effect pool returned'),
 @('    JAISoundChild::newMemPool(mMaxNumChildren);','JAI sound-child pool begin','JAI sound-child pool returned')
)){
 $needle=$stage[0]
 if(!$jauText.Contains($needle)){continue}
 $jauText=$jauText.Replace($needle,'    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("'+$stage[2]+'");')
}
[IO.File]::WriteAllText($jauSource,$jauText,[Text.UTF8Encoding]::new($false))
& $zig c++ @audioCommon $jauSource '-o' $jauObject
if($LASTEXITCODE){throw 'Boot-traced native JAUInitializer provider failed to compile.'}
$audioThreadSource=Join-Path $tmp 'AudioThreadKernel-boot-traced.cpp'
$audioThreadText=Get-Content -LiteralPath (Join-Path $native 'src\generated\AudioThreadKernel.cpp') -Raw
$audioThreadText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $audioThreadText
foreach($stage in @(
 @('    pAudioThread->resume();','JAS audio worker resume begin','JAS audio worker resume returned'),
 @('    std::unique_lock<std::mutex> lock(sAudioThreadStartupMutex);','JAS audio worker DSP-ready wait begin','JAS audio worker DSP-ready lock acquired'),
 @('    JASDriver::initAI(DMACallback);','JAS audio init AI begin','JAS audio init AI returned'),
 @('    JASDsp::boot(DSPCallback);','JAS DSP boot begin','JAS DSP boot returned'),
 @('    JASDsp::initBuffer();','JAS DSP buffer init begin','JAS DSP buffer init returned'),
 @('    JASDSPChannel::initAll();','JAS DSP channels init begin','JAS DSP channels init returned'),
 @('    JASPoolAllocObject_MultiThreaded< JASChannel >::newMemPool(0x48);','JAS channel pool init begin','JAS channel pool init returned'),
 @('    JASDriver::startDMA();','JAS audio DMA start begin','JAS audio DMA start returned')
)){
 $needle=$stage[0]
 if(!$audioThreadText.Contains($needle)){continue}
 $audioThreadText=$audioThreadText.Replace($needle,'    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle + "`n" + '    galaxyNativeBootTrace("'+$stage[2]+'");')
}
$audioRun='void* JASAudioThread::run() {'
if($audioThreadText.Contains($audioRun)){$audioThreadText=$audioThreadText.Replace($audioRun,$audioRun + "`n" + '    galaxyNativeBootTrace("JAS audio worker run entered");')}
[IO.File]::WriteAllText($audioThreadSource,$audioThreadText,[Text.UTF8Encoding]::new($false))
$hostDspSource=Join-Path $tmp 'AudioDSPInterfaceHost-boot-traced.cpp'
$hostDspObject=Join-Path $tmp 'AudioDSPInterfaceHost-boot-traced.obj'
$hostDspText=Get-Content -LiteralPath (Join-Path $native 'src\generated\AudioDSPInterfaceHost.cpp') -Raw
$hostDspText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $hostDspText
$hostDspText=$hostDspText.Replace('        DspBoot(param_0);','        galaxyNativeBootTrace("JASDsp::boot DspBoot begin");' + "`n" + '        DspBoot(param_0);' + "`n" + '        galaxyNativeBootTrace("JASDsp::boot DspBoot returned");')
$hostDspText=$hostDspText.Replace('        DsetVARAM(JKRHeap::getAltAramStartAdr());','        galaxyNativeBootTrace("JASDsp::boot DsetVARAM begin");' + "`n" + '        DsetVARAM(JKRHeap::getAltAramStartAdr());' + "`n" + '        galaxyNativeBootTrace("JASDsp::boot DsetVARAM returned");')
[IO.File]::WriteAllText($hostDspSource,$hostDspText,[Text.UTF8Encoding]::new($false))
$hostDspCommon=$audioCommon+@('-include',"$native\include\Native\HeapPlacement.hpp")
& $zig c++ @hostDspCommon $hostDspSource '-o' $hostDspObject
if($LASTEXITCODE){throw 'Boot-traced host DSP interface provider failed to compile.'}
$decompObject=Join-Path $tmp 'JKRDecomp.obj'
$decompSource=Join-Path $tmp 'JKRDecomp-boot-traced.cpp'
$decompText=Get-Content -LiteralPath (Join-Path $native 'src\generated\JKRDecomp.cpp') -Raw
if($false){
$decompText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $decompText
$decompAnchors=@(
 @('JKRDecomp* JKRDecomp::create(long a1) {','JKRDecomp create begin'),
 @('        sDecompObject = new (JKRHeap::sSystemHeap, 0) JKRDecomp(a1);','JKRDecomp thread object allocated'),
 @('JKRDecomp::JKRDecomp(long a1) : JKRThread(0x4000, 0x10, a1) {','JKRDecomp constructor begin'),
 @('JKRDecomp::~JKRDecomp() {','JKRDecomp destructor entered'),
 @('bool JKRDecomp::orderSync(unsigned char* pSrc, unsigned char* pDst, u32 compressedSize, u32 decompressedSize) {','JKRDecomp orderSync begin'),
 @('    OSSendMessage(&sMessageQueue, command, OS_MESSAGE_NOBLOCK);','JKRDecomp command queued'),
 @('    bool received = sync(command, 0);','JKRDecomp command wait begin')
)
foreach($anchor in $decompAnchors){$needle=$anchor[0];if(!$decompText.Contains($needle)){throw "JKRDecomp trace anchor missing: $needle"};$replacement=$needle + "`n" + '    galaxyNativeBootTrace("'+$anchor[1]+'");';$decompText=$decompText.Replace($needle,$replacement)}
$decompText=$decompText.Replace('        sDecompObject = new (JKRHeap::sSystemHeap, 0) JKRDecomp(a1);','        sDecompObject = new (JKRHeap::sSystemHeap, 0) JKRDecomp(a1);' + "`n" + '        galaxyNativeBootTrace("JKRDecomp constructor returned");')
$decompText=$decompText.Replace('    OSResumeThread(mThread);','    OSResumeThread(mThread);' + "`n" + '    galaxyNativeBootTrace("JKRDecomp worker resumed");')
$decompText=$decompText.Replace('    bool received = sync(command, 0);','    bool received = sync(command, 0);' + "`n" + '    galaxyNativeBootTrace("JKRDecomp command wait complete");')
$decompText=$decompText.Replace('    EJKRCompression compression = checkCompressed(pSrc);','    galaxyNativeBootTrace("JKRDecomp decode begin");' + "`n" + '    EJKRCompression compression = checkCompressed(pSrc);')
$decompText=$decompText.Replace('    } else if (compression == JKR_COMPRESSION_SZS) {' + "`n" + '        decodeSZS(pSrc, pDst, compressedSize, decompressedSize);' + "`n" + '    }','    } else if (compression == JKR_COMPRESSION_SZS) {' + "`n" + '        decodeSZS(pSrc, pDst, compressedSize, decompressedSize);' + "`n" + '    }' + "`n" + '    galaxyNativeBootTrace("JKRDecomp decode complete");')
}
$decompCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h', '-c')
& $zig c++ @decompCommon (Join-Path $native 'src\generated\JKRDecomp.cpp') '-o' $decompObject
if($LASTEXITCODE){throw 'Boot-traced native JKRDecomp provider failed to compile.'}
$fileRipperObject=Join-Path $tmp 'FileRipper.obj'
$fileRipperSource=Join-Path $tmp 'FileRipper-boot-traced.cpp'
$fileRipperText=Get-Content -LiteralPath (Join-Path $native 'src\generated\FileRipper.cpp') -Raw
$fileRipperText=$fileRipperText.Replace('#include "../native_jkr_archive_port.hpp"','#include "'+$native+'\src\native_jkr_archive_port.hpp"')
$fileRipperText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $fileRipperText
$fileRipperText=$fileRipperText.Replace('void* FileRipper::loadToMainRAM(const char* fpath, u8* dest, bool decompress, JKRHeap* pHeap, AllocDirection allocDir) {','void* FileRipper::loadToMainRAM(const char* fpath, u8* dest, bool decompress, JKRHeap* pHeap, AllocDirection allocDir) {' + "`n" + '    char nativeRipperPath[280]; std::snprintf(nativeRipperPath,sizeof(nativeRipperPath),"FileRipper load path=%s decompress=%d",fpath?fpath:"<null>",decompress?1:0); galaxyNativeBootTrace(nativeRipperPath);')
$fileRipperText=$fileRipperText.Replace('bool FileRipper::decompressFromDVD(DVDFileInfo* fileInfo, void* dest, u32 readSize, u32 destSize, const u8* currentPos, u32 copySize) {','bool FileRipper::decompressFromDVD(DVDFileInfo* fileInfo, void* dest, u32 readSize, u32 destSize, const u8* currentPos, u32 copySize) {' + "`n" + '    char nativeDecompStart[220]; std::snprintf(nativeDecompStart,sizeof(nativeDecompStart),"decompressFromDVD begin srcFile=%p dest=%p readSize=%u destSize=%u current=%p copy=%u buffer=%p end=%p",static_cast<void*>(fileInfo),dest,readSize,destSize,currentPos,copySize,static_cast<void*>(::sReadBuffer),static_cast<void*>(::sReadBufferEnd)); galaxyNativeBootTrace(nativeDecompStart);')
$fileRipperText=$fileRipperText.Replace('    do {' + "`n" + '        if (!group_count) {' + "`n" + '            if (src > ::sReadBufferLimit && ::sReadDvdLeftSize) {','    do {' + "`n" + '        if (!group_count) {' + "`n" + '            if (src > ::sReadBufferLimit && !::sReadDvdLeftSize) { char nativeEof[180]; std::snprintf(nativeEof,sizeof(nativeEof),"Yaz0 exhausted src=%p limit=%p end=%p dst=%p dstEnd=%p",static_cast<void*>(src),static_cast<void*>(::sReadBufferLimit),static_cast<void*>(::sReadBufferEnd),static_cast<void*>(dest),static_cast<void*>(dest_end)); galaxyNativeBootTrace(nativeEof); }' + "`n" + '            if (src > ::sReadBufferLimit && ::sReadDvdLeftSize) {' + "`n" + '                char nativeRefill[200]; std::snprintf(nativeRefill,sizeof(nativeRefill),"Yaz0 refill src=%p limit=%p end=%p left=%u offset=%d",static_cast<void*>(src),static_cast<void*>(::sReadBufferLimit),static_cast<void*>(::sReadBufferEnd),::sReadDvdLeftSize,::sReadDvdOffset); galaxyNativeBootTrace(nativeRefill);')
$fileRipperText=$fileRipperText.Replace('u8* FileRipper::readSrcDataFirst() {' + "`n" + '    u8* readBuf = ::sReadBuffer;','u8* FileRipper::readSrcDataFirst() {' + "`n" + '    u8* readBuf = ::sReadBuffer;')
$fileRipperText=$fileRipperText.Replace('    if (::sReadDvdLeftSize < readSize) {' + "`n" + '        readSize = ::sReadDvdLeftSize;' + "`n" + '    }' + "`n" + '    while (true) {' + "`n" + '        s32 result = DVDReadPrio(::sSrcFileInfo, readBuf, readSize, ::sReadDvdOffset, 2);','    if (::sReadDvdLeftSize < readSize) {' + "`n" + '        readSize = ::sReadDvdLeftSize;' + "`n" + '    }' + "`n" + '    { char nativeRead[200]; std::snprintf(nativeRead,sizeof(nativeRead),"Yaz0 first read offset=%d request=%u left=%u buffer=%p",::sReadDvdOffset,readSize,::sReadDvdLeftSize,static_cast<void*>(readBuf)); galaxyNativeBootTrace(nativeRead); }' + "`n" + '    while (true) {' + "`n" + '        s32 result = DVDReadPrio(::sSrcFileInfo, readBuf, readSize, ::sReadDvdOffset, 2);' + "`n" + '        { char nativeResult[160]; std::snprintf(nativeResult,sizeof(nativeResult),"Yaz0 first read result=%d requested=%u",result,readSize); galaxyNativeBootTrace(nativeResult); }')
$fileRipperText=$fileRipperText.Replace('u8* FileRipper::readSrcDataNext(u8* buf) {' + "`n" + '    u32 size = ::sReadBufferEnd - buf;','u8* FileRipper::readSrcDataNext(u8* buf) {' + "`n" + '    u32 size = ::sReadBufferEnd - buf;')
$fileRipperText=$fileRipperText.Replace('    if (::sReadDvdLeftSize < readSize) {' + "`n" + '        readSize = ::sReadDvdLeftSize;' + "`n" + '    }' + "`n" + '    while (true) {' + "`n" + '        s32 result = DVDReadPrio(::sSrcFileInfo, readBuf, readSize, ::sReadDvdOffset, 2);','    if (::sReadDvdLeftSize < readSize) {' + "`n" + '        readSize = ::sReadDvdLeftSize;' + "`n" + '    }' + "`n" + '    { char nativeRead[240]; std::snprintf(nativeRead,sizeof(nativeRead),"Yaz0 next read src=%p start=%p readBuf=%p offset=%d request=%u left=%u",static_cast<void*>(buf),static_cast<void*>(start),static_cast<void*>(readBuf),::sReadDvdOffset,readSize,::sReadDvdLeftSize); galaxyNativeBootTrace(nativeRead); }' + "`n" + '    while (true) {' + "`n" + '        s32 result = DVDReadPrio(::sSrcFileInfo, readBuf, readSize, ::sReadDvdOffset, 2);' + "`n" + '        { char nativeResult[160]; std::snprintf(nativeResult,sizeof(nativeResult),"Yaz0 next read result=%d requested=%u",result,readSize); galaxyNativeBootTrace(nativeResult); }')
[IO.File]::WriteAllText($fileRipperSource,$fileRipperText,[Text.UTF8Encoding]::new($false))
& $zig c++ @gameSystemCommon $fileRipperSource '-o' $fileRipperObject
if($LASTEXITCODE){throw 'Boot-traced native FileRipper provider failed to compile.'}
$displayCopyObject=Join-Path $tmp 'gx_display_copy_port-boot-traced.obj'
$displayCopySource=Join-Path $tmp 'gx_display_copy_port-boot-traced.cpp'
$gxStateHeaderSource=Join-Path $native 'include\Native\GxCommandState.hpp'
$gxStateHeaderTemp=Join-Path $tmp 'GxCommandState-boot-traced.hpp'
$gxStateHeaderText=Get-Content -LiteralPath $gxStateHeaderSource -Raw
$gxStateHeaderText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $gxStateHeaderText
[IO.File]::WriteAllText($gxStateHeaderTemp,$gxStateHeaderText,[Text.UTF8Encoding]::new($false))
$displayCopyText=Get-Content -LiteralPath (Join-Path $native 'src\gx_display_copy_port.cpp') -Raw
$displayCopyText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $displayCopyText
$displayCopyText=$displayCopyText.Replace('#include <Native/GxCommandState.hpp>','#include "'+$gxStateHeaderTemp+'"')
$displayCopyText=$displayCopyText.Replace('extern "C" void GXCopyDisp(void* destination,GXBool clear){','extern "C" void GXCopyDisp(void* destination,GXBool clear){' + "`n" + ' char line[180];std::snprintf(line,sizeof(line),"GXCopyDisp entered binding=%p dest=%p clear=%u",static_cast<void*>(activeBinding),destination,unsigned(clear));galaxyNativeBootTrace(line);')
$displayCopyText=$displayCopyText.Replace('void observeDisplayCopyCommands(const std::uint8_t* start,std::size_t size,bool reset){if(activeBinding)activeBinding->observe(start,size,reset);}','void observeDisplayCopyCommands(const std::uint8_t* start,std::size_t size,bool reset){char line[160];std::snprintf(line,sizeof(line),"Display observer dispatch start=%p size=%zu reset=%d",static_cast<const void*>(start),size,reset?1:0);galaxyNativeBootTrace(line);if(activeBinding)activeBinding->observe(start,size,reset);galaxyNativeBootTrace("Display observer dispatch returned");}')
$displayCopyText=$displayCopyText.Replace('void DisplayCopyBinding::observe(const std::uint8_t* start,std::size_t size,bool reset){','void DisplayCopyBinding::observe(const std::uint8_t* start,std::size_t size,bool reset){' + "`n" + ' char line[180];std::snprintf(line,sizeof(line),"Display observe entered start=%p size=%zu reset=%d observed=%zu",static_cast<const void*>(start),size,reset?1:0,observedBytes);galaxyNativeBootTrace(line);')
$displayCopyText=$displayCopyText.Replace(' if(size>observedBytes){state->consume(start+observedBytes,size-observedBytes,resources);observedBytes=size;}',' if(size>observedBytes){galaxyNativeBootTrace("Display observe consume begin");state->consume(start+observedBytes,size-observedBytes,resources);galaxyNativeBootTrace("Display observe consume returned");observedBytes=size;}')
$displayCopyText=$displayCopyText.Replace(' state->clearPendingCopies();state->clearPendingSyncEvents();',' galaxyNativeBootTrace("Display observe clear pending begin");state->clearPendingCopies();state->clearPendingSyncEvents();galaxyNativeBootTrace("Display observe clear pending returned");')
[IO.File]::WriteAllText($displayCopySource,$displayCopyText,[Text.UTF8Encoding]::new($false))
& $zig c++ @gameSystemCommon $displayCopySource '-o' $displayCopyObject
if($LASTEXITCODE){throw 'Boot-traced native GX display-copy observer provider failed to compile.'}
$resourceManagerObject=Join-Path $tmp 'ResourceHolderManager.obj'
$resourceManagerSource=Join-Path $tmp 'ResourceHolderManager-boot-traced.cpp'
$resourceManagerText=Get-Content -LiteralPath (Join-Path $native 'src\generated\ResourceHolderManager.cpp') -Raw
$resourceManagerText='#include <cstdio>' + "`n" + '#include <windows.h>' + "`n" + '#include <malloc.h>' + "`n" + '#include <JSystem/JKernel/JKRExpHeap.hpp>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + 'static void galaxyNativeThreadTrace(const char* text) { char line[256]; std::snprintf(line,sizeof(line),"%s tid=%lu",text,static_cast<unsigned long>(GetCurrentThreadId())); galaxyNativeBootTrace(line); }' + "`n" + $resourceManagerText
$resourceManagerText=$resourceManagerText.Replace('ResourceHolderManager::ResourceHolderManager() {','ResourceHolderManager::ResourceHolderManager() {' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"ResourceManager ctor entered self=%p gddr=%p start=%p end=%p size=%u",static_cast<void*>(this),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$resourceManagerText=$resourceManagerText.Replace('    mResourceArray.clear();','    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"ResourceManager before clear self=%p gddr=%p start=%p end=%p size=%u",static_cast<void*>(this),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + '    mResourceArray.clear();' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[200]; std::snprintf(line,sizeof(line),"ResourceManager after clear self=%p gddr=%p start=%p end=%p size=%u",static_cast<void*>(this),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$resourceManagerText=$resourceManagerText.Replace('    mResourceArray.push_back(name2Resource);','    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[280]; std::snprintf(line,sizeof(line),"ResourceManager add before this=%p array=%p begin=%p count=%d cap=%d heap=%p start=%p end=%p size=%u",static_cast<void*>(this),static_cast<void*>(&mResourceArray),static_cast<void*>(mResourceArray.begin()),mResourceArray.size(),mResourceArray.capacity(),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + '    mResourceArray.push_back(name2Resource);' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[280]; std::snprintf(line,sizeof(line),"ResourceManager add after this=%p array=%p begin=%p count=%d cap=%d heap=%p start=%p end=%p size=%u",static_cast<void*>(this),static_cast<void*>(&mResourceArray),static_cast<void*>(mResourceArray.begin()),mResourceArray.size(),mResourceArray.capacity(),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$resourceManagerText=$resourceManagerText.Replace('    MR::getMountedArchiveAndHeap(pParam1, &pArchive, &pArgs->mHeap);','    MR::getMountedArchiveAndHeap(pParam1, &pArchive, &pArgs->mHeap);' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[280]; std::snprintf(line,sizeof(line),"layout archive lookup tid=%lu archive=%p chosenHeap=%p gddr=%p start=%p end=%p size=%u",static_cast<unsigned long>(GetCurrentThreadId()),static_cast<void*>(pArchive),static_cast<void*>(pArgs->mHeap),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$layoutHeapRestore='    MR::CurrentHeapRestorer heapRestorer(pArgs->mHeap);'
if(!$resourceManagerText.Contains($layoutHeapRestore)){throw 'LayoutHolder current-heap restore trace anchor missing.'}
$resourceManagerText=$resourceManagerText.Replace($layoutHeapRestore,'    galaxyNativeThreadTrace("LayoutHolder current-heap restore begin");' + "`n" + $layoutHeapRestore + "`n" + '    galaxyNativeThreadTrace("LayoutHolder current-heap restore complete");' + "`n" + '    galaxyNativeThreadTrace("LayoutHolder allocation begin");')
$resourceManagerText=$resourceManagerText.Replace('    pArgs->mLayoutHolder = new LayoutHolder(*pArchive);','    pArgs->mLayoutHolder = new LayoutHolder(*pArchive);' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[280]; std::snprintf(line,sizeof(line),"layout object created tid=%lu layout=%p gddr=%p start=%p end=%p size=%u",static_cast<unsigned long>(GetCurrentThreadId()),static_cast<void*>(pArgs->mLayoutHolder),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$layoutMethod='LayoutHolder* ResourceHolderManager::createAndAddLayoutHolderRawData(const char* pParam1) {'
if(!$resourceManagerText.Contains($layoutMethod)){throw 'ResourceHolderManager raw-layout trace anchor missing.'}
$resourceManagerText=$resourceManagerText.Replace($layoutMethod,$layoutMethod + "`n" + '    galaxyNativeBootTrace("ResourceHolderManager raw-layout begin");')
foreach($stage in @(@('    (this->*create)(pParam1, &args);','ResourceHolderManager createLayoutHolder begin',4),
 @('    return add(pParam1, args)->mLayoutHolder;','ResourceHolderManager add layout holder begin',2))){
 $needle=$stage[0];if(!$resourceManagerText.Contains($needle)){throw "ResourceHolderManager trace anchor missing: $needle"}
 $replacement='    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $needle
 if($stage[2] -eq 4){$replacement=$replacement + "`n" + '    galaxyNativeBootTrace("ResourceHolderManager createLayoutHolder complete");'}
 $resourceManagerText=$resourceManagerText.Replace($needle,$replacement)
}
[IO.File]::WriteAllText($resourceManagerSource,$resourceManagerText,[Text.UTF8Encoding]::new($false))
$resourceManagerCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-gcodeview','-fno-inline','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 "-I$functorInclude", "-I$heapFunctorInclude", "-I$heapPlacementInclude", '-include',$heapPlacementHeader,
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h', '-c')
& $zig c++ @resourceManagerCommon $resourceManagerSource '-o' $resourceManagerObject
if($LASTEXITCODE){throw 'Boot-traced native ResourceHolderManager provider failed to compile.'}
$layoutHolderObject=Join-Path $tmp 'LayoutHolder.obj'
$layoutHolderSource=Join-Path $tmp 'LayoutHolder-boot-traced.cpp'
$layoutHolderText=Get-Content -LiteralPath (Join-Path $native 'src\generated\LayoutHolder.cpp') -Raw
$layoutHolderText='#include <cstdio>' + "`n" + '#include <windows.h>' + "`n" + '#include <JSystem/JKernel/JKRExpHeap.hpp>' + "`n" + '#include "Game/Util/MemoryUtil.hpp"' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + 'static void galaxyNativeThreadTrace(const char* text) { char line[256]; std::snprintf(line,sizeof(line),"%s tid=%lu",text,static_cast<unsigned long>(GetCurrentThreadId())); galaxyNativeBootTrace(line); }' + "`n" + $layoutHolderText
$getResourceStart='void* LayoutHolder::GetResource(u32 resourceKind, const char* pName, u32* pResourceInfo) {'
if(!$layoutHolderText.Contains($getResourceStart)){throw 'LayoutHolder GetResource trace anchor missing.'}
$layoutHolderText=$layoutHolderText.Replace($getResourceStart,$getResourceStart + "`n" + '    { char line[240]; std::snprintf(line,sizeof(line),"LayoutHolder GetResource kind=%08X name=%s info=%p",unsigned(resourceKind),pName?pName:"<null>",static_cast<void*>(pResourceInfo)); galaxyNativeBootTrace(line); }')
$getFontStart='nw4r::ut::Font* LayoutHolder::GetFont(const char* pName) {'
if(!$layoutHolderText.Contains($getFontStart)){throw 'LayoutHolder GetFont trace anchor missing.'}
$layoutHolderText=$layoutHolderText.Replace($getFontStart,$getFontStart + "`n" + '    { char line[200]; std::snprintf(line,sizeof(line),"LayoutHolder GetFont name=%s",pName?pName:"<null>"); galaxyNativeBootTrace(line); }')
$layoutCtor='LayoutHolder::LayoutHolder(JKRArchive& rArchive) : nw4r::lyt::ResourceAccessor(), mArchive(&rArchive) {'
if(!$layoutHolderText.Contains($layoutCtor)){throw 'LayoutHolder constructor trace anchor missing.'}
$layoutHolderText=$layoutHolderText.Replace($layoutCtor,$layoutCtor + "`n" + '    galaxyNativeThreadTrace("LayoutHolder constructor begin");')
$layoutHolderText=$layoutHolderText.Replace('    initializeArc();','    initializeArc();' + "`n" + '    galaxyNativeBootTrace("LayoutHolder initializeArc complete");')
$initArc='void LayoutHolder::initializeArc() {'
if(!$layoutHolderText.Contains($initArc)){throw 'LayoutHolder initializeArc trace anchor missing.'}
$layoutHolderText=$layoutHolderText.Replace($initArc,$initArc + "`n" + '    galaxyNativeThreadTrace("LayoutHolder initializeArc begin");')
foreach($stage in @(@('    u32 resCount = mArchive->countResource();','LayoutHolder countResource complete'),@('    resCount -= initEachResTable(&mLayoutRes, ::sLayoutExt);','LayoutHolder layout table initialized'),@('    resCount -= initEachResTable(&mAnimRes, ::sAnimationExt);','LayoutHolder animation table initialized'),@('    mount(nullptr);','LayoutHolder mount complete'))){$needle=$stage[0];if(!$layoutHolderText.Contains($needle)){throw "LayoutHolder trace anchor missing: $needle"};$probe='    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"stage=%s gddr=%p start=%p end=%p size=%u","'+$stage[1]+'",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }';$replacement=$needle + "`n" + '    galaxyNativeBootTrace("'+$stage[1]+'");' + "`n" + $probe;$layoutHolderText=$layoutHolderText.Replace($needle,$replacement)}
$layoutHolderText=$layoutHolderText.Replace('    if (pFinder->mFileIsFolder) {','    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[320]; std::snprintf(line,sizeof(line),"layout row root=%s name=%s folder=%d gddr=%p start=%p end=%p size=%u",pRoot?pRoot:"<null>",pFinder->mName?pFinder->mName:"<null>",pFinder->mFileIsFolder?1:0,static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + '    if (pFinder->mFileIsFolder) {')
$layoutHolderText=$layoutHolderText.Replace('                mount(path);','                mount(path);' + "`n" + '                { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[260]; std::snprintf(line,sizeof(line),"layout child returned path=%s gddr=%p start=%p end=%p size=%u",path,static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$layoutHolderText=$layoutHolderText.Replace('            pInfo->_C = pFinder->mFileID;','            pInfo->_C = pFinder->mFileID;' + "`n" + '            { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[260]; std::snprintf(line,sizeof(line),"layout resource mounted name=%s id=%u gddr=%p start=%p end=%p size=%u",pFinder->mName,static_cast<unsigned>(pFinder->mFileID),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$layoutHolderText=$layoutHolderText.Replace('    galaxyNativeBootTrace("LayoutHolder finder delete complete");','    galaxyNativeBootTrace("LayoutHolder finder delete complete");' + "`n" + '    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"finder delete complete gddr=%p start=%p end=%p size=%u",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$mountFunctionStart=$layoutHolderText.IndexOf('void LayoutHolder::mount(char* pRoot) {')
if($mountFunctionStart -lt 0){throw 'mount function not found for finder probe'}
$mountFinderCall=$layoutHolderText.IndexOf('getFileFinder(pRoot);',$mountFunctionStart)
if($mountFinderCall -lt 0){throw 'mount getFileFinder call not found'}
$mountStatementStart=$layoutHolderText.LastIndexOf("`n",$mountFinderCall)+1;$mountStatementEnd=$layoutHolderText.IndexOf(';',$mountFinderCall)+1
$probeBefore='    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"mount before finder root=%s gddr=%p start=%p end=%p size=%u",pRoot?pRoot:"<null>",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }'
$probeAfter='    { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"mount after finder root=%s gddr=%p start=%p end=%p size=%u",pRoot?pRoot:"<null>",static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }'
$layoutHolderText=$layoutHolderText.Insert($mountStatementEnd,"`n"+$probeAfter)
$layoutHolderText=$layoutHolderText.Insert($mountStatementStart,$probeBefore+"`n")
$layoutHolderText=$layoutHolderText.Replace('    u32 resCount = mArchive->countResource();','    u32 resCount = mArchive->countResource();' + "`n" + '    { char line[120]; std::snprintf(line,sizeof(line),"LayoutHolder archive resource count=%u",static_cast<unsigned>(resCount)); galaxyNativeBootTrace(line); }')
$layoutHolderText=$layoutHolderText.Replace('    resCount -= initEachResTable(&mLayoutRes, ::sLayoutExt);','    resCount -= initEachResTable(&mLayoutRes, ::sLayoutExt);' + "`n" + '    { char line[120]; std::snprintf(line,sizeof(line),"LayoutHolder remaining after layout table=%u",static_cast<unsigned>(resCount)); galaxyNativeBootTrace(line); }')
$layoutHolderText=$layoutHolderText.Replace('    resCount -= initEachResTable(&mAnimRes, ::sAnimationExt);','    resCount -= initEachResTable(&mAnimRes, ::sAnimationExt);' + "`n" + '    { char line[120]; std::snprintf(line,sizeof(line),"LayoutHolder remaining after anim table=%u",static_cast<unsigned>(resCount)); galaxyNativeBootTrace(line); }')
$layoutHolderText=$layoutHolderText.Replace('        mResOther.newFileInfoTable(resCount);','        { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"LayoutHolder other table alloc before count=%u table=%p gddr=%p start=%p end=%p size=%u",static_cast<unsigned>(resCount),static_cast<void*>(mResOther.mFileInfoTable),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n" + '        mResOther.newFileInfoTable(resCount);' + "`n" + '        { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[240]; std::snprintf(line,sizeof(line),"LayoutHolder other table alloc after table=%p gddr=%p start=%p end=%p size=%u",static_cast<void*>(mResOther.mFileInfoTable),static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }')
$layoutCount='u32 LayoutHolder::count(const char* pExtension, const char* pRoot) {'
if(!$layoutHolderText.Contains($layoutCount)){throw 'LayoutHolder count trace anchor missing.'}
$layoutHolderText=$layoutHolderText.Replace($layoutCount,$layoutCount + "`n" + '    galaxyNativeBootTrace("LayoutHolder count begin");')
$finderLine='    JKRArcFinder* pFinder = getFileFinder(pRoot);'
if(!$layoutHolderText.Contains($finderLine)){throw 'LayoutHolder file-finder trace anchor missing.'}
$finderIndex=$layoutHolderText.IndexOf($finderLine)
if($finderIndex -lt 0){throw 'LayoutHolder file-finder source position missing.'}
$finderReplacement='    galaxyNativeBootTrace("LayoutHolder finder acquire begin");' + "`n" + $finderLine + "`n" + '    galaxyNativeBootTrace(pFinder ? "LayoutHolder finder acquired" : "LayoutHolder finder is null");' + "`n" + '    if (!pFinder) return 0;'
$layoutHolderText=$layoutHolderText.Remove($finderIndex,$finderLine.Length).Insert($finderIndex,$finderReplacement)
$layoutHolderText=$layoutHolderText.Replace('    while (pFinder->mHasMoreFiles) {','    while (pFinder->mHasMoreFiles) {' + "`n" + '        char nativeLayoutTrace[256];' + "`n" + '        std::snprintf(nativeLayoutTrace, sizeof(nativeLayoutTrace), "LayoutHolder entry root=%s name=%s folder=%d", pRoot ? pRoot : "<root>", pFinder->mName ? pFinder->mName : "<null>", pFinder->mFileIsFolder ? 1 : 0);' + "`n" + '        galaxyNativeBootTrace(nativeLayoutTrace);')
$layoutHolderText=$layoutHolderText.Replace('        pFinder->findNextFile();','        pFinder->findNextFile();' + "`n" + '        galaxyNativeBootTrace("LayoutHolder finder advanced");')
$layoutHolderText=$layoutHolderText.Replace('    delete pFinder;','    galaxyNativeBootTrace("LayoutHolder finder delete begin");' + "`n" + '    delete pFinder;' + "`n" + '    galaxyNativeBootTrace("LayoutHolder finder delete complete");')
$deleteFinderNeedle='    delete pFinder;'
if($DiagnosticSkipArcFinderFree){
 $firstFinderDelete=$layoutHolderText.IndexOf($deleteFinderNeedle)
 $secondFinderDelete=$layoutHolderText.IndexOf($deleteFinderNeedle,$firstFinderDelete+$deleteFinderNeedle.Length)
 if($firstFinderDelete -lt 0 -or $secondFinderDelete -lt 0){throw 'LayoutHolder finder deallocation trace anchors missing.'}
 $layoutHolderText=$layoutHolderText.Remove($secondFinderDelete,$deleteFinderNeedle.Length).Insert($secondFinderDelete,'    pFinder->~JKRFileFinder();')
 $layoutHolderText=$layoutHolderText.Remove($firstFinderDelete,$deleteFinderNeedle.Length).Insert($firstFinderDelete,'    pFinder->~JKRArcFinder();')
}
[IO.File]::WriteAllText($layoutHolderSource,$layoutHolderText,[Text.UTF8Encoding]::new($false))
$layoutHolderCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include',"$native\include-runtime\native_no_weak.h",'-include','revolution/types.h',
 '-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @layoutHolderCommon $layoutHolderSource '-o' $layoutHolderObject
if($LASTEXITCODE){throw 'Boot-traced native LayoutHolder provider failed to compile.'}
$archivePubObject=Join-Path $tmp 'JKRArchivePub.obj'
$archivePubCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include','revolution/types.h','-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",
 '-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @archivePubCommon (Join-Path $native 'src\generated\JKRArchivePub.cpp') '-o' $archivePubObject
if($LASTEXITCODE){throw 'Native JKRArchivePub provider failed to compile.'}
$archiveHolderObject=Join-Path $tmp 'ArchiveHolder.obj'
$archiveHolderSource=Join-Path $tmp 'ArchiveHolder-boot-traced.cpp'
$archiveHolderText=Get-Content -LiteralPath (Join-Path $native 'src\generated\ArchiveHolder.cpp') -Raw
$archiveHolderText='#include <cstdio>' + "`n" + '#include <JSystem/JKernel/JKRExpHeap.hpp>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $archiveHolderText
$archiveHolderHelper='static void traceArchiveHolderHeap(const char* stage) { JKRExpHeap* h=MR::getStationedHeapGDDR3(); char line[256]; std::snprintf(line,sizeof(line),"ArchiveHolder %s gddr=%p start=%p end=%p size=%u",stage,static_cast<void*>(h),h?static_cast<void*>(h->mStart):nullptr,h?static_cast<void*>(h->mEnd):nullptr,h?h->mSize:0); galaxyNativeBootTrace(line); }' + "`n"
$archiveHolderText=$archiveHolderText.Replace('#include "Game/Util.hpp"','#include "Game/Util.hpp"' + "`n" + $archiveHolderHelper)
$archiveHolderText=$archiveHolderText.Replace('    JKRMemArchive* archive = new (pHeap, 0) JKRMemArchive();','    traceArchiveHolderHeap("entry ctor before archive alloc");' + "`n" + '    JKRMemArchive* archive = new (pHeap, 0) JKRMemArchive();' + "`n" + '    traceArchiveHolderHeap("entry ctor after archive alloc");')
$archiveHolderText=$archiveHolderText.Replace('    if (!archive->mountFixed(pData, JKR_MEM_BREAK_FLAG_0)) {','    traceArchiveHolderHeap("before mountFixed");' + "`n" + '    if (!archive->mountFixed(pData, JKR_MEM_BREAK_FLAG_0)) {' + "`n" + '        traceArchiveHolderHeap("mountFixed failed");')
$archiveHolderText=$archiveHolderText.Replace('    mArchive = archive;','    mArchive = archive;' + "`n" + '    traceArchiveHolderHeap("after mountFixed");')
$archiveHolderText=$archiveHolderText.Replace('    mArchiveName = new (pHeap, 0) char[len];','    traceArchiveHolderHeap("before name alloc");' + "`n" + '    mArchiveName = new (pHeap, 0) char[len];' + "`n" + '    traceArchiveHolderHeap("after name alloc");')
$archiveHolderText=$archiveHolderText.Replace('    ArchiveHolderArchiveEntry* entry = new (pHeap, 0) ArchiveHolderArchiveEntry(pData, pHeap, pArchiveName);','    traceArchiveHolderHeap("createAndAdd before entry alloc");' + "`n" + '    ArchiveHolderArchiveEntry* entry = new (pHeap, 0) ArchiveHolderArchiveEntry(pData, pHeap, pArchiveName);' + "`n" + '    traceArchiveHolderHeap("createAndAdd after entry alloc");')
$archiveHolderText=$archiveHolderText.Replace('    mEntries.push_back(entry);','    traceArchiveHolderHeap("before entries push");' + "`n" + '    mEntries.push_back(entry);' + "`n" + '    traceArchiveHolderHeap("after entries push");')
[IO.File]::WriteAllText($archiveHolderSource,$archiveHolderText,[Text.UTF8Encoding]::new($false))
& $zig c++ @archivePubCommon $archiveHolderSource '-o' $archiveHolderObject
if($LASTEXITCODE){throw 'Boot-traced native ArchiveHolder provider failed to compile.'}
$fileLoaderReleaseObject=Join-Path $tmp 'JKRFileLoader-heap-release.obj'
& $zig c++ @archivePubCommon (Join-Path $native 'src\generated\JKRFileLoader.cpp') '-o' $fileLoaderReleaseObject
if($LASTEXITCODE){throw 'Heap-owned JKRFileLoader provider failed to compile.'}
$gameSceneObject=Join-Path $tmp 'GameScene-init-traced.obj'
$gameSceneSource=Join-Path $tmp 'GameScene-init-traced.cpp'
$gameSceneText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\Scene\GameScene.cpp') -Raw
$gameSceneBegin=$gameSceneText.IndexOf('void GameScene::init() {')
$gameSceneEnd=$gameSceneText.IndexOf('void GameScene::start() {',$gameSceneBegin)
if($gameSceneBegin-lt 0-or $gameSceneEnd-le $gameSceneBegin){throw 'Game scene init phase anchors missing.'}
$phase=0;$phaseMap=@();$phaseLines=@()
foreach($line in ($gameSceneText.Substring($gameSceneBegin,$gameSceneEnd-$gameSceneBegin)-split '\r?\n')){
 if($line.Trim().EndsWith(';')-and -not $line.Trim().StartsWith('//')){
  ++$phase;$phaseMap+=('phase {0}: {1}'-f $phase,$line.Trim())
  $phaseLines+=('    galaxyNativeBootTrace("GameScene init phase {0} begin");'-f $phase)
  $phaseLines+=$line
  $phaseLines+=('    galaxyNativeBootTrace("GameScene init phase {0} returned");'-f $phase)
  $phaseLines+=('    {{ char line[160]; std::snprintf(line,sizeof(line),"GameScene writer phase={0} current=%p",static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }}'-f $phase)
 }else{$phaseLines+=$line}
}
$phaseMap|Set-Content -LiteralPath (Join-Path $tmp 'game-scene-init-phases.txt')
$gameSceneText=$gameSceneText.Substring(0,$gameSceneBegin)+($phaseLines-join [Environment]::NewLine)+[Environment]::NewLine+$gameSceneText.Substring($gameSceneEnd)
$gameSceneRuntimeTrace=@"
    static const Nerve* previousGameNerve=nullptr;
    const auto* currentGameNerve=mSpine?mSpine->getCurrentNerve():nullptr;
    if(mSpine&&(currentGameNerve!=previousGameNerve||mSpine->mStep%60==0)){
        previousGameNerve=currentGameNerve;
        const char* state=isExecScenarioOpeningCamera()?"opening-camera":isExecScenarioStarter()?"scenario-starter":isNerve(GET_NERVE(GameScene,GameSceneAction))?"action":isPlayMovie()?"movie":"other";
        auto* cameraSpine=mScenarioCamera?mScenarioCamera->mSpine:nullptr;
        char line[320];std::snprintf(line,sizeof(line),"GameScene runtime state=%s step=%d nerve=%p cameraNerve=%p cameraStep=%d cameraDone=%u",state,int(mSpine->mStep),static_cast<const void*>(currentGameNerve),cameraSpine?static_cast<const void*>(cameraSpine->getCurrentNerve()):nullptr,cameraSpine?int(cameraSpine->mStep):-1,unsigned(mScenarioCamera&&mScenarioCamera->isDone()));galaxyNativeBootTrace(line);
        auto* holder=MR::getMarioHolder();auto* player=holder?holder->getMarioActor():nullptr;
        if(player){char playerLine[640];std::snprintf(playerLine,sizeof(playerLine),"Player runtime stage=%s scenario=%d actor=%p pos=(%.5f,%.5f,%.5f) rotation=(%.5f,%.5f,%.5f) scale=(%.5f,%.5f,%.5f) velocity=(%.5f,%.5f,%.5f) dead=%u hidden=%u clipped=%u noCalcView=%u model=%p spine=%p",MR::getCurrentStageName(),MR::getCurrentScenarioNo(),static_cast<void*>(player),double(player->mPosition.x),double(player->mPosition.y),double(player->mPosition.z),double(player->mRotation.x),double(player->mRotation.y),double(player->mRotation.z),double(player->mScale.x),double(player->mScale.y),double(player->mScale.z),double(player->mVelocity.x),double(player->mVelocity.y),double(player->mVelocity.z),unsigned(player->mFlag.mIsDead),unsigned(player->mFlag.mIsHiddenModel),unsigned(player->mFlag.mIsClipped),unsigned(player->mFlag.mIsNoCalcView),static_cast<void*>(player->mModelManager),static_cast<void*>(player->mSpine));galaxyNativeBootTrace(playerLine);char executionLine[192];std::snprintf(executionLine,sizeof(executionLine),"Player execution movementCount=%u nerveStep=%d flags=%u executor=%d",unsigned(player->_378),player->mSpine?int(player->mSpine->mStep):-999,unsigned(static_cast<NameObj*>(player)->getFlag()),int(player->mExecutorIdx));galaxyNativeBootTrace(executionLine);}else galaxyNativeBootTrace("Player runtime actor=null");
        const auto cameraPosition=MR::getCamPos();char cameraLine[384];std::snprintf(cameraLine,sizeof(cameraLine),"Camera runtime position=(%.5f,%.5f,%.5f) near=%.5f far=%.5f fovy=%.5f aspect=%.5f",double(cameraPosition.x),double(cameraPosition.y),double(cameraPosition.z),double(MR::getNearZ()),double(MR::getFarZ()),double(MR::getFovy()),double(MR::getAspect()));galaxyNativeBootTrace(cameraLine);
        const auto& view=MR::getCameraViewMtx();const auto& projection=MR::getCameraProjectionMtx();
        for(unsigned row=0;row<3;++row){char matrixLine[192];std::snprintf(matrixLine,sizeof(matrixLine),"Camera view row=%u %.8g %.8g %.8g %.8g",row,double(view.mMtx[row][0]),double(view.mMtx[row][1]),double(view.mMtx[row][2]),double(view.mMtx[row][3]));galaxyNativeBootTrace(matrixLine);}
        for(unsigned row=0;row<4;++row){char matrixLine[192];std::snprintf(matrixLine,sizeof(matrixLine),"Camera projection row=%u %.8g %.8g %.8g %.8g",row,double(projection.mMtx[row][0]),double(projection.mMtx[row][1]),double(projection.mMtx[row][2]),double(projection.mMtx[row][3]));galaxyNativeBootTrace(matrixLine);}
        auto* group=MR::getAllLiveActorGroup();unsigned alive=0,modeled=0,visible=0;
        if(group){for(int i=0;i<group->getObjNum();++i){const auto* actor=group->getActor(i);if(actor){alive+=!actor->mFlag.mIsDead;modeled+=actor->mModelManager!=nullptr;visible+=!actor->mFlag.mIsDead&&!actor->mFlag.mIsHiddenModel&&!actor->mFlag.mIsClipped&&actor->mModelManager!=nullptr;if(i<16||!actor->mFlag.mIsDead){char actorLine[384];std::snprintf(actorLine,sizeof(actorLine),"Stage actor index=%d name=%s pos=(%.5f,%.5f,%.5f) dead=%u hidden=%u clipped=%u model=%p",i,actor->mName?actor->mName:"(null)",double(actor->mPosition.x),double(actor->mPosition.y),double(actor->mPosition.z),unsigned(actor->mFlag.mIsDead),unsigned(actor->mFlag.mIsHiddenModel),unsigned(actor->mFlag.mIsClipped),static_cast<void*>(actor->mModelManager));galaxyNativeBootTrace(actorLine);}}}char groupLine[192];std::snprintf(groupLine,sizeof(groupLine),"Stage actors total=%d alive=%u modeled=%u visible=%u",group->getObjNum(),alive,modeled,visible);galaxyNativeBootTrace(groupLine);}
    }
"@
if(!$gameSceneText.Contains('void GameScene::update() {')){throw 'Game scene update trace anchor missing'}
$gameSceneText=$gameSceneText.Replace('void GameScene::update() {','void GameScene::update() {'+[Environment]::NewLine+$gameSceneRuntimeTrace)
$gameSceneText='#include "Game/Player/MarioHolder.hpp"'+"`n"+'#include "Game/Player/MarioActor.hpp"'+"`n"+'#include "Game/LiveActor/AllLiveActorGroup.hpp"'+"`n"+$gameSceneText
$gameSceneText='#include <cstdio>'+[Environment]::NewLine+'#include <revolution/gd/GDBase.h>'+[Environment]::NewLine+'extern "C" void galaxyNativeBootTrace(const char*);'+[Environment]::NewLine+$gameSceneText
[IO.File]::WriteAllText($gameSceneSource,$gameSceneText,[Text.UTF8Encoding]::new($false))
$gameSceneCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-c','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-target','x86_64-windows-gnu')
$gameSceneCommon+='-DGALAXY_NATIVE_JPA_USERWORK_64=1'
foreach($relative in @('native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include','Petari\libs\RVLFaceLib\include','Petari\libs\nw4r\include')){$gameSceneCommon+=('-I'+(Join-Path $root $relative))}
$gameSceneCommon+=@('-include','revolution/types.h','-include',(Join-Path $native 'include-runtime\nw4r\ut\TagProcessorBase.h'),'-include',(Join-Path $native 'include-runtime\nw4r\ut\Color.h'))
& $zig c++ @gameSceneCommon $gameSceneSource '-o' $gameSceneObject
if($LASTEXITCODE){throw 'Phase-traced original GameScene provider failed to compile.'}
$prologueRuntimeProviders=@()
foreach($prologueUnit in @('PrologueDirector','ProloguePictureBook')){
 $prologueSource=if($prologueUnit-eq 'PrologueDirector'){Join-Path $native 'src\generated\PrologueDirector.cpp'}else{Join-Path $root 'Petari\src\Game\Screen\ProloguePictureBook.cpp'}
 $prologueText=[IO.File]::ReadAllText($prologueSource)
 $functions=if($prologueUnit-eq 'PrologueDirector'){@('appear','exeWait','exePictureBook','exePeachLetterStart','exePeachLetter','exePeachLetterWait','exePeachLetterEnd','exeBindWait','exeArrive','exeGameStart')}else{@('appear','exePlaying','exeKeyWait','exeEnd')}
 foreach($function in $functions){
  $signature='void '+$prologueUnit+'::'+$function+'() {'
  if(!$prologueText.Contains($signature)){throw "Prologue trace anchor missing: $signature"}
  $trace=if($prologueUnit-eq 'PrologueDirector'){'    if(mSpine&&mSpine->mStep%60==0){char line[240];std::snprintf(line,sizeof(line),"PrologueDirector state='+$function+' step=%d bookDead=%u bookHidden=%u bookStep=%d",int(mSpine->mStep),unsigned(!mPictureBook||mPictureBook->mFlag.mIsDead),unsigned(mPictureBook&&mPictureBook->mFlag.mIsHidden),mPictureBook?mPictureBook->getNerveStep():-1);galaxyNativeBootTrace(line);}'}else{'    if(mSpine&&mSpine->mStep%60==0){auto* frame=mLayoutManager?MR::getAnimCtrl(this,0):nullptr;char line[240];std::snprintf(line,sizeof(line),"PrologueBook state='+$function+' step=%d page=%u dead=%u hidden=%u frame=%.2f rate=%.2f",getNerveStep(),unsigned(mPage),unsigned(mFlag.mIsDead),unsigned(mFlag.mIsHidden),frame?double(frame->getFrame()):-1.0,frame?double(frame->getRate()):-1.0);galaxyNativeBootTrace(line);}'}
  $prologueText=$prologueText.Replace($signature,$signature+"`n"+$trace)
 }
 $prologueText='#include <cstdio>'+"`n"+'#include "Game/LiveActor/Spine.hpp"'+"`n"+'#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$prologueText
 $prologuePath=Join-Path $tmp ($prologueUnit+'-runtime-traced.cpp')
 $prologueObject=Join-Path $tmp ($prologueUnit+'-runtime-traced.obj')
 [IO.File]::WriteAllText($prologuePath,$prologueText,[Text.UTF8Encoding]::new($false))
 & $zig c++ @gameSceneCommon '-include' (Join-Path $native 'include-runtime\native_no_weak.h') $prologuePath '-o' $prologueObject
 if($LASTEXITCODE){throw "Prologue runtime trace compilation failed: $prologueUnit"}
 $prologueRuntimeProviders+=@{Unit=$prologueUnit;Path=$prologueObject}
}
$titleRuntimeText=[IO.File]::ReadAllText((Join-Path $native 'src\generated\TitleSequenceProduct.cpp'))
foreach($function in @('exeDisplayEncouragePal60Window','exeBgmPrepare','exeLogoFadein','exeLogoWait','exeLogoDisplay','exeDecide','exeDead')){
 $signature='void TitleSequenceProduct::'+$function+'() {'
 if(!$titleRuntimeText.Contains($signature)){throw "Title sequence trace anchor missing: $function"}
 $trace='    if(mSpine&&mSpine->mStep%60==0){char line[240];std::snprintf(line,sizeof(line),"Title runtime state='+$function+' step=%d bgmPrepared=%u logoDead=%u pressDead=%u",int(mSpine->mStep),unsigned(MR::isPreparedStageBgm()),unsigned(!mLogoLayout||mLogoLayout->mFlag.mIsDead),unsigned(!mPressStartLayout||mPressStartLayout->mFlag.mIsDead));galaxyNativeBootTrace(line);auto* converter=AudWrap::getSoundNameConverter();JAISoundID requested=converter?converter->getSoundID("STM_TITLE"):JAISoundID(u32(-1));auto* bgm=AudWrap::getStageBgm();auto* handle=bgm?bgm->getHandle():nullptr;auto* sound=handle?handle->getSound():nullptr;auto* stream=sound?sound->asStream():nullptr;char audioLine[512];std::snprintf(audioLine,sizeof(audioLine),"Title audio nameID=%08X expectedID=%08X section=%u group=%u wave=%u multiBit=%u bgm=%p handle=%p sound=%p soundID=%08X state=%d stream=%p prepare=%d entry=%d",unsigned(u32(requested)),unsigned(STM_TITLE),unsigned(requested.getSectionID()),unsigned(requested.getGroupID()),unsigned(requested.getWaveID()),unsigned((u32(requested)&0x10000)!=0),static_cast<void*>(bgm),static_cast<void*>(handle),static_cast<void*>(sound),sound?unsigned(u32(sound->mSoundID)):0,sound?int(sound->mStatus.getState()):-1,static_cast<void*>(stream),stream?stream->mPrepareState:-1,stream?stream->mStreamFileEntry:-1);galaxyNativeBootTrace(audioLine);auto* frame=mLogoLayout&&mLogoLayout->mLayoutManager?MR::getAnimCtrl(mLogoLayout,0):nullptr;if(frame){char frameLine[240];std::snprintf(frameLine,sizeof(frameLine),"Title animation frame=%.3f end=%d rate=%.3f attribute=%u state=%u",double(frame->getFrame()),int(frame->getEnd()),double(frame->getRate()),unsigned(frame->getAttribute()),unsigned(frame->getState()));galaxyNativeBootTrace(frameLine);}}'
 $titleRuntimeText=$titleRuntimeText.Replace($signature,$signature+"`n"+$trace)
}
$titleRuntimeText='#include "Game/AudioLib/AudWrap.hpp"'+"`n"+'#include "Game/AudioLib/AudBgm.hpp"'+"`n"+'#include "Game/AudioLib/AudSoundNameConverter.hpp"'+"`n"+'#include "Game/AudioLib/AudSoundId.hpp"'+"`n"+'#include <JSystem/JAudio2/JAIStream.hpp>'+"`n"+$titleRuntimeText
$titleRuntimeText='#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`n"+$titleRuntimeText
$titleRuntimeText='#include <Native/DiagnosticMenuInput.hpp>'+"`n"+$titleRuntimeText
foreach($phase in @(@('exeLogoDisplay','Title'),@('exeDecide','Quiet'))){
 $anchor='void TitleSequenceProduct::'+$phase[0]+'() {'
 if(!$titleRuntimeText.Contains($anchor)){throw 'Title input observation anchor missing'}
 $titleRuntimeText=$titleRuntimeText.Replace($anchor,$anchor+"`n"+'    NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::'+$phase[1]+');')
}
$titleRuntimeText='#include <cstdio>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$titleRuntimeText
$titleRuntimeSource=Join-Path $tmp 'TitleSequenceProduct-runtime-traced.cpp'
$titleRuntimeObject=Join-Path $tmp 'TitleSequenceProduct-runtime-traced.obj'
[IO.File]::WriteAllText($titleRuntimeSource,$titleRuntimeText,[Text.UTF8Encoding]::new($false))
& $zig c++ @gameSceneCommon '-include' (Join-Path $native 'include-runtime\native_no_weak.h') $titleRuntimeSource '-o' $titleRuntimeObject
if($LASTEXITCODE){throw 'Title sequence runtime trace compile failed'}
$prologueRuntimeProviders+=@{Unit='TitleSequenceProduct';Path=$titleRuntimeObject}
$messageSensorObject=Join-Path $tmp 'MessageSensorHolder-startup.obj'
& $zig c++ @gameSceneCommon (Join-Path $root 'Petari\src\Game\LiveActor\MessageSensorHolder.cpp') '-o' $messageSensorObject
if($LASTEXITCODE){throw 'Original startup MessageSensorHolder provider failed to compile.'}
$objects+=@{Path=$messageSensorObject}
$startupProviders=@(
 @{Source=(Join-Path $root 'Petari\src\Game\GameAudio\AudCameraWatcher.cpp');Name='AudCameraWatcher-startup.obj'},
 @{Source=(Join-Path $root 'Petari\src\Game\Util\ShareUtil.cpp');Name='ShareUtil-startup.obj'},
 @{Source=(Join-Path $root 'Petari\src\Game\Screen\CometRetryButton.cpp');Name='CometRetryButton-startup.obj'},
 @{Source=(Join-Path $native 'src\generated\ProductionEventSequencerInit.cpp');Name='ProductionEventSequencerInit-startup.obj'}
)
foreach($eventUnit in @('PlayerEventDown','PlayerEventAbyss','PlayerEventFireDown','PlayerEventRaceDown','PlayerEventGhostRaceDown','PlayerEventGameOver')){
 $startupProviders+=@{Source=(Join-Path $root ('Petari\src\Game\Player\'+$eventUnit+'.cpp'));Name=($eventUnit+'-startup.obj')}
}
foreach($provider in $startupProviders){
 $provider.Path=Join-Path $tmp $provider.Name
 & $zig c++ @gameSceneCommon $provider.Source '-o' $provider.Path
 if($LASTEXITCODE){throw "Original startup provider failed to compile: $($provider.Source)"}
}
$objects+=$startupProviders
$displayMakerObject=Join-Path $tmp 'DisplayListMaker-texture-address-traced.obj'
$displayMakerSource=Join-Path $tmp 'DisplayListMaker-texture-address-traced.cpp'
$displayMakerText=Get-Content -LiteralPath (Join-Path $native 'src\generated\DisplayListMaker.cpp') -Raw
$displayMakerText="#include <cstdio>`n#include <Native/TexturePointers.hpp>`nextern `"C`" void galaxyNativeBootTrace(const char*);`n"+$displayMakerText
foreach($entry in @(
 @('mModel->getModelData()->getTexture()->setResTIMG(i, *MR::getScreenResTIMG());','MR::getScreenResTIMG()'),
 @('mModel->getModelData()->getTexture()->setResTIMG(i, *texture->getTexInfo());','texture->getTexInfo()')
)){
 $anchor=$entry[0];if(!$displayMakerText.Contains($anchor)){throw 'Display-maker texture replacement trace anchor missing'}
 $trace=' { auto* destination=mModel->getModelData()->getTexture()->getResTIMG(i); auto* source='+$entry[1]+'; char line[300]; std::snprintf(line,sizeof(line),"Texture replacement name=%s model=%p destination=%p source=%p expected-image=%p computed-image=%p stored-offset=%08X",name,mModel,destination,source,Native::textureImage(source),Native::textureImage(destination),unsigned(destination->mImageDataOffset)); galaxyNativeBootTrace(line); }'
 $displayMakerText=$displayMakerText.Replace($anchor,$anchor+$trace)
}
[IO.File]::WriteAllText($displayMakerSource,$displayMakerText,[Text.UTF8Encoding]::new($false))
& $zig c++ @gameSceneCommon $displayMakerSource '-o' $displayMakerObject
if($LASTEXITCODE){throw 'Texture-address-traced DisplayListMaker failed to compile.'}
$texturePointerProviders=@()
$texturePointerRoot=Join-Path $native 'build\actor-runtime-texture-pointers-20261014'
New-Item -ItemType Directory -Force -Path $texturePointerRoot|Out-Null
foreach($unit in @('J3DTevs','J3DDrawBuffer','JUTTexture','J3DModelLoader')){
 $provider=@{Unit=$unit;Path=(Join-Path $texturePointerRoot ($unit+'.obj'))}
 & $zig c++ @gameSceneCommon (Join-Path $native ('src\generated\'+$unit+'.cpp')) '-o' $provider.Path
 if($LASTEXITCODE){throw "Host texture-pointer provider failed: $unit"}
 $texturePointerProviders+=$provider
}
$nameObjAdaptorObject=Join-Path $tmp 'NameObjAdaptor-current-callback-abi.obj'
& $zig c++ @gameSceneCommon (Join-Path $native 'src\generated\NameObjAdaptor.cpp') '-o' $nameObjAdaptorObject
if($LASTEXITCODE){throw 'Current callback-ABI NameObjAdaptor provider failed to compile.'}
$nameObjCategoryListObject=Join-Path $tmp 'NameObjCategoryList-current-callback-abi.obj'
& $zig c++ @gameSceneCommon (Join-Path $root 'Petari\src\Game\NameObj\NameObjCategoryList.cpp') '-o' $nameObjCategoryListObject
if($LASTEXITCODE){throw 'Current callback-ABI NameObjCategoryList provider failed to compile.'}
$stageDataHolderObject=Join-Path $tmp 'StageDataHolder-current-native-tables.obj'
$stageWriterText=Get-Content -LiteralPath (Join-Path $native 'src\generated\StageDataHolder.cpp') -Raw
$stageWriterText='#include <cstdio>'+"`n"+'#include <revolution/gd/GDBase.h>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$stageWriterText
foreach($call in @('initPlacementMario();','_FC->initPlacement();','_104->initPlacement();','_100->initPlacement();','_108->initPlacement();','_10C->initPlacement();','NameObj* obj = funcPtr("マリオアクター");','obj->init(iter);')){
 if(!$stageWriterText.Contains($call)){throw "Placement writer trace anchor missing: $call"}
 $tag=$call.Replace('"','').Replace(';','')
 $stageWriterText=$stageWriterText.Replace($call,'{ char line[220]; std::snprintf(line,sizeof(line),"Placement writer '+$tag+' before=%p",static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }'+"`n"+$call+"`n"+'{ char line[220]; std::snprintf(line,sizeof(line),"Placement writer '+$tag+' after=%p",static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }')
}
$stageWriterSource=Join-Path $tmp 'StageDataHolder-writer-traced.cpp'
[IO.File]::WriteAllText($stageWriterSource,$stageWriterText)
& $zig c++ @gameSceneCommon $stageWriterSource '-o' $stageDataHolderObject
if($LASTEXITCODE){throw 'Native-normalized StageDataHolder provider failed to compile.'}
& (Join-Path $native 'generate-dotcam-reader.ps1')
$cameraReaderProviders=@()
foreach($unit in @('DotCamParams','CameraParamChunkHolder')){
 $provider=@{Unit=$unit;Path=(Join-Path $tmp ($unit+'-native-camera-table.obj'))}
 & $zig c++ @gameSceneCommon (Join-Path $native ('src\generated\'+$unit+'.cpp')) '-o' $provider.Path
 if($LASTEXITCODE){throw "Native camera table provider failed: $unit"}
 $cameraReaderProviders+=$provider
}
$npcFunctionObject=Join-Path $tmp 'NPCFunction-native-item-table.obj'
& $zig c++ @gameSceneCommon (Join-Path $native 'src\generated\NPCFunction.cpp') '-o' $npcFunctionObject
if($LASTEXITCODE){throw 'Native NPC item table provider failed to compile.'}
$miiFaceIconSource=Join-Path $tmp 'MiiFaceIcon-native-image.cpp'
$miiFaceIconObject=Join-Path $tmp 'MiiFaceIcon-native-image.obj'
$miiFaceIconText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\NPC\MiiFaceIcon.cpp') -Raw
$miiImageAnchor='    pImageBuffer->mImageDataOffset = sizeof(ResTIMG);'
if(-not $miiFaceIconText.Contains($miiImageAnchor)){throw 'Mii image header initialization anchor missing.'}
$miiFaceIconText='#include <Native/HeapGpuRange.hpp>'+"`n"+$miiFaceIconText.Replace($miiImageAnchor,$miiImageAnchor+"`n"+'    Native::bindHeapGpuRange(mImageBuffer, imageBufferSize);')
[IO.File]::WriteAllText($miiFaceIconSource,$miiFaceIconText)
& $zig c++ @gameSceneCommon $miiFaceIconSource '-o' $miiFaceIconObject
if($LASTEXITCODE){throw 'Native Mii image provider failed to compile.'}
$fileSelectorFontSource=Join-Path $tmp 'FileSelector-native-font.cpp'
$fileSelectorFontObject=Join-Path $tmp 'FileSelector-native-font.obj'
$fileSelectorFontText=Get-Content -LiteralPath (Join-Path $native 'src\generated\MapRideCompat\FileSelector.cpp') -Raw
foreach($replacement in @(
 @('    JKRMemArchive* pArchive = MR::receiveArchive("/LayoutData/MiiFont.arc");','    mFont = nativeLoadMiiNameFont();'),
 @('    mFont = new nw4r::ut::ResFont();',''),
 @('    mFont->SetResource(pArchive->getResource("/MiiFont26.brfnt"));','')
)){
 if(-not $fileSelectorFontText.Contains($replacement[0])){throw 'FileSelector native font bridge anchor missing.'}
 $fileSelectorFontText=$fileSelectorFontText.Replace($replacement[0],$replacement[1])
}
$fileSelectorInitStart=$fileSelectorFontText.IndexOf('void FileSelector::init(')
$fileSelectorInitEnd=$fileSelectorFontText.IndexOf('void FileSelector::', $fileSelectorInitStart+5)
if($fileSelectorInitStart-lt 0-or $fileSelectorInitEnd-le $fileSelectorInitStart){throw 'FileSelector init trace anchors missing.'}
$fileSelectorInit=$fileSelectorFontText.Substring($fileSelectorInitStart,$fileSelectorInitEnd-$fileSelectorInitStart)
foreach($call in @('createCameraController','createSky','createFileItems','createOperationButton','createBackButton','createBrosButton','createInfoMessage','createSysInfoWindow','createFileInfo','createTitle','createMiiSelect','createMiiConfirmIcon','createManual','createSelectEffect','createMiiFont')){
 $needle='    '+$call+'();'
 if(!$fileSelectorInit.Contains($needle)){throw "FileSelector writer trace call missing: $call"}
 $fileSelectorInit=$fileSelectorInit.Replace($needle,'    { char line[200]; std::snprintf(line,sizeof(line),"FileSelector writer '+$call+' begin=%p",static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }'+"`n"+$needle+"`n"+'    { char line[200]; std::snprintf(line,sizeof(line),"FileSelector writer '+$call+' returned=%p",static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }')
}
$fileSelectorFontText=$fileSelectorFontText.Substring(0,$fileSelectorInitStart)+$fileSelectorInit+$fileSelectorFontText.Substring($fileSelectorInitEnd)
$fileSelectorFontText='#include <cstdio>'+"`n"+'#include <revolution/gd/GDBase.h>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$fileSelectorFontText
$fileSelectorFontText='#include <Native/GameFonts.hpp>'+"`n"+$fileSelectorFontText
$fileSelectorFontText='#include "Game/LiveActor/HitSensor.hpp"'+"`n"+$fileSelectorFontText
$fileSelectorFontText='#include "'+((Join-Path $native 'src\native_framebuffer_probe.hpp').Replace('\','/'))+'"'+"`n"+$fileSelectorFontText
foreach($function in @('exeWaitBind','exeTitle','exeTitleEnd','exeRFLWait','exeRFLError','exeFileSelectStart','exeFileSelect','exeCreateConfirmStart','exeCreateConfirm','exeCreate','exeMiiSelectStart','exeMiiSelect','exeFileConfirmStart','exeFileConfirm','exeDemoStartWait')){
 $signature='void FileSelector::'+$function+'() {'
 if(!$fileSelectorFontText.Contains($signature)){throw "FileSelector state trace anchor missing: $function"}
 $trace='    if(mSpine&&mSpine->mStep%60==0){auto* body=getSensor("body");char line[320];std::snprintf(line,sizeof(line),"FileSelector runtime state='+$function+' step=%d pos=(%.2f,%.2f,%.2f) bodyType=%u bodyValid=%u bodyContacts=%u bodyRadius=%.2f flags=%u",int(mSpine->mStep),double(mPosition.x),double(mPosition.y),double(mPosition.z),body?body->mType:0,unsigned(body&&body->isValid()),body?unsigned(body->mSensorCount):0,body?double(body->mRadius):0.0,unsigned(static_cast<NameObj*>(this)->getFlag()));galaxyNativeBootTrace(line);}'
 $fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+$trace)
}
$menuSignature='void FileSelector::exeFileSelect() {'
if(!$fileSelectorFontText.Contains($menuSignature)){throw 'Menu-ready frame observation anchor missing'}
$fileSelectorFontText='#include <Native/DiagnosticMenuInput.hpp>'+"`n"+'#include "Game/Screen/StarPointerTarget.hpp"'+"`n"+$fileSelectorFontText
$menuObserver=@'
    if(!std::getenv("GALAXY_FRAMEBUFFER_PROBE_CONFIRM_READY")&&!std::getenv("GALAXY_FRAMEBUFFER_PROBE_PROFILE_READY")&&!std::getenv("GALAXY_FRAMEBUFFER_PROBE_FILE_CONFIRM_READY"))NativeFramebufferProbe::armSceneFrame(true);
    if(std::getenv("GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT")||std::getenv("GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT")){
        FileSelectItem* target=nullptr;
        const bool saved=std::getenv("GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT")!=nullptr;
        for(int i=0;i<mItems->getObjNum();++i){auto* item=static_cast<FileSelectItem*>(mItems->getActor(i));if(saved?item&&item->_140==1&&item->isExist()&&!item->isNew():item&&item->isNew()){target=item;break;}}
        if(saved&&(!target||!mUserFile||!mUserFile[0].isCreated()))throw std::runtime_error("Saved-slot diagnostic requires original created profile 1 in ExistWait");
        TVec2f pos;const bool valid=target&&target->mStarPointerTarget&&target->mStarPointerTarget->calcScreenPosition(&pos);
        const float x=valid?2*pos.x/MR::getScreenWidth()-1:0,y=valid?2*pos.y/MR::getScreenHeight()-1:0;
        NativeDiagnosticInput::observeMenu(valid,x,y);
        if(mSpine&&mSpine->mStep%60==0){char line[240];std::snprintf(line,sizeof(line),"Diagnostic %s slot id=%d valid=%u pointer=(%.4f,%.4f) pointed=%u invalid=%u",saved?"saved":"empty",target?target->_140:-1,unsigned(valid),double(x),double(y),unsigned(target&&target->_144),unsigned(target&&target->mIsInvalidateSelect));galaxyNativeBootTrace(line);}
    }
'@
$menuObserver=(Get-Content -LiteralPath (Join-Path $native 'src\diagnostic_profile_reload.inc') -Raw)+"`n"+$menuObserver
$fileSelectorFontText=$fileSelectorFontText.Replace($menuSignature,$menuSignature+"`n"+$menuObserver)
foreach($function in @('exeCreateConfirmStart','exeFileConfirmStart','exeFileConfirm')){
 $signature='void FileSelector::'+$function+'() {'
 if(!$fileSelectorFontText.Contains($signature)){throw 'Confirm observation anchor missing'}
 $fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+'    NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);')
}
$signature='void FileSelector::exeFileConfirmStart() {'
$fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+'    if(std::getenv("GALAXY_FRAMEBUFFER_PROBE_FILE_CONFIRM_READY"))NativeFramebufferProbe::armSceneFrame(true);')
$startObserver=@'
    if(std::getenv("GALAXY_DIAGNOSTIC_SELECT_START_BUTTON")&&mOperationButton&&!MR::isDead(mOperationButton)){
        auto* pane=MR::getPane(mOperationButton,"BoxStartButton");TVec2f pos;
        if(pane){auto base=pane->GetVtxPos();MR::convertPaneLocalPosToScreenPos(&pos,pane,TVec2f(base.x+pane->mSize.width*0.5f,base.y+pane->mSize.height*0.5f));}
        const bool ready=pane&&mOperationButton->isStartButtonPointing();
        NativeDiagnosticInput::observeStartButton(pane,pane?2*pos.x/MR::getScreenWidth()-1:0,pane?2*pos.y/MR::getScreenHeight()-1:0,ready);
        if(mSpine&&mSpine->mStep%30==0){char line[240];std::snprintf(line,sizeof(line),"Diagnostic Start button valid=%u pointed=%u pointer=(%.4f,%.4f)",unsigned(pane!=nullptr),unsigned(ready),pane?double(2*pos.x/MR::getScreenWidth()-1):0,pane?double(2*pos.y/MR::getScreenHeight()-1):0);galaxyNativeBootTrace(line);}
    }else NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);
'@
$startAnchor='void FileSelector::exeFileConfirm() {'+"`n"+'    NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);'
if(!$fileSelectorFontText.Contains($startAnchor)){throw 'Start-button diagnostic anchor missing'}
$fileSelectorFontText=$fileSelectorFontText.Replace($startAnchor,'void FileSelector::exeFileConfirm() {'+"`n"+$startObserver.TrimEnd())
$fileSelectorFontText='#include "Game/Screen/YesNoController.hpp"'+"`n"+'#include "Game/Screen/ButtonPaneController.hpp"'+"`n"+'#include <nw4r/lyt/pane.h>'+"`n"+$fileSelectorFontText
$confirmationObserver=@'
    if(std::getenv("GALAXY_FRAMEBUFFER_PROBE_CONFIRM_READY")&&!std::getenv("GALAXY_FRAMEBUFFER_PROBE_PROFILE_READY"))NativeFramebufferProbe::armSceneFrame(true);
    if(std::getenv("GALAXY_DIAGNOSTIC_CREATE_PROFILE")&&mSysInfoWindow&&!MR::isDead(mSysInfoWindow)){
        auto* pane=MR::getPane(mSysInfoWindow,"BoxRight");TVec2f pos;
        if(pane){auto base=pane->GetVtxPos();MR::convertPaneLocalPosToScreenPos(&pos,pane,TVec2f(base.x+pane->mSize.width*0.5f,base.y+pane->mSize.height*0.5f));}
        const bool ready=mSysInfoWindow->isWait()&&mSysInfoWindow->mYesNoSelector&&mSysInfoWindow->mYesNoSelector->mButtonYesPaneCtrl->isPointing();
        NativeDiagnosticInput::observeConfirmation(pane,pane?2*pos.x/MR::getScreenWidth()-1:0,pane?2*pos.y/MR::getScreenHeight()-1:0,ready);
        if(mSpine&&mSpine->mStep%30==0){char line[256];std::snprintf(line,sizeof(line),"Diagnostic create Yes windowWait=%u pointed=%u selected=%u pointer=(%.4f,%.4f)",unsigned(mSysInfoWindow->isWait()),unsigned(ready),unsigned(mSysInfoWindow->isSelectedYes()),pane?double(2*pos.x/MR::getScreenWidth()-1):0,pane?double(2*pos.y/MR::getScreenHeight()-1):0);galaxyNativeBootTrace(line);}
    }else NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);
'@
$signature='void FileSelector::exeCreateConfirm() {'
$fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+$confirmationObserver)
$signature='void FileSelector::exeMiiSelect() {'
$fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+'    if(std::getenv("GALAXY_FRAMEBUFFER_PROBE_PROFILE_READY"))NativeFramebufferProbe::armSceneFrame(true);')
$demoStageRequest='    MR::requestChangeStageInGameAfterLoadingGameData();'
if(!$fileSelectorFontText.Contains($demoStageRequest)){throw 'First-stage framebuffer arm anchor missing'}
$fileSelectorFontText=$fileSelectorFontText.Replace($demoStageRequest,$demoStageRequest+"`n"+'    if(std::getenv("GALAXY_FRAMEBUFFER_PROBE_AFTER_PROFILE_START"))galaxyNativeBootTrace("Saved-profile game-data load completed; waiting for initialized gameplay scene capture");')
$signature='void FileSelector::exeCreate() {'
$fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+'    NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);if(mSpine&&mSpine->mStep%30==0){char line[160];std::snprintf(line,sizeof(line),"Diagnostic profile creation slot=%d saveActive=%u saveSuccess=%u",_B4?_B4->_140:-1,unsigned(GameSequenceFunction::isActiveSaveDataHandleSequence()),unsigned(GameSequenceFunction::isSuccessSaveDataHandleSequence()));galaxyNativeBootTrace(line);}')
$signature='bool FileSelector::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {'
if(!$fileSelectorFontText.Contains($signature)){throw 'FileSelector sensor-message trace anchor missing'}
$fileSelectorFontText=$fileSelectorFontText.Replace($signature,$signature+"`n"+'    static unsigned receivedMessages=0;if(receivedMessages++<16){char line[192];std::snprintf(line,sizeof(line),"FileSelector message msg=%u sender=%p receiver=%p",msg,static_cast<void*>(pSender),static_cast<void*>(pReceiver));galaxyNativeBootTrace(line);}')
[IO.File]::WriteAllText($fileSelectorFontSource,$fileSelectorFontText)
& $zig c++ @gameSceneCommon $fileSelectorFontSource '-o' $fileSelectorFontObject
if($LASTEXITCODE){throw 'Native FileSelector font provider failed to compile.'}
$fontFileUtilSource=Join-Path $tmp 'FileUtil-mii-font-traced.cpp'
$fontFileUtilObject=Join-Path $tmp 'FileUtil-mii-font-traced.obj'
$fontFileUtilText=Get-Content -LiteralPath (Join-Path $native 'src\generated\FileUtil.cpp') -Raw
$fontFileUtilText='#include <cstring>'+"`n"+$fontFileUtilText
$fontMountAnchor='        bool isExistLayoutArc = makeLayoutArchiveFileNameFromPrefix(layoutArcPath, sizeof(layoutArcPath), pFilePrefix, false);'
if(-not $fontFileUtilText.Contains($fontMountAnchor)){throw 'Mii font archive request trace anchor missing.'}
$fontFileUtilText=$fontFileUtilText.Replace($fontMountAnchor,$fontMountAnchor+"`n"+'        if(std::strstr(pFilePrefix,"MiiFont"))std::fprintf(stderr,"MiiFont request object=%d path=%s layout=%d path=%s\n",isExistObjArc,objArcPath,isExistLayoutArc,layoutArcPath);')
$fontReceiveAnchor='        return SingletonHolder< FileLoader >::get()->receiveArchive(filePath);'
if(-not $fontFileUtilText.Contains($fontReceiveAnchor)){throw 'Mii font archive receive trace anchor missing.'}
$fontFileUtilText=$fontFileUtilText.Replace($fontReceiveAnchor,'        auto* archive=SingletonHolder< FileLoader >::get()->receiveArchive(filePath);'+"`n"+'        if(std::strstr(pFilePath,"MiiFont"))std::fprintf(stderr,"MiiFont receive requested=%s resolved=%s archive=%p\n",pFilePath,filePath,(void*)archive);'+"`n"+'        return archive;')
[IO.File]::WriteAllText($fontFileUtilSource,$fontFileUtilText)
& $zig c++ @gameSceneCommon $fontFileUtilSource '-o' $fontFileUtilObject
if($LASTEXITCODE){throw 'Font-traced FileUtil provider failed to compile.'}
& (Join-Path $native 'build-sdk-mem-host.ps1')
if($LASTEXITCODE){throw 'Native SDK MEM heap providers failed to compile.'}
# Refresh shared FunctorV0F producers and all remaining cached FunctorBase
# clone consumers. The latter were compiled with clone at slot 0x18; the
# current native interface preserves the original clone slot at 0x08.
$callbackProducers=@(
 @{Unit='VolumeModelDrawer';Source=(Join-Path $root 'Petari\src\Game\LiveActor\VolumeModelDrawer.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-model-drawers-20261011\VolumeModelDrawer.obj')},
 @{Unit='BigBubbleHolder';Source=(Join-Path $root 'Petari\src\Game\MapObj\BigBubbleHolder.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\BigBubbleHolder.obj')},
 @{Unit='ClipAreaHolder';Source=(Join-Path $root 'Petari\src\Game\MapObj\ClipAreaHolder.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\ClipAreaHolder.obj')},
 @{Unit='GamePauseSequence';Source=(Join-Path $native 'src\generated\GamePauseSequence.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\GamePauseSequence.obj')},
 @{Unit='SaveDataHandleSequence';Source=(Join-Path $native 'src\generated\SaveDataHandleSequence.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\SaveDataHandleSequence.obj')},
 @{Unit='SwitchEventFunctorListener';Source=(Join-Path $root 'Petari\src\Game\Util\SwitchEventFunctorListener.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\SwitchEventFunctorListener.obj')},
 @{Unit='DemoActionKeeper';Source=(Join-Path $root 'Petari\src\Game\Demo\DemoActionKeeper.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-probe\DemoActionKeeper.obj')},
 @{Unit='DrawAdaptor';Source=(Join-Path $root 'Petari\src\Game\Player\DrawAdaptor.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-player-model-support-20261011\DrawAdaptor.obj')},
 @{Unit='FileSelectButton';Source=(Join-Path $root 'Petari\src\Game\Screen\FileSelectButton.cpp');OriginalObject=(Join-Path $native 'build\actor-runtime-file-select-20261011\FileSelectButton.obj')}
)
foreach($provider in $callbackProducers){
 $provider.Path=Join-Path $tmp ($provider.Unit+'-current-callback-abi.obj')
 $callbackUnitArgs=@($gameSceneCommon)
 if($provider.Unit-eq 'FileSelectButton'){$callbackUnitArgs+=@('-include',(Join-Path $native 'include-runtime\native_no_weak.h'))}
 & $zig c++ @callbackUnitArgs $provider.Source '-o' $provider.Path
 if($LASTEXITCODE){throw "Current callback-ABI producer failed to compile: $($provider.Unit)"}
}
# Refresh the complete native save boundary even when the input response uses
# cached original actor objects. The previous cache wrote host-order save bytes.
$nativeSaveProviders=@(& (Join-Path $native 'native-save-providers.ps1'))
foreach($provider in $nativeSaveProviders){
 $provider | Add-Member -NotePropertyName Path -NotePropertyValue (Join-Path $tmp ($provider.Unit+'-native-save.obj'))
 & $zig c++ @gameSceneCommon $provider.Source '-o' $provider.Path
 if($LASTEXITCODE){throw "Native save provider failed to compile: $($provider.Unit)"}
}
$errorWatcherObject=Join-Path $tmp 'GameSystemErrorWatcher.obj'
$errorWatcherSource=Join-Path $tmp 'GameSystemErrorWatcher-boot-traced.cpp'
$errorWatcherText=Get-Content -LiteralPath (Join-Path $native 'src\generated\GameSystemErrorWatcher.cpp') -Raw
$errorWatcherText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $errorWatcherText
$errorWatcherAnchors=@(
 @('    mWindow = new ErrorMessageWindow();','ErrorWatcher window allocation begin'),
 @('    mWindow->initWithoutIter();','ErrorWatcher window initialization begin'),
 @('    _1C = new TriggerChecker();','ErrorWatcher trigger checker allocation begin'),
 @('    _1C->setInput(false);','ErrorWatcher trigger checker initialized'),
 @('    initNerve(GET_NERVE_ANON(GameSystemErrorWatcherNoError));','ErrorWatcher nerve init begin')
)
foreach($anchor in $errorWatcherAnchors){$needle=$anchor[0];if(!$errorWatcherText.Contains($needle)){throw "ErrorWatcher trace anchor missing: $needle"};$errorWatcherText=$errorWatcherText.Replace($needle,'    galaxyNativeBootTrace("'+$anchor[1]+'");' + "`n" + $needle)}
$ctorBody='mPermissionUpdateWiiRemoteStatus(false) {'
if(!$errorWatcherText.Contains($ctorBody)){throw 'ErrorWatcher constructor body trace anchor missing.'}
$errorWatcherText=$errorWatcherText.Replace($ctorBody,$ctorBody + "`n" + '    galaxyNativeBootTrace("ErrorWatcher ctor body entered");')
[IO.File]::WriteAllText($errorWatcherSource,$errorWatcherText,[Text.UTF8Encoding]::new($false))
$errorWatcherCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-ffunction-sections','-fdata-sections','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER',
 '-target','x86_64-windows-gnu',"-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\RVLFaceLib\include",
 "-I$root\Petari\libs\nw4r\include", '-include',"$native\include-runtime\native_no_weak.h",'-include','revolution/types.h',
 '-include',"$native\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$native\include-runtime\nw4r\ut\Color.h",'-c')
& $zig c++ @errorWatcherCommon $errorWatcherSource '-o' $errorWatcherObject
if($LASTEXITCODE){throw 'Boot-traced GameSystemErrorWatcher provider failed to compile.'}
$errorWindowObject=Join-Path $tmp 'ErrorMessageWindow.obj'
$errorWindowSource=Join-Path $tmp 'ErrorMessageWindow-boot-traced.cpp'
$errorWindowText=Get-Content -LiteralPath (Join-Path $native 'src\generated\ErrorMessageWindow.cpp') -Raw
$errorWindowText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $errorWindowText
$errorWindowAnchors=@(
 @('void ErrorMessageWindow::init(const JMapInfoIter& rIter) {','ErrorMessageWindow init body entered'),
 @('    initLayoutManagerNoConvertFilename("ErrorMessageWindow", 1);','ErrorMessageWindow layout manager init begin'),
 @('    initNerve(GET_NERVE_ANON(ErrorMessageWindowHide));','ErrorMessageWindow nerve init begin'),
 @('    kill();','ErrorMessageWindow init kill begin')
)
foreach($anchor in $errorWindowAnchors){$needle=$anchor[0];if(!$errorWindowText.Contains($needle)){throw "ErrorMessageWindow trace anchor missing: $needle"};$errorWindowText=$errorWindowText.Replace($needle,$needle + "`n" + '    galaxyNativeBootTrace("'+$anchor[1]+'");')}
[IO.File]::WriteAllText($errorWindowSource,$errorWindowText,[Text.UTF8Encoding]::new($false))
$errorWindowObjectCommon=$errorWatcherCommon
& $zig c++ @errorWindowObjectCommon $errorWindowSource '-o' $errorWindowObject
if($LASTEXITCODE){throw 'Boot-traced ErrorMessageWindow provider failed to compile.'}
$layoutManagerObject=Join-Path $tmp 'LayoutManager.obj'
$layoutManagerSource=Join-Path $tmp 'LayoutManager-boot-traced.cpp'
$layoutManagerText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\Screen\LayoutManager.cpp') -Raw
$layoutManagerText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $layoutManagerText
$layoutManagerAnchors=@(
 @('    initArc(archiveName, pLayoutName);','LayoutManager constructor initArc complete'),
 @('void LayoutManager::initArc(const char* pArchiveName, const char* pLayoutName) {','LayoutManager initArc entered'),
 @('    mLayoutHolder = MR::createAndAddLayoutHolder(pArchiveName);','LayoutManager layout holder acquired'),
 @("    void* pLayoutRes = mLayoutHolder->GetResource('blyt', layoutResName, nullptr);",'LayoutManager layout resource lookup complete'),
 @('    mLayout->Build(pLayoutRes, mLayoutHolder);','LayoutManager layout Build complete'),
 @('    initPaneInfo();','LayoutManager pane info initialized'),
 @('    initGroupCtrlList();','LayoutManager group control list initialized'),
 @('    initDrawInfo();','LayoutManager draw info initialized'),
 @('    initTextBoxRecursive(mLayout->mpRootPane, nullptr, mLayoutName, textBoxBufferLength);','LayoutManager text boxes initialized'),
 @('    replaceIndDummyTexture();','LayoutManager dummy textures replaced'),
 @('        createAndAddRootPaneCtrl(rootPaneAnimLayerNum);','LayoutManager root pane controller created'),
 @('    mLayout->Animate(0);','LayoutManager initial animation complete'),
 @('    mLayout->CalculateMtx(mDrawInfo);','LayoutManager initial matrices calculated')
)
foreach($anchor in $layoutManagerAnchors){$needle=$anchor[0];if(!$layoutManagerText.Contains($needle)){throw "LayoutManager trace anchor missing: $needle"};$layoutManagerText=$layoutManagerText.Replace($needle,$needle + "`n" + '    galaxyNativeBootTrace("'+$anchor[1]+'");')}
$layoutManagerText=$layoutManagerText.Replace("    void* pLayoutRes = mLayoutHolder->GetResource('blyt', layoutResName, nullptr);", "    void* pLayoutRes = mLayoutHolder->GetResource('blyt', layoutResName, nullptr);" + "`n" + '    galaxyNativeBootTrace(pLayoutRes ? "LayoutManager layout resource found" : "LayoutManager layout resource is null");')
$layoutManagerText=$layoutManagerText.Replace('    snprintf(layoutResName, sizeof(layoutResName), "%s.brlyt", pLayoutName);','    snprintf(layoutResName, sizeof(layoutResName), "%s.brlyt", pLayoutName);' + "`n" + '    { char line[200]; std::snprintf(line,sizeof(line),"LayoutManager lookup archive=%s layout=%s request=%s resCount=%u",pArchiveName,pLayoutName,layoutResName,mLayoutHolder->mLayoutRes.mCount); galaxyNativeBootTrace(line); for(u32 i=0;i<mLayoutHolder->mLayoutRes.mCount;i++){ char row[200]; std::snprintf(row,sizeof(row),"LayoutManager resource[%u]=%s",i,mLayoutHolder->mLayoutRes.getResName(i)); galaxyNativeBootTrace(row); } }')
$layoutManagerText=$layoutManagerText.Replace('layoutBytes[4], layoutBytes[5], layoutBytes[6], layoutBytes[7], layoutBytes[8], layoutBytes[9]','layoutBytes ? layoutBytes[4] : 0, layoutBytes ? layoutBytes[5] : 0, layoutBytes ? layoutBytes[6] : 0, layoutBytes ? layoutBytes[7] : 0, layoutBytes ? layoutBytes[8] : 0, layoutBytes ? layoutBytes[9] : 0')
$recursivePaneSignature='void LayoutManager::initTextBoxRecursive(nw4r::lyt::Pane* pPane, nw4r::lyt::Pane* pUserDataPane, const char* pLayoutName, u32 textBoxBufferLength) {'
if(!$layoutManagerText.Contains($recursivePaneSignature)){throw 'LayoutManager text-box recursion trace anchor missing.'}
$layoutManagerText=$layoutManagerText.Replace($recursivePaneSignature,$recursivePaneSignature + "`n" + '    static unsigned long nativePaneTraceIndex = 0;' + "`n" + '    const auto nativePaneIndex = nativePaneTraceIndex++;' + "`n" + '    char nativePaneTrace[96]; std::snprintf(nativePaneTrace, sizeof(nativePaneTrace), "TextBox recursion pane %lu begin", nativePaneIndex); galaxyNativeBootTrace(nativePaneTrace);')
$layoutManagerText=$layoutManagerText.Replace('    if (pPane != nullptr && pPane->GetRuntimeTypeInfo()->IsDerivedFrom(pTextBoxRuntimeInfo)) {','    galaxyNativeBootTrace("TextBox recursion runtime type check begin");' + "`n" + '    if (pPane != nullptr && pPane->GetRuntimeTypeInfo()->IsDerivedFrom(pTextBoxRuntimeInfo)) {' + "`n" + '        galaxyNativeBootTrace("TextBox recursion pane is a textbox");')
$layoutManagerText=$layoutManagerText.Replace('        pTextBox = nullptr;' + "`n" + '    }','        pTextBox = nullptr;' + "`n" + '        galaxyNativeBootTrace("TextBox recursion pane is not a textbox");' + "`n" + '    }')
$layoutManagerText=$layoutManagerText.Replace('    LayoutCoreUtil::getPaneUserData(pPane, userData);','    galaxyNativeBootTrace("TextBox recursion userdata read begin");' + "`n" + '    LayoutCoreUtil::getPaneUserData(pPane, userData);' + "`n" + '    galaxyNativeBootTrace("TextBox recursion userdata read complete");')
$layoutManagerText=$layoutManagerText.Replace('            LayoutCoreUtil::initTextBoxPane(pTextBox, nullptr, textBoxBufferLength);','            galaxyNativeBootTrace("TextBox recursion buffer init begin");' + "`n" + '            LayoutCoreUtil::initTextBoxPane(pTextBox, nullptr, textBoxBufferLength);' + "`n" + '            galaxyNativeBootTrace("TextBox recursion buffer init complete");')
$layoutManagerText=$layoutManagerText.Replace('    nw4r::lyt::PaneList& rPaneList = pPane->mChildList;','    galaxyNativeBootTrace("TextBox recursion child-list begin");' + "`n" + '    nw4r::lyt::PaneList& rPaneList = pPane->mChildList;')
[IO.File]::WriteAllText($layoutManagerSource,$layoutManagerText,[Text.UTF8Encoding]::new($false))
$layoutManagerCommon=$errorWatcherCommon
& $zig c++ @layoutManagerCommon $layoutManagerSource '-o' $layoutManagerObject
if($LASTEXITCODE){throw 'Boot-traced LayoutManager provider failed to compile.'}
$lytLayoutObject=Join-Path $tmp 'lyt_layout.obj'
$lytLayoutSource=Join-Path $tmp 'lyt_layout-boot-traced.cpp'
$lytLayoutText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_layout.cpp') -Raw
$lytLayoutText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytLayoutText
$blockHead='                const res::DataBlockHeader* pDataBlockHead = static_cast< const res::DataBlockHeader* >(dataPtr);'
if(!$lytLayoutText.Contains($blockHead)){throw 'NW4R layout block trace anchor missing.'}
$lytLayoutText=$lytLayoutText.Replace($blockHead,$blockHead + "`n" + '                char layoutBlockDiag[48]; std::snprintf(layoutBlockDiag, sizeof(layoutBlockDiag), "RLYT block %c%c%c%c", pDataBlockHead->kind[0], pDataBlockHead->kind[1], pDataBlockHead->kind[2], pDataBlockHead->kind[3]); galaxyNativeBootTrace(layoutBlockDiag);')
$paneBuild='                    Pane* pPane = BuildPaneObj(detail::GetSignatureInt(pDataBlockHead->kind), dataPtr, resBlockSet);'
if(!$lytLayoutText.Contains($paneBuild)){throw 'NW4R pane build trace anchor missing.'}
$lytLayoutText=$lytLayoutText.Replace($paneBuild,'                    galaxyNativeBootTrace("RLYT pane build begin");' + "`n" + $paneBuild + "`n" + '                    galaxyNativeBootTrace("RLYT pane build complete");')
$lytLayoutText=$lytLayoutText.Replace('        AnimTransform* Layout::CreateAnimTransform() {','        AnimTransform* Layout::CreateAnimTransform() {' + "`n" + '            galaxyNativeBootTrace("Layout CreateAnimTransform allocate begin");')
$lytLayoutText=$lytLayoutText.Replace('                mAnimTransList.PushBack(pAnimTrans);','                galaxyNativeBootTrace("Layout CreateAnimTransform list insert begin");' + "`n" + '                mAnimTransList.PushBack(pAnimTrans);' + "`n" + '                galaxyNativeBootTrace("Layout CreateAnimTransform list insert complete");')
$lytLayoutText=$lytLayoutText.Replace('        AnimTransform* Layout::CreateAnimTransform(const AnimResource& animRes, ResourceAccessor* pResAcsr) {','        AnimTransform* Layout::CreateAnimTransform(const AnimResource& animRes, ResourceAccessor* pResAcsr) {' + "`n" + '            galaxyNativeBootTrace("Layout CreateAnimTransform resource overload entered");')
$lytLayoutText=$lytLayoutText.Replace('            AnimTransform* const pAnimTrans = CreateAnimTransform();','            galaxyNativeBootTrace("Layout CreateAnimTransform block valid");' + "`n" + '            AnimTransform* const pAnimTrans = CreateAnimTransform();' + "`n" + '            galaxyNativeBootTrace(pAnimTrans ? "Layout CreateAnimTransform object returned" : "Layout CreateAnimTransform allocation failed");')
$lytLayoutText=$lytLayoutText.Replace('                pAnimTrans->SetResource(pAnimBlock, pResAcsr);','                { char line[200]; void** vtable = *reinterpret_cast<void***>(pAnimTrans); std::snprintf(line,sizeof(line),"Layout animation dispatch object=%p vtable=%p slot=%p",static_cast<void*>(pAnimTrans),static_cast<void*>(vtable),vtable ? vtable[1] : nullptr); galaxyNativeBootTrace(line); }' + "`n" + '                pAnimTrans->SetResource(pAnimBlock, pResAcsr);' + "`n" + '                galaxyNativeBootTrace("Layout animation virtual SetResource returned");')
[IO.File]::WriteAllText($lytLayoutSource,$lytLayoutText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytLayoutSource '-o' $lytLayoutObject
if($LASTEXITCODE){throw 'Host NW4R layout parser provider failed to compile.'}
$lytCommonObject=Join-Path $tmp 'lyt_common.obj'
& $zig c++ @layoutManagerCommon -c (Join-Path $root 'Petari\src\nw4r\lyt\lyt_common.cpp') '-o' $lytCommonObject
if($LASTEXITCODE){throw 'Host NW4R layout common provider failed to compile.'}
$lytPaneObject=Join-Path $tmp 'lyt_pane.obj'
$lytPaneSource=Join-Path $tmp 'lyt_pane-boot-traced.cpp'
$lytPaneText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_pane.cpp') -Raw
$lytPaneText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytPaneText
$paneCtor='Pane::Pane(const res::Pane* pRes) {'
if(!$lytPaneText.Contains($paneCtor)){throw 'NW4R Pane constructor trace anchor missing.'}
$lytPaneText=$lytPaneText.Replace($paneCtor,$paneCtor + "`n" + '            galaxyNativeBootTrace("NW4R Pane ctor body entered");')
[IO.File]::WriteAllText($lytPaneSource,$lytPaneText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytPaneSource '-o' $lytPaneObject
if($LASTEXITCODE){throw 'Boot-traced NW4R Pane provider failed to compile.'}
$lytWindowObject=Join-Path $tmp 'lyt_window.obj'
$lytWindowSource=Join-Path $tmp 'lyt_window-boot-traced.cpp'
$lytWindowText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_window.cpp') -Raw
$lytWindowText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytWindowText
$windowCtor='Window::Window(const res::Window* pRes, const ResBlockSet& rBlockSet) : Pane(pRes) {'
if(!$lytWindowText.Contains($windowCtor)){throw 'NW4R Window constructor trace anchor missing.'}
$lytWindowText=$lytWindowText.Replace($windowCtor,$windowCtor + "`n" + '            galaxyNativeBootTrace("NW4R Window ctor body entered");')
$lytWindowText=$lytWindowText.Replace('            mContentInflation = pRes->inflation;','            mContentInflation = pRes->inflation;' + "`n" + '            galaxyNativeBootTrace("NW4R Window inflation read");')
[IO.File]::WriteAllText($lytWindowSource,$lytWindowText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytWindowSource '-o' $lytWindowObject
if($LASTEXITCODE){throw 'Boot-traced NW4R Window provider failed to compile.'}
$lytPictureObject=Join-Path $tmp 'lyt_picture.obj'
$lytPictureSource=Join-Path $tmp 'lyt_picture-boot-traced.cpp'
$lytPictureText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_picture.cpp') -Raw
$lytPictureText='#include <Native/DiagnosticMenuInput.hpp>'+"`n"+$lytPictureText
$lytPictureText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytPictureText
$lytPictureText="#include <cstdio>`n#include <stdexcept>`n"+$lytPictureText
$pictureSetup='            bool useVtxColor = mpMaterial->SetupGX(detail::IsModulateVertexColor(mVtxColors, mGlbAlpha), mGlbAlpha);'
if(!$lytPictureText.Contains($pictureSetup)){throw 'Picture material draw diagnostic anchor missing.'}
$lytPictureText=$lytPictureText.Replace($pictureSetup,'            bool useVtxColor; try { useVtxColor = mpMaterial->SetupGX(detail::IsModulateVertexColor(mVtxColors, mGlbAlpha), mGlbAlpha); } catch(const std::exception& error) { char detail[768]; std::snprintf(detail,sizeof(detail),"Picture %.16s material=%p: %s",mName,static_cast<void*>(mpMaterial),error.what()); throw std::runtime_error(detail); }')
$pictureUvAnchor='            detail::SetVertexFormat(useVtxColor, mTexCoordAry.GetSize());'
if(!$lytPictureText.Contains($pictureUvAnchor)){throw 'Picture UV provenance anchor missing'}
$lytPictureText=$lytPictureText.Replace($pictureUvAnchor,'            if(mTexCoordAry.GetSize()==0&&mpMaterial->GetTextureNum()>0&&NativeDiagnosticInput::state().phase==NativeDiagnosticInput::Phase::Menu){static unsigned count=0;if(count++<16){char line[256];std::snprintf(line,sizeof(line),"Pointer quad Picture name=%.16s coords=%u capacity=%u textures=%u generators=%u size=(%.2f,%.2f)",mName,unsigned(mTexCoordAry.GetSize()),unsigned(mTexCoordAry.mCap),unsigned(mpMaterial->GetTextureNum()),unsigned(mpMaterial->mGXMemNum.texCoordGen),double(mSize.width),double(mSize.height));galaxyNativeBootTrace(line);}}'+"`n"+$pictureUvAnchor)
$pictureCtor='Picture::Picture(const res::Picture* pRes, const ResBlockSet& rBlockSet) : Pane(pRes) {'
if(!$lytPictureText.Contains($pictureCtor)){throw 'NW4R Picture constructor trace anchor missing.'}
$lytPictureText=$lytPictureText.Replace($pictureCtor,$pictureCtor + "`n" + '            galaxyNativeBootTrace("NW4R Picture ctor body entered");')
$pictureAnchors=@(
 @('            Init(num);','NW4R Picture texture coordinates initialized'),
 @('            void* pMaterialBuf = Layout::AllocMemory(sizeof(Material));','NW4R Picture material storage allocated'),
 @('                    pictureBytes + sizeof(res::Pane) + sizeof(pRes->vtxCols));','NW4R Picture material lookup complete'),
 @('                    rBlockSet.pMaterialList, materialOffset);','NW4R Picture material pointer resolved'),
 @('                Material* pMaterial = new (pMaterialBuf) Material(pResMaterial, rBlockSet);','NW4R Picture material constructed')
)
foreach($anchor in $pictureAnchors){$needle=$anchor[0];if(!$lytPictureText.Contains($needle)){throw "NW4R Picture trace anchor missing: $needle"};$lytPictureText=$lytPictureText.Replace($needle,$needle + "`n" + '                galaxyNativeBootTrace("'+$anchor[1]+'");')}
[IO.File]::WriteAllText($lytPictureSource,$lytPictureText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytPictureSource '-o' $lytPictureObject
if($LASTEXITCODE){throw 'Boot-traced NW4R Picture provider failed to compile.'}
$lytMaterialObject=Join-Path $tmp 'lyt_material.obj'
$lytMaterialSource=Join-Path $tmp 'lyt_material-boot-traced.cpp'
$lytMaterialText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_material.cpp') -Raw
$lytMaterialText='#define TRUE 1' + "`n" + '#define ROUND_UP(value,alignment) (((value)+(alignment)-1) & -((alignment)))' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytMaterialText
$materialTextureNeedle=@'
                        const u16 textureIndex = detail::ReadBigEndianU16(
                            reinterpret_cast<const u8*>(&pResTexMap[si].texIdx));
'@
$materialTextureNeedle=$materialTextureNeedle.TrimEnd("`r","`n")
if(!$lytMaterialText.Contains($materialTextureNeedle)){throw 'NW4R Material texture lookup trace anchor missing.'}
$lytMaterialReplacement=$materialTextureNeedle + "`n" + '                        { char line[220]; std::snprintf(line,sizeof(line),"NW4R texture offset begin material=%p list=%p texmaps=%p si=%u index=%u count=%u",static_cast<const void*>(pRes),static_cast<const void*>(textures),static_cast<const void*>(pResTexMap),unsigned(si),unsigned(textureIndex),unsigned(textureCount)); galaxyNativeBootTrace(line); }'
$lytMaterialText=$lytMaterialText.Replace($materialTextureNeedle,$lytMaterialReplacement)
$lytMaterialText=$lytMaterialText.Replace('                            textureBytes + offsetof(res::Texture, nameStrOffset));','                            textureBytes + offsetof(res::Texture, nameStrOffset));' + "`n" + '                        { char line[180]; std::snprintf(line,sizeof(line),"NW4R texture name offset=%u",unsigned(nameOffset)); galaxyNativeBootTrace(line); }')
$lytMaterialText=$lytMaterialText.Replace('                        void* const pTplRes = resBlockSet.pResAccessor->GetResource(0x74696D67, fileName, nullptr);','                        galaxyNativeBootTrace("NW4R texture resource lookup begin");' + "`n" + '                        void* const pTplRes = resBlockSet.pResAccessor->GetResource(0x74696D67, fileName, nullptr);' + "`n" + '                        { char line[180]; std::snprintf(line,sizeof(line),"NW4R texture resource lookup returned resource=%p",pTplRes); galaxyNativeBootTrace(line); }')
[IO.File]::WriteAllText($lytMaterialSource,$lytMaterialText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytMaterialSource '-o' $lytMaterialObject
if($LASTEXITCODE){throw 'Boot-traced NW4R Material provider failed to compile.'}
$lytTexMapObject=Join-Path $tmp 'lyt_texMap.obj'
$lytTexMapSource=Join-Path $tmp 'lyt_texMap-boot-traced.cpp'
$lytTexMapText=Get-Content -LiteralPath (Join-Path $native 'src\generated\lyt_texMap.cpp') -Raw
$lytTexMapText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytTexMapText
$lytTexMapText="#include <cstdio>`n#include <Native/HeapPlacement.hpp>`n"+$lytTexMapText
$textureGet='        void TexMap::Get(_GXTexObj* pTexObj) const {'
if(!$lytTexMapText.Contains($textureGet)){throw 'TexMap draw diagnostic anchor missing.'}
$textureCheck=@'
            try { Native::gpuResources().byteAddress(mImage); }
            catch(const std::exception& error) {
                auto* heap=JKRHeap::findFromRoot(mImage);
                const void* tpl=nullptr;std::size_t tplBytes=0,tplRefs=0;bool tplOwns=false;
                { auto& registry=Native::detail::tplResourceRegistry();std::lock_guard<std::mutex> lock(registry.mutex);
                  auto address=reinterpret_cast<uintptr_t>(mImage);
                  for(const auto& entry:registry.resources){auto base=reinterpret_cast<uintptr_t>(entry.first);if(address>=base&&address-base<entry.second.size){tpl=entry.first;tplBytes=entry.second.size;tplRefs=entry.second.references;tplOwns=entry.second.ownsGpuMapping;break;}}
                }
                char detail[576];std::snprintf(detail,sizeof(detail),"TexMap=%p image=%p size=%ux%u format=%u heap=%p heapExtent=%p..%p tpl=%p bytes=%zu refs=%zu owns=%u: %s",static_cast<const void*>(this),mImage,unsigned(mWidth),unsigned(mHeight),unsigned(GetTexelFormat()),static_cast<void*>(heap),heap?heap->getStartAddr():nullptr,heap?heap->getEndAddr():nullptr,tpl,tplBytes,tplRefs,unsigned(tplOwns),error.what());
                throw std::runtime_error(detail);
            }
'@
$lytTexMapText=$lytTexMapText.Replace($textureGet,$textureGet+"`n"+$textureCheck)
$tplReplaceAnchor='        void TexMap::ReplaceImage(TPLPalette* p, u32 id) {'
if(!$lytTexMapText.Contains($tplReplaceAnchor)){throw 'Native TPL decoder trace anchor missing.'}
$lytTexMapText=$lytTexMapText.Replace($tplReplaceAnchor,$tplReplaceAnchor + "`n" + '            galaxyNativeBootTrace("Native TPL ReplaceImage entered");')
$tplMetadataAnchor='            SetTexelFormat(GXTexFmt(format));'
$lytTexMapText=$lytTexMapText.Replace($tplMetadataAnchor,$tplMetadataAnchor + "`n" + '            { char line[180]; std::snprintf(line,sizeof(line),"Native TPL decoded width=%u height=%u format=%u bytes=%zu",unsigned(width),unsigned(height),unsigned(format),imageBytes); galaxyNativeBootTrace(line); }')
[IO.File]::WriteAllText($lytTexMapSource,$lytTexMapText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytTexMapSource '-o' $lytTexMapObject
if($LASTEXITCODE){throw 'Boot-traced NW4R TPL provider failed to compile.'}
$lytAnimationObject=Join-Path $tmp 'lyt_animation-big-endian.obj'
$lytAnimationSource=Join-Path $tmp 'lyt_animation-boot-traced.cpp'
$lytAnimationText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_animation.cpp') -Raw
$lytAnimationText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytAnimationText
$lytAnimationText=$lytAnimationText.Replace('            mpRes = pBlock;','            { char line[160]; std::snprintf(line,sizeof(line),"AnimTransform SetResource block=%p",static_cast<const void*>(pBlock)); galaxyNativeBootTrace(line); }' + "`n" + '            mpRes = pBlock;')
$lytAnimationText=$lytAnimationText.Replace('            const u16 fileCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, fileNum));','            const u16 fileCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, fileNum));' + "`n" + '            { char line[180]; const u16 animCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, animContNum)); std::snprintf(line,sizeof(line),"AnimTransform counts files=%u animations=%u",unsigned(fileCount),unsigned(animCount)); galaxyNativeBootTrace(line); }')
$lytAnimationText=$lytAnimationText.Replace('            const u16 headerSize = detail::ReadBigEndianU16(fileBytes + offsetof(res::BinaryFileHeader, headerSize));','            const u16 headerSize = detail::ReadBigEndianU16(fileBytes + offsetof(res::BinaryFileHeader, headerSize));' + "`n" + '            { char line[160]; std::snprintf(line,sizeof(line),"RLAN header size=%u blocks=%u",unsigned(headerSize),unsigned(detail::ReadBigEndianU16(fileBytes + offsetof(res::BinaryFileHeader, dataBlocks)))); galaxyNativeBootTrace(line); }')
$lytAnimationText=$lytAnimationText.Replace('                const u32 blockSize = detail::ReadBigEndianU32(blockBytes + offsetof(res::DataBlockHeader, size));','                const u32 blockSize = detail::ReadBigEndianU32(blockBytes + offsetof(res::DataBlockHeader, size));' + "`n" + '                { char line[180]; std::snprintf(line,sizeof(line),"RLAN block kind=%08X size=%u",unsigned(detail::GetSignatureInt(pDataBlockHead->kind)),unsigned(blockSize)); galaxyNativeBootTrace(line); }')
$lytAnimationText=$lytAnimationText.Replace('            }' + "`n" + '        }' + "`n" + "`n" + '        void AnimResource::Init()', '            }' + "`n" + '            { char line[160]; std::snprintf(line,sizeof(line),"RLAN parsed pai1=%p",static_cast<const void*>(mpResBlock)); galaxyNativeBootTrace(line); }' + "`n" + '        }' + "`n" + "`n" + '        void AnimResource::Init()')
[IO.File]::WriteAllText($lytAnimationSource,$lytAnimationText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytAnimationSource '-o' $lytAnimationObject
if($LASTEXITCODE){throw 'Big-endian NW4R animation resource provider failed to compile.'}
$layoutAnmPlayerObject=Join-Path $tmp 'LayoutAnmPlayer.obj'
& $zig c++ @layoutManagerCommon -c (Join-Path $root 'Petari\src\Game\Animation\LayoutAnmPlayer.cpp') '-o' $layoutAnmPlayerObject
if($LASTEXITCODE){throw 'Layout animation null-resource guard provider failed to compile.'}
$lytTextBoxObject=Join-Path $tmp 'lyt_textBox.obj'
$lytTextBoxSource=Join-Path $tmp 'lyt_textBox-host.cpp'
$lytTextBoxText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\nw4r\lyt\lyt_textBox.cpp') -Raw
$lytTextBoxText=$lytTextBoxText.Replace('#include <wstring.h>','#include <wchar.h>')
$lytTextBoxText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $lytTextBoxText
$textBoxAllocatorSignature='        void TextBox::AllocStringBuffer(u16 len) {'
if(!$lytTextBoxText.Contains($textBoxAllocatorSignature)){throw 'NW4R TextBox allocator trace anchor missing.'}
$lytTextBoxText=$lytTextBoxText.Replace($textBoxAllocatorSignature,$textBoxAllocatorSignature + "`n" + '            galaxyNativeBootTrace("NW4R TextBox allocator entered");')
$lytTextBoxText=$lytTextBoxText.Replace('                FreeStringBuffer();','                galaxyNativeBootTrace("NW4R TextBox free old buffer begin");' + "`n" + '                FreeStringBuffer();' + "`n" + '                galaxyNativeBootTrace("NW4R TextBox free old buffer complete");')
$lytTextBoxText=$lytTextBoxText.Replace('                mTextBuf = static_cast< wchar_t* >(Layout::AllocMemory(bytes));','                galaxyNativeBootTrace("NW4R TextBox Layout allocator begin");' + "`n" + '                mTextBuf = static_cast< wchar_t* >(Layout::AllocMemory(bytes));' + "`n" + '                galaxyNativeBootTrace(mTextBuf ? "NW4R TextBox Layout allocator returned memory" : "NW4R TextBox Layout allocator returned null");')
$lytTextBoxText=$lytTextBoxText.Replace('            if (bytes > mTextBufBytes) {','            galaxyNativeBootTrace("NW4R TextBox allocation size checked");' + "`n" + '            if (bytes > mTextBufBytes) {')
$textBoxFreeSignature='        void TextBox::FreeStringBuffer() {'
if(!$lytTextBoxText.Contains($textBoxFreeSignature)){throw 'NW4R TextBox free trace anchor missing.'}
$lytTextBoxText=$lytTextBoxText.Replace($textBoxFreeSignature,$textBoxFreeSignature + "`n" + '            char nativeFreeTrace[160]; std::snprintf(nativeFreeTrace, sizeof(nativeFreeTrace), "NW4R TextBox free entered self=%p buffer=%p allocator=%p", static_cast<void*>(this), static_cast<void*>(mTextBuf), static_cast<void*>(Layout::mspAllocator)); galaxyNativeBootTrace(nativeFreeTrace);')
$lytTextBoxText=$lytTextBoxText.Replace('            Layout::FreeMemory(mTextBuf);','            galaxyNativeBootTrace("NW4R TextBox Layout::FreeMemory begin");' + "`n" + '            Layout::FreeMemory(mTextBuf);' + "`n" + '            galaxyNativeBootTrace("NW4R TextBox Layout::FreeMemory returned");')
[IO.File]::WriteAllText($lytTextBoxSource,$lytTextBoxText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $lytTextBoxSource '-o' $lytTextBoxObject
if($LASTEXITCODE){throw 'Host NW4R TextBox provider failed to compile.'}
$layoutCoreUtilObject=Join-Path $tmp 'LayoutCoreUtil.obj'
$layoutCoreUtilSource=Join-Path $tmp 'LayoutCoreUtil-boot-traced.cpp'
$layoutCoreUtilText=Get-Content -LiteralPath (Join-Path $native 'src\generated\LayoutCoreUtil.cpp') -Raw
$layoutCoreUtilText='extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $layoutCoreUtilText
$layoutCoreUtilAnchors=@(
 @('        CustomTagProcessor* processor = new CustomTagProcessor(pTextBox);','LayoutCoreUtil custom tag processor created'),
 @('        pTextBox->mpTagProcessor = processor;','LayoutCoreUtil tag processor installed'),
 @('        reallocateTextBoxMessage(pTextBox, allocSize - 1);','LayoutCoreUtil buffer allocation begin'),
 @('        setTextBoxMessage(pTextBox, defaultStr);','LayoutCoreUtil default string set begin')
)
foreach($anchor in $layoutCoreUtilAnchors){$needle=$anchor[0];if(!$layoutCoreUtilText.Contains($needle)){throw "LayoutCoreUtil trace anchor missing: $needle"};$layoutCoreUtilText=$layoutCoreUtilText.Replace($needle,'        galaxyNativeBootTrace("'+$anchor[1]+'");' + "`n" + $needle + "`n" + '        galaxyNativeBootTrace("'+$anchor[1]+' complete");')}
$layoutCoreUtilText=$layoutCoreUtilText.Replace('    void initTextBoxPane(nw4r::lyt::TextBox* pTextBox, const char* pMessageId, u32 allocSize) {','    void initTextBoxPane(nw4r::lyt::TextBox* pTextBox, const char* pMessageId, u32 allocSize) {' + "`n" + '        galaxyNativeBootTrace("LayoutCoreUtil initTextBoxPane entered");')
[IO.File]::WriteAllText($layoutCoreUtilSource,$layoutCoreUtilText,[Text.UTF8Encoding]::new($false))
& $zig c++ @layoutManagerCommon -c $layoutCoreUtilSource '-o' $layoutCoreUtilObject
if($LASTEXITCODE){throw 'Boot-traced LayoutCoreUtil provider failed to compile.'}
$memoryUtilObject=Join-Path $tmp 'MemoryUtil.obj'
$memoryUtilSource=Join-Path $tmp 'MemoryUtil-boot-traced.cpp'
$memoryUtilText=Get-Content -LiteralPath (Join-Path $native 'src\generated\MemoryUtil.cpp') -Raw
$memoryUtilText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $memoryUtilText
$memoryFreeSignature='    void NewDeleteAllocator::free(MEMAllocator* pAllocator, void* pPtr) {'
if(!$memoryUtilText.Contains($memoryFreeSignature)){throw 'NewDeleteAllocator free trace anchor missing.'}
$memoryUtilText=$memoryUtilText.Replace($memoryFreeSignature,$memoryFreeSignature + "`n" + '        char nativeFreeCallbackTrace[128]; std::snprintf(nativeFreeCallbackTrace, sizeof(nativeFreeCallbackTrace), "NewDeleteAllocator free entered allocator=%p memory=%p", static_cast<void*>(pAllocator), pPtr); galaxyNativeBootTrace(nativeFreeCallbackTrace);')
$memoryUtilText=$memoryUtilText.Replace('        JKRHeap::free(pPtr, nullptr);','        JKRHeap::free(pPtr, nullptr);' + "`n" + '        galaxyNativeBootTrace("NewDeleteAllocator free returned");')
[IO.File]::WriteAllText($memoryUtilSource,$memoryUtilText,[Text.UTF8Encoding]::new($false))
& $zig c++ @errorWatcherCommon $memoryUtilSource '-o' $memoryUtilObject
if($LASTEXITCODE){throw 'Boot-traced NewDeleteAllocator provider failed to compile.'}
$fileHolderObject=Join-Path $tmp 'FileHolder-heap-owned-delete.obj'
$nandProviderObject=Join-Path $tmp 'native_nand_port-current.obj'
& $zig c++ @heapCommon -c (Join-Path $native 'src\native_nand_port.cpp') '-o' $nandProviderObject
if($LASTEXITCODE){throw 'Current native NAND provider compilation failed.'}
& $zig c++ @heapCommon -c (Join-Path $native 'src\generated\FileHolder.cpp') '-o' $fileHolderObject
if($LASTEXITCODE){throw 'Heap-owned FileHolder provider failed to compile.'}
$response=$inputResponse
# The original image-effect factory includes screen blur; the actor audit did
# not retain that provider because its constructor was previously unreachable.
$screenBlurObject=Join-Path $tmp 'ScreenBlurEffect-current.obj'
& $zig c++ @gameSceneCommon (Join-Path $root 'Petari\src\Game\Screen\ScreenBlurEffect.cpp') '-o' $screenBlurObject
if($LASTEXITCODE){throw 'Original screen-blur provider compilation failed.'}
$screenBlurMatches=[regex]::Matches($response,'"[^"\r\n]*\\ScreenBlurEffect\.(?:obj|cpp)"')
if($screenBlurMatches.Count-gt 1){throw 'Multiple screen-blur providers in input response.'}
if($screenBlurMatches.Count-eq 1){$response=$response.Replace($screenBlurMatches[0].Value,'"'+$screenBlurObject+'"')}
else{$response+=' "'+$screenBlurObject+'"'}
$npcFunctionMatches=[regex]::Matches($response,'"[^"\r\n]*\\NPCFunction\.obj"')
if($npcFunctionMatches.Count-ne 1){throw 'Expected one original NPCFunction provider.'}
$response=$response.Replace($npcFunctionMatches[0].Value,'"'+$npcFunctionObject+'"')
$response='-DGALAXY_NATIVE_JAI_SOUND_ID=1 -DGALAXY_NATIVE_JPA_USERWORK_64=1 '+$response
# These cached units inline JAISoundID accessors. Keep every numeric-ID consumer coherent.
foreach($provider in $soundIdRefreshUnits){
 $soundIdObject=Join-Path $tmp ($provider.Unit+'-native-id.obj')
 & $zig c++ @gameSceneCommon '-include' (Join-Path $native 'include\Native\HeapPlacement.hpp') '-Wno-c++11-narrowing' $provider.Source '-o' $soundIdObject
 if($LASTEXITCODE){throw "Native sound-ID consumer compile failed: $($provider.Unit)"}
 $idMatches=[regex]::Matches($response,'"[^"\r\n]*\\'+$provider.Unit+'\.obj"')
 if($idMatches.Count-ne 1){throw "Expected one cached native sound-ID consumer: $($provider.Unit)"}
 $response=$response.Replace($idMatches[0].Value,'"'+$soundIdObject+'"')
}
# Refresh platform input objects instead of retaining the actor audit's old cache.
$inputRefreshCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-target','x86_64-windows-gnu',"-I$native\include-runtime","-I$native\include",'-include','revolution/types.h','-include',"$native\include-runtime\revolution\os.h",'-include',"$native\include-runtime\revolution\gx\GXVert.h","-I$root\Petari\libs\RVL_SDK\include")
foreach($inputUnit in @('native_kpad_input','native_keyboard_kpad','native_wpad_port')){
 $inputObject=Join-Path $tmp ($inputUnit+'-current.obj')
 & $zig c++ @inputRefreshCommon -c (Join-Path $native ('src\'+$inputUnit+'.cpp')) -o $inputObject
 if($LASTEXITCODE){throw "Current native input provider compile failed: $inputUnit"}
 $inputMatches=[regex]::Matches($response,'"[^"\r\n]*native-input-provider\\'+$inputUnit+'\.obj"')
 if($inputMatches.Count-ne 1){throw "Expected one cached native input provider: $inputUnit"}
 $response=$response.Replace($inputMatches[0].Value,'"'+$inputObject+'"')
}
$nandProviderMatch=[regex]::Match($response,'"[^"\r\n]*\\native_nand_port\.obj"')
if(!$nandProviderMatch.Success){throw 'Missing cached native NAND provider.'}
$response=$response.Replace($nandProviderMatch.Value,'"'+$nandProviderObject+'"')
$nandThreadMatch=[regex]::Match($response,'"[^"\r\n]*\\NANDManagerThread\.obj"')
if(!$nandThreadMatch.Success){throw 'Missing cached NANDManagerThread provider.'}
$nandThreadText=Get-Content -LiteralPath (Join-Path $root 'Petari\src\Game\System\NANDManagerThread.cpp') -Raw
$nandReadAnchor='    pRequestInfo->mResult = NANDOpen(pRequestInfo->mPath, &info, NAND_ACCESS_READ);'
if(!$nandThreadText.Contains($nandReadAnchor)){throw 'NAND worker read trace anchor missing.'}
$nandThreadText='#include <cstdio>'+"`n"+$nandThreadText.Replace($nandReadAnchor,$nandReadAnchor+"`n"+'    std::fprintf(stderr, "NAND worker open result=%d request=%p\n", int(pRequestInfo->mResult), static_cast<void*>(pRequestInfo));')
$nandThreadSource=Join-Path $tmp 'NANDManagerThread-current-traced.cpp'
$nandThreadObject=Join-Path $tmp 'NANDManagerThread-current.obj'
[IO.File]::WriteAllText($nandThreadSource,$nandThreadText,[Text.UTF8Encoding]::new($false))
& $zig c++ @heapCommon -c $nandThreadSource '-o' $nandThreadObject
if($LASTEXITCODE){throw 'Current NAND worker compilation failed.'}
$response=$response.Replace($nandThreadMatch.Value,'"'+$nandThreadObject+'"')
# These providers use current message, scenario, or layout adapters. Rebuild
# them so incremental links preserve fixes to the native resource structures.
$messageProviderCommon=@('-DGALAXY_NATIVE_JAI_SOUND_ID=1','-std=c++17','-O2','-target','x86_64-windows-gnu',
 '-DGALAXY_ORIGINAL_GD_GEOMETRY','-DGALAXY_SHARED_RUNTIME_MATH','-DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER','-UNDEBUG',
 "-I$native\src", "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
 "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include", "-I$root\Petari\libs\nw4r\include",
 '-include','revolution/types.h','-include',"$native\include-runtime\revolution\os.h",
 '-include',"$native\include-runtime\revolution\gx\GXVert.h",'-include',"$native\include-runtime\revolution\os\OSTime.h",'-c')
foreach($provider in @(
 @('MessageUtil', (Join-Path $root 'Petari\src\Game\Util\MessageUtil.cpp')),
 @('ReplaceTagProcessor', (Join-Path $root 'Petari\src\Game\Screen\ReplaceTagProcessor.cpp')),
 @('CustomTagProcessor', (Join-Path $native 'src\generated\CustomTagProcessor.cpp')),
 @('MessageHolder', (Join-Path $native 'src\generated\MessageHolder.cpp')),
 @('MessageTagSkipTagProcessor', (Join-Path $native 'src\generated\MessageTagSkipTagProcessor.cpp')),
 @('ScenarioDataParser', (Join-Path $native 'src\generated\ScenarioDataParser.cpp')),
 @('NameObjListExecutor', (Join-Path $root 'Petari\src\Game\NameObj\NameObjListExecutor.cpp')),
 # PaneList's native node offset is eight bytes on x64. These providers
 # traverse it inline, so old probe objects must not be mixed with current
 # Pane/Window/Layout objects.
 @('LayoutPaneCtrl', (Join-Path $root 'Petari\src\Game\Screen\LayoutPaneCtrl.cpp')),
 @('LayoutUtil', (Join-Path $native 'src\generated\LayoutUtil.cpp')),
 @('TalkTextFormer', (Join-Path $root 'Petari\src\Game\NPC\TalkTextFormer.cpp'))
)){
 $providerMatch=[regex]::Match($response,'"[^"\r\n]*\\'+$provider[0]+'\.obj"')
 if(!$providerMatch.Success){throw "Missing message provider: $($provider[0])"}
 $providerObject=Join-Path $tmp ($provider[0]+'-message-current.obj')
 $providerLog=Join-Path $tmp ($provider[0]+'-message-current.log')
 & $zig c++ @messageProviderCommon $provider[1] '-o' $providerObject *> $providerLog
 if($LASTEXITCODE){Get-Content -LiteralPath $providerLog -Tail 20;throw "Message provider compile failed: $($provider[0])"}
 $response=$response.Replace($providerMatch.Value,'"'+$providerObject+'"')
}
# BckCtrl retains converted BANMT bytes in its native header. The probe object
# can predate that conversion; rebuild it by default so later links cannot
# silently restore raw big-endian JMap reads. Explicit object overrides remain
# available for focused diagnostics.
if(!$BckCtrlObject){
 $BckCtrlObject=Join-Path $tmp 'BckCtrl-current.obj'
 $bckCtrlLog=Join-Path $tmp 'BckCtrl-current.log'
 & $zig c++ @messageProviderCommon (Join-Path $root 'Petari\src\Game\Animation\BckCtrl.cpp') '-o' $BckCtrlObject *> $bckCtrlLog
 if($LASTEXITCODE){Get-Content -LiteralPath $bckCtrlLog -Tail 20;throw 'Current native BckCtrl provider failed to compile.'}
}
if(!$ResourceAccessorObject){
 $ResourceAccessorObject=Join-Path $tmp 'lyt_resourceAccessor-current.obj'
 $resourceAccessorLog=Join-Path $tmp 'lyt_resourceAccessor-current.log'
 & $zig c++ @messageProviderCommon (Join-Path $root 'Petari\src\nw4r\lyt\lyt_resourceAccessor.cpp') '-o' $ResourceAccessorObject *> $resourceAccessorLog
 if($LASTEXITCODE){Get-Content -LiteralPath $resourceAccessorLog -Tail 20;throw 'Current layout ResourceAccessor provider failed to compile.'}
}
if($BckCtrlObject){
 if(!(Test-Path -LiteralPath $BckCtrlObject)){throw 'The supplied native BckCtrl object does not exist.'}
 $oldBckCtrl=[regex]::Match($response,'"[^"]*\\BckCtrl\.obj"')
 if(!$oldBckCtrl.Success){throw 'The response file does not contain the original BckCtrl provider.'}
 $response=$response.Replace($oldBckCtrl.Value,'"'+$BckCtrlObject+'"')
}
if($ResourceAccessorObject){
 if(!(Test-Path -LiteralPath $ResourceAccessorObject)){throw 'The supplied ResourceAccessor object does not exist.'}
 $oldAccessor=[regex]::Match($response,'"[^"]*\\lyt_resourceAccessor\.obj"')
 if(!$oldAccessor.Success){throw 'The response file does not contain the original lyt_resourceAccessor provider.'}
 $response=$response.Replace($oldAccessor.Value,'"'+$ResourceAccessorObject+'"')
}
# Trace the first particle lookups in diagnostic builds without changing emitter behavior.
foreach($particleUnit in @('JPAResourceLoader','JPAEmitterManager')){
 $particleOriginal=Join-Path $native ('src\generated\'+$particleUnit+'.cpp')
 $particleText=Get-Content -LiteralPath $particleOriginal -Raw
 $particleText="#include <cstdio>`nextern `"C`" void galaxyNativeBootTrace(const char*);`n"+$particleText
 if($particleUnit-eq 'JPAResourceLoader'){
  $anchor='    if (*(u32*)(data + 4) == ''2-10'') {'
  $trace='    char trace[192]; std::snprintf(trace, sizeof(trace), "JPA loader header=%c%c%c%c%c%c%c%c rawVersion=%08X resourcesBE=%u texturesBE=%u", data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7], *(const u32*)(data+4), unsigned(data[8])*256+data[9], unsigned(data[10])*256+data[11]); galaxyNativeBootTrace(trace);'
 }else{
  $anchor='    JPAResource* pRes = pResMgrAry[res_mgr_id]->getResource(resID);'
  $trace='    static unsigned traceCount=0; if(traceCount++<8) { char trace[192]; std::snprintf(trace,sizeof(trace),"JPA emitter request id=%u group=%u manager=%u resources=%u free=%u found=%d",unsigned(resID),unsigned(group_id),unsigned(res_mgr_id),unsigned(pResMgrAry[res_mgr_id]->mResNum),unsigned(mFreeEmtrList.getNumLinks()),pRes!=nullptr); galaxyNativeBootTrace(trace); }'
 }
 if(!$particleText.Contains($anchor)){throw "Particle trace anchor missing: $particleUnit"}
 $replacement=if($particleUnit-eq 'JPAResourceLoader'){$trace+"`n"+$anchor}else{$anchor+"`n"+$trace}
 $particleText=$particleText.Replace($anchor,$replacement)
 $particleSource=Join-Path $tmp ($particleUnit+'-startup-traced.cpp')
 $particleObject=Join-Path $tmp ($particleUnit+'-startup-traced.obj')
 [IO.File]::WriteAllText($particleSource,$particleText,[Text.UTF8Encoding]::new($false))
 & $zig c++ @gameSceneCommon $particleSource '-o' $particleObject
 if($LASTEXITCODE){throw "Particle trace compilation failed: $particleUnit"}
 $quotedOriginal='"'+$particleOriginal+'"'
 if(!$response.Contains($quotedOriginal)){throw "Particle source missing from link response: $particleUnit"}
 $response=$response.Replace($quotedOriginal,'"'+$particleObject+'"')
}
if($ParticleResourceHolderObject){
 if(!(Test-Path -LiteralPath $ParticleResourceHolderObject)){throw 'The supplied native ParticleResourceHolder object does not exist.'}
 $oldParticleHolder=[regex]::Match($response,'"[^"]*\\Petari\\src\\Game\\Effect\\ParticleResourceHolder\.cpp"')
 if(!$oldParticleHolder.Success){throw 'The response file does not contain the original ParticleResourceHolder source.'}
 $response=$response.Replace($oldParticleHolder.Value,'"'+$ParticleResourceHolderObject+'"')
}
if($TextureObjectLodKernelObject){
 if(!(Test-Path -LiteralPath $TextureObjectLodKernelObject)){throw 'The supplied native TextureObjectLodKernel object does not exist.'}
 $oldTextureKernel=[regex]::Match($response,'"[^"]*\\native\\src\\generated\\TextureObjectLodKernel\.cpp"')
 if(!$oldTextureKernel.Success){throw 'The response file does not contain the original TextureObjectLodKernel source.'}
 $response=$response.Replace($oldTextureKernel.Value,'"'+$TextureObjectLodKernelObject+'"')
}
if(!$response.Contains($oldStopSource)){throw 'The response file does not contain the original StopSceneController source.'}
$response=$response.Replace($oldStopSource,'"'+$stopSource+'"')
if(!$response.Contains($oldNameRegisterSource)){throw 'The response file does not contain the original NameObjRegister source.'}
$response=$response.Replace($oldNameRegisterSource,'"'+$nameRegisterSource+'"')
if(!$response.Contains($oldNameExecObject)){throw 'The response file does not contain the original NameObjExecuteHolder provider.'}
$response=$response.Replace($oldNameExecObject,'"'+$nameExecObject+'"')
$threadPortSource=Join-Path $tmp 'os_thread_port-exception-probe.cpp'
$threadPortText=Get-Content -LiteralPath (Join-Path $native 'src\os_thread_port.cpp') -Raw
$threadPortText='#include <cstdio>' + [Environment]::NewLine + '#include <exception>' + [Environment]::NewLine + 'extern "C" void galaxyNativeBootTrace(const char*);' + [Environment]::NewLine + $threadPortText
$threadCatchNeedle='catch(const NativeThreadCancelled&){}nativeThreadSuspensionPoint=nullptr;'
$threadCatchProbe='catch(const NativeThreadCancelled&){}catch(const std::exception& exception){galaxyNativeBootTrace(exception.what());}catch(...){galaxyNativeBootTrace("OSCreateThread worker caught non-standard exception");}nativeThreadSuspensionPoint=nullptr;'
if(!$threadPortText.Contains($threadCatchNeedle)){throw 'Could not locate the native OS-thread entry catch boundary.'}
$threadPortText=$threadPortText.Replace($threadCatchNeedle,$threadCatchProbe)
[IO.File]::WriteAllText($threadPortSource,$threadPortText,[Text.UTF8Encoding]::new($false))
$oldThreadPort='"'+$native+'\src\os_thread_port.cpp"'
if(!$response.Contains($oldThreadPort)){throw 'The response file does not contain the native OS-thread provider.'}
$response=$response.Replace($oldThreadPort,'"'+$threadPortSource+'"')
$gxSubmissionSource=Join-Path $native 'src\gx_submission_port.cpp'
$gxSubmissionTemp=Join-Path $tmp 'gx_submission_port-boot-traced.cpp'
$gxSubmissionObject=Join-Path $tmp 'gx_submission_port-boot-traced.obj'
$gxSubmissionText=Get-Content -LiteralPath $gxSubmissionSource -Raw
$gxSubmissionText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $gxSubmissionText
$gxSubmissionText=$gxSubmissionText.Replace('void flush(bool finish){','void flush(bool finish){' + "`n" + ' galaxyNativeBootTrace("GX flush provider entered");')
$gxSubmissionText=$gxSubmissionText.Replace('  batch.assign(writer->start,writer->ptr);','  galaxyNativeBootTrace("GX flush batch copy begin");' + "`n" + '  batch.assign(writer->start,writer->ptr);' + "`n" + '  galaxyNativeBootTrace("GX flush batch copy complete");')
$gxSubmissionText=$gxSubmissionText.Replace('  Native::notifyGxCommandObserver(writer->start,batch.size());','  galaxyNativeBootTrace("GX flush observer notify begin");' + "`n" + '  Native::notifyGxCommandObserver(writer->start,batch.size());' + "`n" + '  galaxyNativeBootTrace("GX flush observer notify returned");')
$gxSubmissionText=$gxSubmissionText.Replace('  writer->ptr=writer->start;','  galaxyNativeBootTrace("GX flush writer reset begin");' + "`n" + '  writer->ptr=writer->start;' + "`n" + '  galaxyNativeBootTrace("GX flush writer reset complete");')
$gxSubmissionText=$gxSubmissionText.Replace('  Native::notifyGxCommandObserver(nullptr,0,true);','  galaxyNativeBootTrace("GX flush observer reset begin");' + "`n" + '  Native::notifyGxCommandObserver(nullptr,0,true);' + "`n" + '  galaxyNativeBootTrace("GX flush observer reset complete");')
$gxSubmissionText=$gxSubmissionText.Replace(' try{backend->submit(batch.data(),batch.size(),finish);}catch(...){',' try{galaxyNativeBootTrace("GX flush backend submit begin");backend->submit(batch.data(),batch.size(),finish);galaxyNativeBootTrace("GX flush backend submit returned");}catch(...){')
[IO.File]::WriteAllText($gxSubmissionTemp,$gxSubmissionText,[Text.UTF8Encoding]::new($false))
& $zig c++ @heapCommon -c $gxSubmissionTemp '-o' $gxSubmissionObject
if($LASTEXITCODE){throw 'Boot-traced GX submission provider failed to compile.'}
$gxSubmissionQuoted='"'+$gxSubmissionSource+'"'
if(!$response.Contains($gxSubmissionQuoted)){throw 'The response file does not contain the native GX submission provider.'}
$response=$response.Replace($gxSubmissionQuoted,'"'+$gxSubmissionObject+'"')
$heapDispatchObject=Join-Path $tmp 'JKRHeapDispatch.obj'
$heapDispatchSource=Join-Path $tmp 'JKRHeapDispatch-boot-traced.cpp'
$heapDispatchText=Get-Content -LiteralPath (Join-Path $native 'src\generated\JKRHeapDispatch.cpp') -Raw
$heapDispatchText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $heapDispatchText
$heapDispatchText=$heapDispatchText.Replace('extern "C" void galaxyNativeBootTrace(const char*);', 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + 'static void galaxyNativeHeapIteratorTrace(const char* phase, const void* current, const void* next) { char line[200]; std::snprintf(line, sizeof(line), "JKRHeap iterator %s current=%p next=%p", phase, current, next); galaxyNativeBootTrace(line); }')
$heapDispatchText=$heapDispatchText.Replace('void* JKRHeap::alloc(u32 size, int align, JKRHeap* pHeap) {','void* JKRHeap::alloc(u32 size, int align, JKRHeap* pHeap) {' + "`n" + '    { char line[220]; std::snprintf(line,sizeof(line),"JKRHeap static alloc size=%u align=%d heap=%p current=%p",size,align,static_cast<void*>(pHeap),static_cast<void*>(sCurrentHeap)); galaxyNativeBootTrace(line); }')
$heapDispatchText=$heapDispatchText.Replace('void* JKRHeap::alloc(u32 size, int align) {','void* JKRHeap::alloc(u32 size, int align) {' + "`n" + '    { char line[220]; std::snprintf(line,sizeof(line),"JKRHeap instance alloc this=%p size=%u align=%d",static_cast<void*>(this),size,align); galaxyNativeBootTrace(line); }')
$findIterator='for (JSUTreeIterator< JKRHeap > iterator(mChildTree.getFirstChild()); iterator != mChildTree.getEndChild(); ++iterator) {'
$findIteratorTraced='for (JSUTreeIterator< JKRHeap > iterator(mChildTree.getFirstChild()); iterator != mChildTree.getEndChild(); galaxyNativeHeapIteratorTrace("advance begin", static_cast<const void*>(iterator.mTree), iterator.mTree ? static_cast<const void*>(iterator.mTree->getNextChild()) : nullptr), ++iterator, galaxyNativeHeapIteratorTrace("advance complete", static_cast<const void*>(iterator.mTree), iterator.mTree ? static_cast<const void*>(iterator.mTree->getNextChild()) : nullptr)) {'
if($heapDispatchText.Contains($findIterator)){$heapDispatchText=$heapDispatchText.Replace($findIterator,$findIteratorTraced)}
$findAllIterator='for (JSUTreeIterator< JKRHeap > iterator(mChildTree.getFirstChild()); iterator != mChildTree.getEndChild(); ++iterator) {'
if($heapDispatchText.Contains($findAllIterator)){$heapDispatchText=$heapDispatchText.Replace($findAllIterator,$findIteratorTraced)}
if(!$heapDispatchText.Contains('    _69 = false;')){throw 'JKRHeap constructor trace anchor missing.'}
$heapDispatchText=$heapDispatchText.Replace('    _69 = false;','    _69 = false;' + "`n" + '    char nativeHeapCtorTrace[220]; std::snprintf(nativeHeapCtorTrace, sizeof(nativeHeapCtorTrace), "JKRHeap ctor this=%p parent=%p start=%p end=%p childTree=%p childOwner=%p", static_cast<void*>(this), static_cast<void*>(parent), static_cast<void*>(mStart), static_cast<void*>(mEnd), static_cast<void*>(&mChildTree), static_cast<void*>(mChildTree.getObject())); galaxyNativeBootTrace(nativeHeapCtorTrace);')
$heapDtorSignature='JKRHeap::~JKRHeap() {'
if(!$heapDispatchText.Contains($heapDtorSignature)){throw 'JKRHeap destructor trace anchor missing.'}
$heapDispatchText=$heapDispatchText.Replace($heapDtorSignature,$heapDtorSignature + "`n" + '    char nativeHeapDtorTrace[180]; std::snprintf(nativeHeapDtorTrace, sizeof(nativeHeapDtorTrace), "JKRHeap dtor this=%p start=%p end=%p childTree=%p childOwner=%p", static_cast<void*>(this), static_cast<void*>(mStart), static_cast<void*>(mEnd), static_cast<void*>(&mChildTree), static_cast<void*>(mChildTree.getObject())); galaxyNativeBootTrace(nativeHeapDtorTrace);')
$heapFreeSignature='void JKRHeap::free(void* pData, JKRHeap* pHeap) {'
if(!$heapDispatchText.Contains($heapFreeSignature)){throw 'JKRHeap free dispatch trace anchor missing.'}
$heapDispatchText=$heapDispatchText.Replace($heapFreeSignature,$heapFreeSignature + "`n" + '    char nativeHeapFreeTrace[200]; std::snprintf(nativeHeapFreeTrace, sizeof(nativeHeapFreeTrace), "JKRHeap::free entered data=%p heap=%p root=%p current=%p", pData, static_cast<void*>(pHeap), static_cast<void*>(sRootHeap), static_cast<void*>(sCurrentHeap)); galaxyNativeBootTrace(nativeHeapFreeTrace);')
$heapDispatchText=$heapDispatchText.Replace('        pHeap = findFromRoot(pData);','        galaxyNativeBootTrace("JKRHeap::free findFromRoot begin");' + "`n" + '        pHeap = findFromRoot(pData);' + "`n" + '        galaxyNativeBootTrace(pHeap ? "JKRHeap::free owning heap found" : "JKRHeap::free owning heap missing");')
$heapDispatchText=$heapDispatchText.Replace('    pHeap->do_free(pData);','    galaxyNativeBootTrace("JKRHeap::free do_free begin");' + "`n" + '    pHeap->do_free(pData);' + "`n" + '    galaxyNativeBootTrace("JKRHeap::free do_free returned");')
$heapDispatchText=$heapDispatchText.Replace('JKRHeap* JKRHeap::findFromRoot(void* pData) {','JKRHeap* JKRHeap::findFromRoot(void* pData) {' + "`n" + '    galaxyNativeBootTrace("JKRHeap::findFromRoot entered");')
$heapDispatchText=$heapDispatchText.Replace('    JKRHeap* root = sRootHeap;','    JKRHeap* root = sRootHeap; char rootRangeTrace[200]; std::snprintf(rootRangeTrace, sizeof(rootRangeTrace), "findFromRoot root=%p start=%p end=%p data=%p", static_cast<void*>(root), root ? reinterpret_cast<void*>(root->mStart) : nullptr, root ? reinterpret_cast<void*>(root->mEnd) : nullptr, pData); galaxyNativeBootTrace(rootRangeTrace);')
$heapDispatchText=$heapDispatchText.Replace('    if ((void*)root->mStart <= pData && pData < (void*)root->mEnd) {','    galaxyNativeBootTrace("findFromRoot root range compare begin");' + "`n" + '    if ((void*)root->mStart <= pData && pData < (void*)root->mEnd) {' + "`n" + '        galaxyNativeBootTrace("findFromRoot data inside root range");')
$heapDispatchText=$heapDispatchText.Replace('        return root->find(pData);','        galaxyNativeBootTrace("findFromRoot root find begin");' + "`n" + '        return root->find(pData);')
$heapDispatchText=$heapDispatchText.Replace('    return root->findAllHeap(pData);','    galaxyNativeBootTrace("findFromRoot recursive find-all begin");' + "`n" + '    return root->findAllHeap(pData);')
$heapDispatchText=$heapDispatchText.Replace('JKRHeap* JKRHeap::find(void* pData) const {','JKRHeap* JKRHeap::find(void* pData) const {' + "`n" + '    galaxyNativeBootTrace("JKRHeap::find entered");')
$heapDispatchText=$heapDispatchText.Replace('    if (mStart <= pData && pData < mEnd) {','    char heapRangeTrace[200]; std::snprintf(heapRangeTrace, sizeof(heapRangeTrace), "JKRHeap::find self=%p start=%p end=%p data=%p", static_cast<const void*>(this), reinterpret_cast<void*>(mStart), reinterpret_cast<void*>(mEnd), pData); galaxyNativeBootTrace(heapRangeTrace);' + "`n" + '    if (mStart <= pData && pData < mEnd) {')
$heapDispatchText=$heapDispatchText.Replace('        const JSUTree< JKRHeap >& tree = mChildTree;','        galaxyNativeBootTrace("JKRHeap::find address inside heap");' + "`n" + '        const JSUTree< JKRHeap >& tree = mChildTree;' + "`n" + '        galaxyNativeBootTrace("JKRHeap::find child tree read");')
$heapDispatchText=$heapDispatchText.Replace('                JKRHeap* result = iterator->find(pData);','                JKRHeap* childHeap = iterator.getObject(); char childHeapTrace[220]; auto* childNode = iterator.mTree; std::snprintf(childHeapTrace, sizeof(childHeapTrace), "JKRHeap child node=%p object=%p target=%p next=%p prev=%p parent=%p", static_cast<void*>(childNode), static_cast<void*>(childHeap), pData, static_cast<void*>(childNode->getNextChild()), static_cast<void*>(childNode->getPrevChild()), static_cast<void*>(childNode->getParent())); galaxyNativeBootTrace(childHeapTrace);' + "`n" + '                JKRHeap* result = childHeap->find(pData);' + "`n" + '                galaxyNativeBootTrace("JKRHeap::find child invocation returned");')
[IO.File]::WriteAllText($heapDispatchSource,$heapDispatchText,[Text.UTF8Encoding]::new($false))
& $zig c++ @heapCommon -c $heapDispatchSource '-o' $heapDispatchObject
if($LASTEXITCODE){throw 'Boot-traced JKRHeap dispatch provider failed to compile.'}
$expHeapSourceOriginal=Join-Path $native 'src\generated\JKRExpHeap.cpp'
$expHeapSource=Join-Path $tmp 'JKRExpHeap-allocation-traced.cpp'
$expHeapText=Get-Content -LiteralPath $expHeapSourceOriginal -Raw
$expHeapText='#include <cstdio>' + "`n" + 'extern "C" void galaxyNativeBootTrace(const char*);' + "`n" + $expHeapText
$expHeapText=$expHeapText.Replace('void* JKRExpHeap::do_alloc(u32 size, int align) {','void* JKRExpHeap::do_alloc(u32 size, int align) {' + "`n" + '    { char line[220]; std::snprintf(line,sizeof(line),"JKRExpHeap do_alloc entered heap=%p size=%u align=%d start=%p end=%p",static_cast<void*>(this),size,align,reinterpret_cast<void*>(mStart),reinterpret_cast<void*>(mEnd)); galaxyNativeBootTrace(line); }')
$expHeapText=$expHeapText.Replace('    OSLockMutex(&mMutex);','    galaxyNativeBootTrace("JKRExpHeap do_alloc lock begin");' + "`n" + '    OSLockMutex(&mMutex);' + "`n" + '    galaxyNativeBootTrace("JKRExpHeap do_alloc lock returned");')
$expHeapText=$expHeapText.Replace('    OSUnlockMutex(&mMutex);' + "`n" + '    return ptr;','    OSUnlockMutex(&mMutex);' + "`n" + '    { char line[180]; std::snprintf(line,sizeof(line),"JKRExpHeap do_alloc unlocked result=%p",ptr); galaxyNativeBootTrace(line); }' + "`n" + '    return ptr;')
[IO.File]::WriteAllText($expHeapSource,$expHeapText,[Text.UTF8Encoding]::new($false))
$expHeapObject=Join-Path $tmp 'JKRExpHeap-allocation-traced.obj'
& $zig c++ @heapCommon -c $expHeapSource '-o' $expHeapObject
if($LASTEXITCODE){throw 'Boot-traced JKRExpHeap allocation provider failed to compile.'}
$oldHeapDispatch='"'+$native+'\build\actor-runtime-probe\JKRHeapDispatch.obj"'
if(!$response.Contains($oldHeapDispatch)){throw 'The response file does not contain the original JKRHeapDispatch provider.'}
$response=$response.Replace($oldHeapDispatch,'"'+$heapDispatchObject+'"')
$oldExpHeap='"'+$native+'\build\actor-runtime-probe\JKRExpHeap.obj"'
if(!$response.Contains($oldExpHeap)){throw 'The response file does not contain the original JKRExpHeap provider.'}
$response=$response.Replace($oldExpHeap,'"'+$expHeapObject+'"')
$oldEntry='"'+$native+'\build\native-game-system-entry-audit\GameSystemEntry.obj"'
if(!$response.Contains($oldEntry)){throw 'The response file does not contain the standalone game entry provider.'}
$response=$response.Replace($oldEntry,'"'+$entryObject+'"')
$oldHeap='"'+$native+'\build\actor-runtime-probe\HeapMemoryWatcher.obj"'
if(!$response.Contains($oldHeap)){throw 'The response file does not contain the original HeapMemoryWatcher provider.'}
$response=$response.Replace($oldHeap,'"'+$heapObject+'"')
$oldGameSystem='"'+$native+'\build\actor-runtime-probe\GameSystem.obj"'
if(!$response.Contains($oldGameSystem)){throw 'The response file does not contain the original GameSystem provider.'}
$response=$response.Replace($oldGameSystem,'"'+$gameSystemObject+'"')
$mainLoopMatch=[regex]::Match($response,'"([^"\r\n]*MainLoopFramework\.obj)"')
if(!$mainLoopMatch.Success){throw 'The response file does not contain the original MainLoopFramework provider.'}
$oldMainLoop=$mainLoopMatch.Groups[0].Value
$response=$response.Replace($oldMainLoop,'"'+$mainLoopObject+'"')
$oldObjHolder='"'+$native+'\build\actor-runtime-probe\GameSystemObjHolder.obj"'
if(!$response.Contains($oldObjHolder)){throw 'The response file does not contain the original GameSystemObjHolder provider.'}
$response=$response.Replace($oldObjHolder,'"'+$objHolderObject+'"')
$oldAsync='"'+$native+'\build\actor-runtime-probe\FunctionAsyncExecutor.obj"'
if(!$response.Contains($oldAsync)){throw 'The response file does not contain the original FunctionAsyncExecutor provider.'}
$response=$response.Replace($oldAsync,'"'+$asyncObject+'"')
$stationedLoaderMatch=[regex]::Match($response,'"[^"]*\\GameSystemStationedArchiveLoader\.obj"')
if(!$stationedLoaderMatch.Success){throw 'The response file does not contain the original GameSystemStationedArchiveLoader provider.'}
$response=$response.Replace($stationedLoaderMatch.Value,'"'+$stationedLoaderObject+'"')
$resourceHolderMatch=[regex]::Match($response,'"[^"]*\\ResourceHolder\.obj"')
if(!$resourceHolderMatch.Success){throw 'The response file does not contain the original ResourceHolder provider.'}
$response=$response.Replace($resourceHolderMatch.Value,'"'+$resourceHolderObject+'"')
$nativeBckMatch=[regex]::Match($response,'"[^"\r\n]*native_bck_animation\.cpp"')
if(!$nativeBckMatch.Success){throw 'The response file does not contain the native animation decoder source.'}
$response=$response.Replace($nativeBckMatch.Value,'"'+$nativeBckTraceObject+'"')
$materialAnmMatch=[regex]::Match($response,'"[^"\r\n]*\\MaterialAnmBuffer\.obj"')
if(!$materialAnmMatch.Success){throw 'The response file does not contain the original MaterialAnmBuffer provider.'}
$response=$response.Replace($materialAnmMatch.Value,'"'+$materialAnmObject+'"')
$oldSceneController='"'+$native+'\build\actor-runtime-probe\GameSystemSceneController.obj"'
if(!$response.Contains($oldSceneController)){throw 'The response file does not contain the original GameSystemSceneController provider.'}
$response=$response.Replace($oldSceneController,'"'+$sceneControllerObject+'"')
$oldLogo='"'+$native+'\build\actor-runtime-probe\LogoScene.obj"'
if(!$response.Contains($oldLogo)){throw 'The response file does not contain the original LogoScene provider.'}
$response=$response.Replace($oldLogo,'"'+$logoObject+'"')
$oldSceneFunction='"'+$native+'\build\actor-runtime-probe\SceneFunction.obj"'
if(!$response.Contains($oldSceneFunction)){throw 'The response file does not contain the original SceneFunction provider.'}
$response=$response.Replace($oldSceneFunction,'"'+$sceneFunctionObject+'"')
$oldNativeCollision='"'+$native+'\src\native_collision_scene.cpp"'
if(!$response.Contains($oldNativeCollision)){throw 'The response file does not contain the native collision-scene provider.'}
$response=$response.Replace($oldNativeCollision,'"'+$collisionSource+'"')
$oldMiiIcon='"'+(Join-Path $native 'build\actor-runtime-probe\MiiFaceIcon.obj')+'"'
if(-not $response.Contains($oldMiiIcon)){throw 'Original Mii icon provider missing from response.'}
$response=$response.Replace($oldMiiIcon,'"'+$miiFaceIconObject+'"')
foreach($provider in $cameraReaderProviders){
 $oldCamera='"'+(Join-Path $native ('build\actor-runtime-probe\'+$provider.Unit+'.obj'))+'"'
 if(-not $response.Contains($oldCamera)){throw "Original camera table provider missing: $($provider.Unit)"}
 $response=$response.Replace($oldCamera,'"'+$provider.Path+'"')
}
$oldFontFileUtil='"'+(Join-Path $native 'build\actor-runtime-probe\FileUtil.obj')+'"'
if(-not $response.Contains($oldFontFileUtil)){throw 'Original FileUtil provider missing from response.'}
$response=$response.Replace($oldFontFileUtil,'"'+$fontFileUtilObject+'"')
# The original Mii scene holder's class definition includes RVLFaceLib.h.
$response+=' "-I'+(Join-Path $native 'build\rfl-original\include')+'" "-I'+(Join-Path $root 'Petari\libs\RVLFaceLib\include')+'"'
foreach($unit in @('mem_expHeap','mem_heapCommon','mem_list')){
 $oldMem='"'+(Join-Path $native ('build\rfl-original\'+$unit+'.obj'))+'"'
 $newMem='"'+(Join-Path $native ('build\sdk-mem-host\'+$unit+'.obj'))+'"'
 if(-not $response.Contains($oldMem)){throw "Original SDK MEM provider missing from response: $unit"}
 $response=$response.Replace($oldMem,$newMem)
}
$oldAudio='"'+$native+'\build\actor-runtime-probe\AudSystemWrapper.obj"'
if(!$response.Contains($oldAudio)){throw 'The response file does not contain the original AudSystemWrapper provider.'}
$response=$response.Replace($oldAudio,'"'+$audioObject+'"')
$oldMic='"'+$native+'\build\actor-runtime-probe\AudMicWrap.obj"'
if(!$response.Contains($oldMic)){throw 'The response file does not contain the original AudMicWrap provider.'}
$response=$response.Replace($oldMic,'"'+$micObject+'"')
$oldAudKernel='"'+$native+'\src\generated\AudioSystemKernel.cpp"'
if(!$response.Contains($oldAudKernel)){throw 'The response file does not contain the original generated AudSystem provider.'}
$response=$response.Replace($oldAudKernel,'"'+$audKernelSource+'"')
$oldSectionHeap='"'+$native+'\src\generated\AudioSectionHeapKernel.cpp"'
if(!$response.Contains($oldSectionHeap)){throw 'The response file does not contain the generated JAUSectionHeap provider.'}
$response=$response.Replace($oldSectionHeap,'"'+$sectionHeapSource+'"')
$oldRemixManager='"'+$native+'\build\actor-runtime-probe\AudRemixMgr.obj"'
if(!$response.Contains($oldRemixManager)){throw 'The response file does not contain the original Remix manager provider.'}
$response=$response.Replace($oldRemixManager,'"'+$remixManagerObject+'"')
$oldJau='"'+$native+'\build\actor-runtime-probe\JAUInitializer.obj"'
if(!$response.Contains($oldJau)){throw 'The response file does not contain the original JAUInitializer provider.'}
$response=$response.Replace($oldJau,'"'+$jauObject+'"')
$oldAudioThread='"'+$native+'\src\generated\AudioThreadKernel.cpp"'
if(!$response.Contains($oldAudioThread)){throw 'The response file does not contain the original native audio-thread provider.'}
$response=$response.Replace($oldAudioThread,'"'+$audioThreadSource+'"')
$oldHostDsp='"'+$native+'\build\actor-runtime-probe\JASDSPInterface.obj"'
if(!$response.Contains($oldHostDsp)){throw 'The response file does not contain the original host DSP interface provider.'}
$response=$response.Replace($oldHostDsp,'"'+$hostDspObject+'"')
$oldDecomp='"'+$native+'\build\actor-runtime-probe\JKRDecomp.obj"'
if(!$response.Contains($oldDecomp)){throw 'The response file does not contain the original JKRDecomp provider.'}
$response=$response.Replace($oldDecomp,'"'+$decompObject+'"')
$oldFileRipper='"'+$native+'\build\actor-runtime-probe\FileRipper.obj"'
if(!$response.Contains($oldFileRipper)){throw 'The response file does not contain the original FileRipper provider.'}
$response=$response.Replace($oldFileRipper,'"'+$fileRipperObject+'"')
$oldDisplayCopy='"'+$native+'\src\gx_display_copy_port.cpp"'
if(!$response.Contains($oldDisplayCopy)){throw 'The response file does not contain the native GX display-copy observer provider.'}
$response=$response.Replace($oldDisplayCopy,'"'+$displayCopyObject+'"')
$oldResourceManager='"'+$native+'\build\actor-runtime-probe\ResourceHolderManager.obj"'
if(!$response.Contains($oldResourceManager)){throw 'The response file does not contain the original ResourceHolderManager provider.'}
$response=$response.Replace($oldResourceManager,'"'+$resourceManagerObject+'"')
$oldLayoutHolder='"'+$native+'\build\actor-runtime-probe\LayoutHolder.obj"'
if(!$response.Contains($oldLayoutHolder)){throw 'The response file does not contain the original LayoutHolder provider.'}
$response=$response.Replace($oldLayoutHolder,'"'+$layoutHolderObject+'"')
$oldArchivePub='"'+$native+'\build\actor-runtime-probe\JKRArchivePub.obj"'
if(!$response.Contains($oldArchivePub)){throw 'The response file does not contain the original JKRArchivePub provider.'}
$response=$response.Replace($oldArchivePub,'"'+$archivePubObject+'"')
$oldArchiveHolder='"'+$native+'\build\actor-runtime-probe\ArchiveHolder.obj"'
$oldStageDataHolder='"'+$native+'\build\actor-runtime-probe\StageDataHolder.obj"'
if(!$response.Contains($oldStageDataHolder)){throw 'The response file does not contain the cached StageDataHolder provider.'}
$response=$response.Replace($oldStageDataHolder,'"'+$stageDataHolderObject+'"')
$oldNameObjAdaptor='"'+$native+'\build\actor-runtime-probe\NameObjAdaptor.obj"'
if(!$response.Contains($oldNameObjAdaptor)){throw 'The response file does not contain the cached NameObjAdaptor provider.'}
$response=$response.Replace($oldNameObjAdaptor,'"'+$nameObjAdaptorObject+'"')
$oldNameObjCategoryList='"'+$native+'\build\actor-runtime-probe\NameObjCategoryList.obj"'
if(!$response.Contains($oldNameObjCategoryList)){throw 'The response file does not contain the cached NameObjCategoryList provider.'}
$response=$response.Replace($oldNameObjCategoryList,'"'+$nameObjCategoryListObject+'"')
foreach($provider in $callbackProducers){
 $oldProvider='"'+$provider.OriginalObject+'"'
 if(!$response.Contains($oldProvider)){throw "Missing cached callback producer: $($provider.OriginalObject)"}
 $response=$response.Replace($oldProvider,'"'+$provider.Path+'"')
}
foreach($provider in $nativeSaveProviders){
 $oldProvider='"'+$provider.OriginalObject+'"'
 if([regex]::Matches($response,[regex]::Escape($oldProvider)).Count-ne 1){throw "Expected exactly one original save provider: $($provider.Unit)"}
 $response=$response.Replace($oldProvider,'"'+$provider.Path+'"')
}
# Keep the complete cached callback producer closure on the same interface as
# the refreshed clone consumers. The manifest records exact original paths.
$callbackManifest=Join-Path $native 'build\startup-verification-20261014\remaining-cached-functor-providers.txt'
$callbackRefreshRoot=Join-Path $native 'build\actor-runtime-callback-abi-20261014'
$callbackHeaderTime=(Get-Item -LiteralPath (Join-Path $native 'include\Game\Util\Functor.hpp')).LastWriteTimeUtc
if(!(Test-Path -LiteralPath $callbackManifest)){throw 'Missing callback ABI provider manifest.'}
foreach($originalCallbackObject in (Get-Content -LiteralPath $callbackManifest)){
 $unit=[IO.Path]::GetFileNameWithoutExtension($originalCallbackObject)
 $refreshedObject=Join-Path $callbackRefreshRoot ($unit+'.obj')
 if(!(Test-Path -LiteralPath $refreshedObject)){throw "Rebuild the callback ABI provider set; missing $unit"}
 if((Get-Item -LiteralPath $refreshedObject).LastWriteTimeUtc-lt $callbackHeaderTime){throw "Rebuild callback provider $unit after the callback header change."}
 $quotedOriginal='"'+$originalCallbackObject+'"'
 if(!$response.Contains($quotedOriginal)){throw "Callback manifest provider is absent from link response: $unit"}
 $response=$response.Replace($quotedOriginal,'"'+$refreshedObject+'"')
}
$oldFileSelectorFont='"'+(Join-Path $callbackRefreshRoot 'FileSelector.obj')+'"'
if(-not $response.Contains($oldFileSelectorFont)){throw 'Current callback-ABI FileSelector provider missing from response.'}
$response=$response.Replace($oldFileSelectorFont,'"'+$fileSelectorFontObject+'"')
$gdWriterManifest=Join-Path $native 'build\startup-verification-20261014\gd-writer-cached-providers.txt'
$gdWriterRoot=Join-Path $native 'build\actor-runtime-gd-writer-tls-20261014'
if(!(Test-Path -LiteralPath $gdWriterManifest)){throw 'Missing GD writer provider manifest.'}
foreach($originalWriterObject in (Get-Content -LiteralPath $gdWriterManifest)){
 $unit=[IO.Path]::GetFileNameWithoutExtension($originalWriterObject)
 $refreshedObject=Join-Path $gdWriterRoot ($unit+'.obj')
 if(!(Test-Path -LiteralPath $refreshedObject)){throw "Rebuild the thread-local GD provider set; missing $unit"}
 $quotedOriginal='"'+$originalWriterObject+'"'
 if(!$response.Contains($quotedOriginal)){throw "GD writer manifest provider is absent from link response: $unit"}
 $response=$response.Replace($quotedOriginal,'"'+$refreshedObject+'"')
}
& (Join-Path $native 'generate-j3d-packet.ps1')
$packetWriterObject=Join-Path $tmp 'J3DPacket-restored-writer.obj'
& $zig c++ @gameSceneCommon (Join-Path $native 'src\generated\J3DPacket.cpp') '-o' $packetWriterObject
if($LASTEXITCODE){throw 'Restoring J3D packet writer provider failed.'}
$packetMatches=[regex]::Matches($response,'"[^"\r\n]*\\J3DPacket\.obj"')
if($packetMatches.Count-ne 1){throw 'Expected exactly one J3D packet provider.'}
$response=$response.Replace($packetMatches[0].Value,'"'+$packetWriterObject+'"')
$shadowWriterObject=Join-Path $tmp 'MarioShadow-restored-writer.obj'
& $zig c++ @gameSceneCommon (Join-Path $native 'src\generated\MarioShadow.cpp') '-o' $shadowWriterObject
if($LASTEXITCODE){throw 'Restoring collision shadow writer provider failed.'}
$shadowMatches=[regex]::Matches($response,'"[^"\r\n]*\\MarioShadow\.obj"')
if($shadowMatches.Count-ne 1){throw 'Expected exactly one MarioShadow provider.'}
$response=$response.Replace($shadowMatches[0].Value,'"'+$shadowWriterObject+'"')
# Record the actual writer switches in player private-list builders.
foreach($unit in @('MarioActorDraw','MarioActorSpecialDraw','J3DModelX')){
 $writerTraceText=Get-Content -LiteralPath (Join-Path $native ('src\generated\'+$unit+'.cpp')) -Raw
 $writerTraceText='#include <cstdio>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$writerTraceText
 $pattern='GDSetCurrent\(&[A-Za-z0-9_]+\);|__GDCurrentDL\s*=\s*&[A-Za-z0-9_]+;'
 if(![regex]::IsMatch($writerTraceText,$pattern)){throw "No player writer switches found: $unit"}
 $writerTraceText=[regex]::Replace($writerTraceText,$pattern,{param($m) '{ char line[200]; std::snprintf(line,sizeof(line),"Player writer switch %s before=%p",__func__,static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }'+"`n"+$m.Value+"`n"+'{ char line[200]; std::snprintf(line,sizeof(line),"Player writer switch %s after=%p",__func__,static_cast<void*>(__GDCurrentDL)); galaxyNativeBootTrace(line); }'})
 $writerTraceSource=Join-Path $tmp ($unit+'-writer-switches.cpp')
 $writerTraceObject=Join-Path $tmp ($unit+'-writer-switches.obj')
 [IO.File]::WriteAllText($writerTraceSource,$writerTraceText)
 # Relative includes in these generated sources resolve from their source directory.
 & $zig c++ @gameSceneCommon ('-I'+(Join-Path $native 'src\generated')) $writerTraceSource '-o' $writerTraceObject
 if($LASTEXITCODE){throw "Player writer trace provider failed: $unit"}
 $writerMatches=[regex]::Matches($response,'"[^"\r\n]*\\'+$unit+'\.obj"')
 if($writerMatches.Count-ne 1){throw "Expected one player writer provider: $unit"}
 $response=$response.Replace($writerMatches[0].Value,'"'+$writerTraceObject+'"')
}
$shapeArrayText=Get-Content -LiteralPath (Join-Path $native 'src\generated\J3DShape.cpp') -Raw
$shapeArrayText='#include <cstdio>'+"`n"+'#include <JSystem/J3DGraphAnimator/J3DModel.hpp>'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+$shapeArrayText
foreach($attr in @('GX_VA_POS','GX_VA_NRM','GX_VA_CLR0')){
 $expr=switch($attr){'GX_VA_POS'{'j3dSys.getVtxPos()'}'GX_VA_NRM'{'j3dSys.getVtxNrm()'}'GX_VA_CLR0'{'j3dSys.getVtxCol()'}}
 $call='J3DLoadArrayBasePtr('+$attr+', '+$expr+');'
 if(!$shapeArrayText.Contains($call)){throw "Shape array diagnostic anchor missing: $attr"}
 $diagnostic='try{'+$call+'}catch(...){auto* model=j3dSys.getModel();auto* data=model?model->mModelData:nullptr;char line[640];std::snprintf(line,sizeof(line),"Vertex array failure attr='+$attr+' shape=%u model=%p data=%p current-pos=%p current-nrm=%p current-col=%p source-pos=%p source-nrm=%p source-col=%p vertices=%u normals=%u",unsigned(mIndex),static_cast<void*>(model),static_cast<void*>(data),j3dSys.getVtxPos(),j3dSys.getVtxNrm(),j3dSys.getVtxCol(),data?data->getVertexData().mVtxPosArray:nullptr,data?data->getVertexData().mVtxNrmArray:nullptr,data?data->getVertexData().mVtxColorArray[0]:nullptr,data?unsigned(data->getVtxNum()):0,data?unsigned(data->getNrmNum()):0);galaxyNativeBootTrace(line);throw;}'
 $shapeArrayText=$shapeArrayText.Replace($call,$diagnostic)
}
$shapeArraySource=Join-Path $tmp 'J3DShape-array-traced.cpp'
$shapeArrayObject=Join-Path $tmp 'J3DShape-array-traced.obj'
[IO.File]::WriteAllText($shapeArraySource,$shapeArrayText)
& $zig c++ @gameSceneCommon $shapeArraySource '-o' $shapeArrayObject
if($LASTEXITCODE){throw 'Shape array diagnostic provider failed.'}
$shapeArrayMatches=[regex]::Matches($response,'"[^"\r\n]*\\J3DShape\.obj"')
if($shapeArrayMatches.Count-ne 1){throw 'Expected exactly one J3DShape provider.'}
$response=$response.Replace($shapeArrayMatches[0].Value,'"'+$shapeArrayObject+'"')
$matrixStorageObject=Join-Path $tmp 'J3DMtxBuffer-native-storage.obj'
& $zig c++ @gameSceneCommon (Join-Path $native 'src\generated\J3DMtxBuffer.cpp') '-o' $matrixStorageObject
if($LASTEXITCODE){throw 'Native matrix storage provider failed.'}
$matrixStorageMatches=[regex]::Matches($response,'"[^"\r\n]*\\J3DMtxBuffer\.obj"')
if($matrixStorageMatches.Count-ne 1){throw 'Expected exactly one matrix buffer provider.'}
$response=$response.Replace($matrixStorageMatches[0].Value,'"'+$matrixStorageObject+'"')
# Only two cached providers in this link directly include the complete emitter
# layout. Other JPA/effect providers are compiled from source in this build;
# the ABI define above invalidates their old compiler-cache configuration.
foreach($jpaProvider in @(
 @{Unit='SpinPullParticleCallBack';Source=(Join-Path $root 'Petari\src\Game\Effect\SpinPullParticleCallBack.cpp')},
 @{Unit='MarioEffect';Source=(Join-Path $native 'src\generated\MarioEffect.cpp')}
)){
 $jpaObject=Join-Path $tmp ($jpaProvider.Unit+'-native-emitter-layout.obj')
 & $zig c++ @gameSceneCommon $jpaProvider.Source '-o' $jpaObject
 if($LASTEXITCODE){throw "Native emitter-layout provider failed: $($jpaProvider.Unit)"}
 $jpaMatches=[regex]::Matches($response,'"[^"\r\n]*\\'+$jpaProvider.Unit+'\.obj"')
 if($jpaMatches.Count-ne 1){throw "Expected exactly one cached emitter-layout provider: $($jpaProvider.Unit)"}
 $response=$response.Replace($jpaMatches[0].Value,'"'+$jpaObject+'"')
}
$oldDisplayMaker='"'+$native+'\build\actor-runtime-probe\DisplayListMaker.obj"'
foreach($provider in $texturePointerProviders){
 $matches=[regex]::Matches($response,'"[^"]*\\'+$provider.Unit+'\.obj"')
 if($matches.Count-ne 1){throw "Expected one texture-pointer provider in link response: $($provider.Unit)"}
 $response=$response.Replace($matches[0].Value,'"'+$provider.Path+'"')
}
if(!$response.Contains($oldDisplayMaker)){throw 'Missing cached display-list maker provider'}
$response=$response.Replace($oldDisplayMaker,'"'+$displayMakerObject+'"')
foreach($provider in $prologueRuntimeProviders){
 $matches=[regex]::Matches($response,'"[^"]*\\'+$provider.Unit+'\.obj"')
 if($matches.Count-ne 1){throw "Expected one cached prologue provider: $($provider.Unit)"}
 $response=$response.Replace($matches[0].Value,'"'+$provider.Path+'"')
}
$oldGameScene='"'+$native+'\build\actor-runtime-probe\GameScene.obj"'
if(!$response.Contains($oldGameScene)){throw 'The response file does not contain the original GameScene provider.'}
$response=$response.Replace($oldGameScene,'"'+$gameSceneObject+'"')
$oldFileLoaderRelease='"'+$native+'\build\actor-runtime-probe\JKRFileLoader.obj"'
if(!$response.Contains($oldFileLoaderRelease)){throw 'The response file does not contain the original JKRFileLoader provider.'}
$response=$response.Replace($oldFileLoaderRelease,'"'+$fileLoaderReleaseObject+'"')
if(!$response.Contains($oldArchiveHolder)){throw 'The response file does not contain the original ArchiveHolder provider.'}
$response=$response.Replace($oldArchiveHolder,'"'+$archiveHolderObject+'"')
$oldErrorWatcher='"'+$native+'\build\actor-runtime-probe\GameSystemErrorWatcher.obj"'
if(!$response.Contains($oldErrorWatcher)){throw 'The response file does not contain the original GameSystemErrorWatcher provider.'}
$response=$response.Replace($oldErrorWatcher,'"'+$errorWatcherObject+'"')
$oldErrorWindow='"'+$native+'\build\actor-runtime-probe\ErrorMessageWindow.obj"'
if(!$response.Contains($oldErrorWindow)){throw 'The response file does not contain the original ErrorMessageWindow provider.'}
$response=$response.Replace($oldErrorWindow,'"'+$errorWindowObject+'"')
$oldLayoutManager='"'+$native+'\build\actor-runtime-probe\LayoutManager.obj"'
if(!$response.Contains($oldLayoutManager)){throw 'The response file does not contain the original LayoutManager provider.'}
$response=$response.Replace($oldLayoutManager,'"'+$layoutManagerObject+'"')
$oldLytLayout='"'+$native+'\build\actor-runtime-probe\lyt_layout.obj"'
if(!$response.Contains($oldLytLayout)){throw 'The response file does not contain the original NW4R lyt_layout provider.'}
$response=$response.Replace($oldLytLayout,'"'+$lytLayoutObject+'"')
$oldLytCommon='"'+$native+'\build\actor-runtime-probe\lyt_common.obj"'
if(!$response.Contains($oldLytCommon)){throw 'The response file does not contain the original NW4R lyt_common provider.'}
$response=$response.Replace($oldLytCommon,'"'+$lytCommonObject+'"')
$oldLytPane='"'+$native+'\build\actor-runtime-probe\lyt_pane.obj"'
if(!$response.Contains($oldLytPane)){throw 'The response file does not contain the original NW4R lyt_pane provider.'}
$response=$response.Replace($oldLytPane,'"'+$lytPaneObject+'"')
$oldLytWindow='"'+$native+'\build\actor-runtime-probe\lyt_window.obj"'
if(!$response.Contains($oldLytWindow)){throw 'The response file does not contain the original NW4R lyt_window provider.'}
$response=$response.Replace($oldLytWindow,'"'+$lytWindowObject+'"')
$oldLytPicture='"'+$native+'\build\actor-runtime-probe\lyt_picture.obj"'
if(!$response.Contains($oldLytPicture)){throw 'The response file does not contain the original NW4R lyt_picture provider.'}
$response=$response.Replace($oldLytPicture,'"'+$lytPictureObject+'"')
$oldLytMaterial='"'+$native+'\build\actor-runtime-probe\lyt_material.obj"'
if(!$response.Contains($oldLytMaterial)){throw 'The response file does not contain the original NW4R lyt_material provider.'}
$response=$response.Replace($oldLytMaterial,'"'+$lytMaterialObject+'"')
$oldLytTexMap='"'+$native+'\build\actor-runtime-probe\lyt_texMap.obj"'
if(!$response.Contains($oldLytTexMap)){throw 'The response file does not contain the original NW4R lyt_texMap provider.'}
$response=$response.Replace($oldLytTexMap,'"'+$lytTexMapObject+'"')
$oldLytAnimation='"'+$native+'\build\actor-runtime-probe\lyt_animation.obj"'
if(!$response.Contains($oldLytAnimation)){throw 'The response file does not contain the original NW4R lyt_animation provider.'}
$response=$response.Replace($oldLytAnimation,'"'+$lytAnimationObject+'"')
$oldLayoutAnmPlayer='"'+$native+'\build\actor-runtime-probe\LayoutAnmPlayer.obj"'
if(!$response.Contains($oldLayoutAnmPlayer)){throw 'The response file does not contain the original LayoutAnmPlayer provider.'}
$response=$response.Replace($oldLayoutAnmPlayer,'"'+$layoutAnmPlayerObject+'"')
$oldLytTextBox='"'+$native+'\build\actor-runtime-probe\lyt_textBox.obj"'
if(!$response.Contains($oldLytTextBox)){throw 'The response file does not contain the original NW4R lyt_textBox provider.'}
$response=$response.Replace($oldLytTextBox,'"'+$lytTextBoxObject+'"')
$oldLayoutCoreUtil='"'+$native+'\build\actor-runtime-probe\LayoutCoreUtil.obj"'
if(!$response.Contains($oldLayoutCoreUtil)){throw 'The response file does not contain the original LayoutCoreUtil provider.'}
$response=$response.Replace($oldLayoutCoreUtil,'"'+$layoutCoreUtilObject+'"')
$oldMemoryUtil='"'+$native+'\build\actor-runtime-probe\MemoryUtil.obj"'
if(!$response.Contains($oldMemoryUtil)){throw 'The response file does not contain the original MemoryUtil provider.'}
$response=$response.Replace($oldMemoryUtil,'"'+$memoryUtilObject+'"')
$oldFileHolder='"'+$native+'\build\actor-runtime-probe\FileHolder.obj"'
if(!$response.Contains($oldFileHolder)){throw 'The response file does not contain the original FileHolder provider.'}
$response=$response.Replace($oldFileHolder,'"'+$fileHolderObject+'"')
$oldOutput='"'+$native+'\build\actor-runtime-link-probe.dll"'
if(!$response.Contains($oldOutput)){throw 'The selected response file does not target actor-runtime-link-probe.dll.'}
$dll=Join-Path $tmp 'galaxy-native.dll'
$response=$response.Replace($oldOutput,'"'+$dll+'"')
$providerArgs=($objects | ForEach-Object {' "'+$_.Path+'"'}) -join ''
if(!$response.Contains(' -lbcrypt')){throw 'Expected native runtime link libraries were not found in the response file.'}
$response=$response.Replace(' -lbcrypt',$providerArgs+' -ld3d11 -ld3dcompiler_47 -lbcrypt')
$probeResponse=Join-Path $tmp 'actor-runtime-link-with-dspboot.rsp'
$log=Join-Path $tmp 'actor-runtime-link-with-dspboot.log'
[IO.File]::WriteAllText($probeResponse,$response,[Text.UTF8Encoding]::new($false))
$saveProviderManifest=@(foreach($provider in $nativeSaveProviders){
 $old='"'+$provider.OriginalObject+'"';$current='"'+$provider.Path+'"'
 if($response.Contains($old)-or [regex]::Matches($response,[regex]::Escape($current)).Count-ne 1){throw "Native save link verification failed: $($provider.Unit)"}
 [pscustomobject]@{Unit=$provider.Unit;Source=$provider.Source;SourceSHA256=(Get-FileHash -LiteralPath $provider.Source -Algorithm SHA256).Hash;Object=$provider.Path;ObjectSHA256=(Get-FileHash -LiteralPath $provider.Path -Algorithm SHA256).Hash;ReplacedObject=$provider.OriginalObject}
})
$saveProviderManifest | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath (Join-Path $tmp 'native-save-providers.json')
Write-Output "PASS all7 native save providers selected exactly once; cached serializers removed from boot link."

$oldErrorActionPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
& $zig c++ "@$probeResponse" *> $log
$ErrorActionPreference=$oldErrorActionPreference
if($LASTEXITCODE){Get-Content $log -Tail 60;exit $LASTEXITCODE}
if(!(Test-Path -LiteralPath $dll) -or (Get-Item -LiteralPath $dll).Length -lt 1){throw 'The linker returned success without producing its DLL.'}
$launcher=Join-Path $tmp ('galaxy-native-'+[guid]::NewGuid().ToString('N')+'.exe')
$launcherSource=Join-Path $tmp 'native_game_launcher-crashdump.cpp'
$launcherText=Get-Content -LiteralPath (Join-Path $native 'src\native_game_launcher.cpp') -Raw
$launcherText=$launcherText.Replace('#include <shellapi.h>','#include <shellapi.h>' + "`n" + '#include <dbghelp.h>' + "`n" + '#include <cstdio>' + "`n" + '#include <cstring>' + "`n" + '#include <cstdint>')
$launcherText=$launcherText.Replace('#include <dbghelp.h>','#include <dbghelp.h>' + [Environment]::NewLine + '#include <exception>')
$launcherErrorFile=Join-Path $env:TEMP 'galaxy-native-launcher-error.log'
$launcherErrorLiteral=$launcherErrorFile.Replace('\','\\')
$launcherErrorAnchor='    const std::string text = detail ? detail : "No diagnostic was returned.";'
if(!$launcherText.Contains($launcherErrorAnchor)){throw 'Launcher error trace anchor missing.'}
$launcherText=$launcherText.Replace($launcherErrorAnchor,$launcherErrorAnchor + "`n" + '    if(FILE* trace=std::fopen("' + $launcherErrorLiteral + '","wb")){std::fwrite(text.data(),1,text.size(),trace);std::fclose(trace);}')
$crashDumpHandler=@'
static void galaxyNativeTerminate() noexcept {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, path);
    if (length && length < MAX_PATH - 28) {
        wcscat_s(path, L"galaxy-native-terminate.log");
        HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            const char message[] = "std::terminate called\r\n";
            DWORD written = 0;
            WriteFile(file, message, sizeof(message) - 1, &written, nullptr);
            CloseHandle(file);
        }
        GetTempPathW(MAX_PATH, path);
        wcscat_s(path, L"galaxy-native-terminate.dmp");
        HANDLE dump = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (dump != INVALID_HANDLE_VALUE) {
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dump,
                              static_cast<MINIDUMP_TYPE>(MiniDumpWithThreadInfo |
                                  MiniDumpWithUnloadedModules |
                                  MiniDumpWithIndirectlyReferencedMemory |
                                  MiniDumpWithPrivateReadWriteMemory),
                              nullptr, nullptr, nullptr);
            CloseHandle(dump);
        }
    }
    TerminateProcess(GetCurrentProcess(), 0xC0000409u);
}
static LONG CALLBACK galaxyNativeVectored(EXCEPTION_POINTERS* exception) noexcept {
    static volatile LONG dumpedBadFunctionTable = 0;
    if (!exception || !exception->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
    const auto exceptionCode = exception->ExceptionRecord->ExceptionCode;
    if (exceptionCode != 0xC00000FFu && exceptionCode != 0xC0000005u &&
        exceptionCode != 0xC0000374u &&
        exceptionCode != 0xC000001Du && exceptionCode != 0xC0000409u &&
        exceptionCode != 0xE06D7363u && exceptionCode != 0x20474343u) return EXCEPTION_CONTINUE_SEARCH;
    static volatile LONG gccThrowCount = 0;
    if (exceptionCode == 0x20474343u && InterlockedIncrement(&gccThrowCount) > 64) return EXCEPTION_CONTINUE_SEARCH;
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, path);
    if (!length || length >= MAX_PATH - 24) return EXCEPTION_CONTINUE_SEARCH;
    wcscat_s(path, L"galaxy-native-firstchance.log");
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        char line[1536];
        const auto code = exceptionCode;
        const auto address = exception->ExceptionRecord->ExceptionAddress;
        auto* context = exception->ContextRecord;
        void* stack[8]{};
        SIZE_T stackBytes = 0;
        void* frames[20]{};
        unsigned frameCount = 0;
        if (context) {
            CONTEXT walk = *context;
            for (; frameCount < sizeof(frames) / sizeof(frames[0]); ++frameCount) {
                frames[frameCount] = reinterpret_cast<void*>(walk.Rip);
                DWORD64 imageBase = 0;
                auto* function = RtlLookupFunctionEntry(walk.Rip, &imageBase, nullptr);
                if (!function) {
                    DWORD64 caller = 0;
                    SIZE_T read = 0;
                    if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(walk.Rsp),
                                           &caller, sizeof(caller), &read) || read != sizeof(caller) || !caller) break;
                    walk.Rip = caller;
                    walk.Rsp += sizeof(caller);
                    continue;
                }
                PVOID handlerData = nullptr;
                DWORD64 establisherFrame = 0;
                RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, walk.Rip, function,
                                 &walk, &handlerData, &establisherFrame, nullptr);
                if (!walk.Rip) break;
            }
        }
        if (context) ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(context->Rsp),
                                       stack, sizeof(stack), &stackBytes);
        const int count = std::snprintf(line, sizeof(line),
            "first-chance tid=%lu code=%08lX flags=%08lX address=%p rip=%p rsp=%p native=%p info=%llX,%llX frames=%u:%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p,%p stack=%p,%p,%p,%p,%p,%p,%p,%p\r\n",
            static_cast<unsigned long>(GetCurrentThreadId()),
            static_cast<unsigned long>(code),
            static_cast<unsigned long>(exception->ExceptionRecord->ExceptionFlags), address,
            context ? reinterpret_cast<void*>(context->Rip) : nullptr,
            context ? reinterpret_cast<void*>(context->Rsp) : nullptr,
            GetModuleHandleW(L"galaxy-native.dll"),
            static_cast<unsigned long long>(exception->ExceptionRecord->NumberParameters > 0 ? exception->ExceptionRecord->ExceptionInformation[0] : 0),
            static_cast<unsigned long long>(exception->ExceptionRecord->NumberParameters > 1 ? exception->ExceptionRecord->ExceptionInformation[1] : 0),
            frameCount, frames[0], frames[1], frames[2], frames[3], frames[4], frames[5], frames[6], frames[7],
            frames[8], frames[9], frames[10], frames[11], frames[12], frames[13], frames[14], frames[15],
            frames[16], frames[17], frames[18], frames[19],
            stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]);
        DWORD written = 0;
        if (count > 0) WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        HANDLE symbols = GetCurrentProcess();
        SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        if (SymInitialize(symbols, nullptr, TRUE)) {
            for (unsigned index = 0; index < frameCount; ++index) {
                alignas(SYMBOL_INFO) char storage[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
                auto* symbol = reinterpret_cast<SYMBOL_INFO*>(storage);
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                symbol->MaxNameLen = MAX_SYM_NAME;
                DWORD64 displacement = 0;
                char resolved[1024];
                IMAGEHLP_LINE64 sourceLine{};
                sourceLine.SizeOfStruct = sizeof(sourceLine);
                DWORD lineDisplacement = 0;
                const bool found = SymFromAddr(symbols, reinterpret_cast<DWORD64>(frames[index]), &displacement, symbol) != FALSE;
                const bool hasLine = SymGetLineFromAddr64(symbols, reinterpret_cast<DWORD64>(frames[index]), &lineDisplacement, &sourceLine) != FALSE;
                const int resolvedLength = std::snprintf(resolved, sizeof(resolved),
                    "resolved-frame=%u address=%p symbol=%s+0x%llX source=%s:%lu\r\n",
                    index, frames[index], found ? symbol->Name : "<unknown>",
                    static_cast<unsigned long long>(displacement),
                    hasLine ? sourceLine.FileName : "<unknown>", hasLine ? sourceLine.LineNumber : 0);
                if (resolvedLength > 0 && resolvedLength < static_cast<int>(sizeof(resolved)))
                    WriteFile(file, resolved, static_cast<DWORD>(resolvedLength), &written, nullptr);
            }
            SymCleanup(symbols);
        }
        CloseHandle(file);
    }
    if ((exceptionCode == 0xC00000FFu || exceptionCode == 0xC0000374u) &&
        InterlockedCompareExchange(&dumpedBadFunctionTable, 1, 0) == 0) {
        wchar_t dumpPath[MAX_PATH]{};
        const DWORD dumpPathLength = GetTempPathW(MAX_PATH, dumpPath);
        if (dumpPathLength && dumpPathLength < MAX_PATH - 32) {
            wcscat_s(dumpPath, L"galaxy-native-firstchance.dmp");
            HANDLE dumpFile = CreateFileW(dumpPath, GENERIC_WRITE, FILE_SHARE_READ,
                                          nullptr, CREATE_ALWAYS,
                                          FILE_ATTRIBUTE_NORMAL, nullptr);
            if (dumpFile != INVALID_HANDLE_VALUE) {
                MINIDUMP_EXCEPTION_INFORMATION info{GetCurrentThreadId(), exception, FALSE};
                MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile,
                                  static_cast<MINIDUMP_TYPE>(MiniDumpWithThreadInfo |
                                      MiniDumpWithUnloadedModules |
                                      MiniDumpWithIndirectlyReferencedMemory),
                                  &info, nullptr, nullptr);
                CloseHandle(dumpFile);
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
static LONG WINAPI galaxyNativeCrashDump(EXCEPTION_POINTERS* exception) noexcept {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, path);
    if (!length || length >= MAX_PATH - 24) return EXCEPTION_EXECUTE_HANDLER;
    wcscat_s(path, L"galaxy-native-crash.dmp");
    HANDLE file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return EXCEPTION_EXECUTE_HANDLER;
    MINIDUMP_EXCEPTION_INFORMATION info{GetCurrentThreadId(), exception, FALSE};
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpWithThreadInfo,
                      exception ? &info : nullptr, nullptr, nullptr);
    CloseHandle(file);
    return EXCEPTION_EXECUTE_HANDLER;
}

'@
$launcherText=$launcherText.Replace('int main() {',$crashDumpHandler + "`n" + 'int main() {' + "`n" + '    AddVectoredExceptionHandler(1, galaxyNativeVectored);' + "`n" + '    SetUnhandledExceptionFilter(galaxyNativeCrashDump);')
$launcherText=$launcherText.Replace('int main() {','int main() {' + [Environment]::NewLine + '    std::set_terminate(galaxyNativeTerminate);')
[IO.File]::WriteAllText($launcherSource,$launcherText,[Text.UTF8Encoding]::new($false))
& $zig c++ -std=c++17 -O2 -target x86_64-windows-gnu $launcherSource '-lshell32' '-luser32' '-ldbghelp' '-o' $launcher
if($LASTEXITCODE){throw 'Native Windows launcher failed to compile.'}
if(!(Test-Path -LiteralPath $launcher) -or (Get-Item -LiteralPath $launcher).Length -lt 1){throw 'The launcher compiler returned success without producing its executable.'}
[pscustomobject]@{Schema=1;LauncherName=[IO.Path]::GetFileName($launcher);LauncherSHA256=(Get-FileHash -LiteralPath $launcher -Algorithm SHA256).Hash;LibrarySHA256=(Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash;BuiltAt=(Get-Date -Format o)} | ConvertTo-Json | Set-Content -LiteralPath $runtimeManifestPath

Write-Output ("PASS actor-runtime full link and launcher build with WBFS start export: DLL {0:N0} bytes, EXE {1:N0} bytes" -f (Get-Item -LiteralPath $dll).Length,(Get-Item -LiteralPath $launcher).Length)

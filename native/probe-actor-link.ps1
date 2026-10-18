$oldGeneratorErrorActionPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
& "$PSScriptRoot\compile-speaker-system-providers.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$PSScriptRoot\compile-native-font-provider.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$PSScriptRoot\compile-native-ui-providers.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$PSScriptRoot\compile-native-content-providers.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
if($env:NATIVE_REGENERATE_RUNTIME -eq '1'){
& "$PSScriptRoot\generate-wipe-layouts.ps1"
& "$PSScriptRoot\generate-miss-layout.ps1"
& "$PSScriptRoot\generate-console.ps1"
& "$PSScriptRoot\generate-ocean-pipe.ps1"
& "$PSScriptRoot\generate-area-runtime.ps1"
& "$PSScriptRoot\generate-power-star.ps1"
& "$PSScriptRoot\generate-effect-keeper.ps1"
& "$PSScriptRoot\generate-jpa-drawing-overrides.ps1"
& "$PSScriptRoot\generate-jpa-field-overrides.ps1"
& "$PSScriptRoot\generate-jpa-resources.ps1"
& "$PSScriptRoot\generate-jpa-billboard-camera.ps1"
& "$PSScriptRoot\generate-jpa-runtime.ps1"
& "$PSScriptRoot\generate-multi-emitter.ps1"
& "$PSScriptRoot\generate-auto-effect-info.ps1"
& "$PSScriptRoot\generate-particle-holder.ps1"
& "$PSScriptRoot\generate-effect-animation-reset.ps1"
& "$PSScriptRoot\generate-vector2-normalize.ps1"
& "$PSScriptRoot\generate-camera-shaker.ps1"
& "$PSScriptRoot\generate-fur-drawer.ps1"
# The checked-in native JASBNKParser/WaveSystemParser header is the compiled
# link-probe input. Do not regenerate it during a link audit: that generator
# writes into the protected include tree and its output is already checked in.
# Checked-in generated provider; avoid rewriting the protected source here.
& "$PSScriptRoot\generate-audio-section-heap.ps1"
& "$PSScriptRoot\generate-audio-arc-interpreter.ps1"
& "$PSScriptRoot\generate-audio-scene.ps1"
& "$PSScriptRoot\generate-audio-system.ps1"
& "$PSScriptRoot\generate-sound-name-converter.ps1"
& "$PSScriptRoot\generate-sound-table.ps1"
& "$PSScriptRoot\generate-stream-lifecycle-header.ps1"
& "$PSScriptRoot\generate-aram-stream.ps1"
& "$PSScriptRoot\generate-dvd-aram-ripper-transfer.ps1"
& "$PSScriptRoot\generate-dvd-aram-ripper-entry.ps1"
& "$PSScriptRoot\generate-bank-dispatch.ps1"
& "$PSScriptRoot\generate-drum-set.ps1"
& "$PSScriptRoot\generate-bank-instrument.ps1"
& "$PSScriptRoot\generate-bank-effector.ps1"
& "$PSScriptRoot\generate-audio-probe.ps1"
& "$PSScriptRoot\generate-audio-dac-mix.ps1"
& "$PSScriptRoot\generate-audio-frame.ps1"
& "$PSScriptRoot\generate-collision-scene-no-parts.ps1"
& "$PSScriptRoot\generate-audio-dsp-state.ps1"
& "$PSScriptRoot\generate-gx-project.ps1"
& "$PSScriptRoot\generate-animation-playing.ps1"
& "$PSScriptRoot\generate-actor-animation-sound.ps1"
& "$PSScriptRoot\generate-animation-sound-object.ps1"
& "$PSScriptRoot\generate-sequence-reader.ps1"
& "$PSScriptRoot\generate-sequence-parser.ps1"
& "$PSScriptRoot\generate-direct-draw.ps1"
& "$PSScriptRoot\generate-shadow-draw.ps1"
& "$PSScriptRoot\generate-gravity-routing.ps1"
& "$PSScriptRoot\generate-random.ps1"
& "$PSScriptRoot\generate-direction-blend.ps1"
& "$PSScriptRoot\generate-joint-controller.ps1"
& "$PSScriptRoot\generate-stage-switch-lifetime.ps1"
& "$PSScriptRoot\generate-parts-model.ps1"
& "$PSScriptRoot\generate-joint-access.ps1"
& "$PSScriptRoot\generate-contact-classification.ps1"
& "$PSScriptRoot\generate-binder-access.ps1"
& "$PSScriptRoot\generate-actor-groups.ps1"
& "$PSScriptRoot\generate-player-access.ps1"
& "$PSScriptRoot\generate-player-distance.ps1"
& "$PSScriptRoot\generate-jut-assert.ps1"
& "$PSScriptRoot\generate-sleep-controller.ps1"
& "$PSScriptRoot\generate-placement-matrix.ps1"
& "$PSScriptRoot\generate-area-object.ps1"
& "$PSScriptRoot\generate-light-services.ps1"
& "$PSScriptRoot\generate-light-blend.ps1"
& "$PSScriptRoot\generate-zone-light-data.ps1"
& "$PSScriptRoot\generate-area-light-data.ps1"
& "$PSScriptRoot\generate-actor-light-load.ps1"

& "$PSScriptRoot\generate-sdk-projection.ps1"
& "$PSScriptRoot\generate-both-dir-list.ps1"
& "$PSScriptRoot\generate-light-object.ps1"
& "$PSScriptRoot\generate-display-copy-emit.ps1"
& "$PSScriptRoot\generate-display-copy-setup.ps1"
& "$PSScriptRoot\generate-camera-access.ps1"
& "$PSScriptRoot\generate-model-fog.ps1"
& "$PSScriptRoot\generate-screen-access.ps1"
& "$PSScriptRoot\generate-texture-copy-emit.ps1"
& "$PSScriptRoot\generate-texture-copy-setup.ps1"
& "$PSScriptRoot\generate-display-copy-filter.ps1"
& "$PSScriptRoot\generate-display-copy-scale.ps1"
& "$PSScriptRoot\probe-main-loop.ps1"
& "$PSScriptRoot\generate-texture-buffer-size.ps1"
& "$PSScriptRoot\probe-screen-capture.ps1"
& "$PSScriptRoot\generate-model-orientation.ps1"
& "$PSScriptRoot\generate-vector-projection.ps1"
& "$PSScriptRoot\generate-memory-copy.ps1"
& "$PSScriptRoot\generate-model-env-init.ps1"
& "$PSScriptRoot\probe-shape-packet-userdata.ps1"
& "$PSScriptRoot\probe-original-gd-effects.ps1"
& "$PSScriptRoot\probe-original-model-x.ps1"
& "$PSScriptRoot\generate-model-fur-check.ps1"
}
$ErrorActionPreference=$oldGeneratorErrorActionPreference
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;if(-not $env:ZIG_GLOBAL_CACHE_DIR){$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"}
& "$n\build-rfl-original.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
if($env:NATIVE_REGENERATE_RUNTIME -eq '1'){
& "$n\generate-gx-host-init.ps1"
& "$n\generate-texture-object.ps1"
& "$n\generate-model-access.ps1"
& "$n\generate-animation-helper-runtime.ps1"
& "$n\generate-player-util-runtime.ps1"
& "$n\generate-native-retail-tables.ps1"
}
$line=Get-Content "$n\test-original-kcl-server.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\src\native_collision_scene.cpp" "$n\src\generated\CollisionSceneKernel.cpp" "$p\Petari\src\Game\Map\CollisionDirector.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\CollisionCodeMetadata.cpp" "$n\src\sensor_hash_port.cpp" "$n\src\original_live_actor_link_entry.cpp" "$n\src\gpu_resources.cpp" '
& "$n\generate-native-game-entry.ps1" | Out-Null
$gameEntryProviderDir="$n\build\native-game-system-entry-audit"
New-Item -ItemType Directory -Force -Path $gameEntryProviderDir | Out-Null
$gameEntryProvider="$gameEntryProviderDir\GameSystemEntry.obj"
$gameEntryCommon=@('-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\nw4r\include",'-include',"$n\include-runtime\native_no_weak.h",'-include','revolution/types.h','-include',"$n\include-runtime\revolution\os.h")
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @gameEntryCommon -c "$n\src\generated\GameSystemEntry.cpp" -o $gameEntryProvider
if($LASTEXITCODE -ne 0){throw 'Standalone original game entry provider failed to compile'}
$providers+=' "'+$gameEntryProvider+'" '
$gameEntryAdded=1
$providers+=' "$n\src\native_game_language.cpp" "$n\src\gx_depth_peek_port.cpp" "$n\src\wii_sound_mode_port.cpp" "$n\src\collision_code_triangle_queries_port.cpp" '
$providers+=' "$n\build\NativeRetailTables.cpp" '
$providers+=' "$n\build\actor-runtime-font\JUTFont.obj" '
$providers+=' "$n\build\actor-runtime-ui\InformationObserver.obj" "$n\build\actor-runtime-ui\RumbleCalculator.obj" '
$providers+=' "$n\build\actor-runtime-content-providers\AutoEffectGroup.obj" "$n\build\actor-runtime-content-providers\StationedArchiveLoader.obj" '
$providers+=' "$n\build\actor-runtime-audio-host\SpkSystem.obj" "$n\build\actor-runtime-audio-host\SpkMixingBuffer.obj" "$n\build\actor-runtime-audio-host\SpkData.obj" "$n\build\actor-runtime-audio-host\SpkTableConstructor.obj" "$n\build\actor-runtime-audio-host\SpkWave.obj" "$n\build\actor-runtime-audio-host\SpkSound.obj" "$n\build\actor-runtime-audio-host\SpkSpeakerCtrlHost.obj" "$n\build\actor-runtime-audio-host\AudMePlayer.obj" "$n\build\actor-runtime-audio-host\NativeMeParams.obj" "$n\build\actor-runtime-audio-host\AudMeHandles.obj" "$n\build\actor-runtime-audio-host\AudMeTrack.obj" "$n\build\actor-runtime-audio-host\AudMeSeqCtrl.obj" "$n\build\actor-runtime-audio-host\AudMeChannelMgr.obj" "$n\build\actor-runtime-audio-host\AudMeTrackCallback.obj" "$n\build\actor-runtime-audio-host\AudRhythmHolder.obj" "$n\build\actor-runtime-audio-host\AudRhythmSeqParser.obj" "$n\build\actor-runtime-audio-host\AudChordInfo.obj" "$n\build\actor-runtime-audio-host\NativeChordTable.obj" "$n\build\actor-runtime-audio-host\AudMeSeqReader.obj" "$n\build\actor-runtime-audio-host\AudMeSeqParser.obj" '
$providers+=' "$n\src\gd_storage.cpp" "$n\src\generated\NarrowTextWriter.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" '
$providers+=' "$n\src\sdk_cache_port.cpp" "$n\src\native_arc_port.cpp" "$n\src\native_jkr_archive_port.cpp" "$n\src\generated\HeapCopyKernel.cpp" "$n\src\generated\AramCompletionKernel.cpp" "$n\src\native_jkr_aram_port.cpp" "$n\src\native_dvd_port.cpp" "$n\src\native_os_power_port.cpp" '
$providers+=' "$n\src\aram_read_port.cpp" "$n\src\generated\JKRCompArchive.cpp" "$n\src\generated\JKRAramArchive.cpp" "$n\src\sdk_report_port.cpp" "$n\src\aram_manager_kernel.cpp" "$n\src\aram_create_kernel.cpp" "$n\src\generated\AramQueueKernel.cpp" "$n\src\generated\AramCommandKernel.cpp" "$n\src\generated\JKRAramHeap.cpp" "$n\src\generated\JKRAramBlock.cpp" "$n\src\native_material_factory_port.cpp" "$n\src\gx_array_port.cpp" "$n\src\gx_display_list_port.cpp" '
$providers+=' "$n\src\generated\GDGeometry.cpp" "$n\src\generated\GDTexture.cpp" "$n\build\actor-runtime-probe\GDTransform.obj" '
$providers+=' "$n\src\j3d_shape_matrix_dispatch.cpp" '
$providers+=' "$n\src\gx_indirect_port.cpp" "$n\src\gx_draw_port.cpp" "$n\src\gx_vertex_cache_port.cpp" "$n\src\gx_texture_cache_port.cpp" '
$providers+=' "$n\src\gx_texture_mode_port.cpp" '
$providers+=' "$n\src\generated\BothDirListKernel.cpp" "$n\src\gx_tev_port.cpp" "$n\src\gx_dstalpha_port.cpp" "$n\src\gx_projection_port.cpp" "$n\src\generated\SdkProjectionKernel.cpp" "$n\src\gx_viewport_port.cpp" "$n\src\gx_scissor_port.cpp" "$n\src\generated\ActorLightLoadKernel.cpp" "$n\src\generated\LightServicesKernel.cpp" "$n\src\generated\AreaLightDataKernel.cpp" "$n\src\generated\ZoneLightDataKernel.cpp" "$n\src\gx_light_object_port.cpp" "$n\src\generated\LightObjectKernel.cpp" '
$providers+=' "$n\src\gx_host_service_port.cpp" "$n\src\gx_efb_readback_port.cpp" "$n\src\generated\GxHostInitKernel.cpp" '
$providers+=' "$n\src\j3d_packet_base_port.cpp" "$n\src\os_critical_port.cpp" "$n\src\os_message_port.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_time_port.cpp" "$n\src\os_alarm_port.cpp" "$n\src\generated\DrawSyncManager.cpp" "$n\src\vi_port.cpp" "$n\src\gx_draw_completion_port.cpp" "$n\src\gx_draw_sync_completion_port.cpp" "$n\src\gx_submission_port.cpp" '
$providers+=' "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\os_zero_memory_port.cpp" "$n\src\heap_arena_port.cpp" "$n\src\input_workspace_port.cpp" "$n\src\resource_font_port.cpp" "$n\src\game_font_backend.cpp" "$n\src\game_message_backend.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\build\actor-runtime-jutexception\JUTConsoleHostDiagnosticsNoPanic.obj" "$n\src\generated\JUTAssertKernel.cpp" '
$providers+= ' "$n\src\display_config_port.cpp" "$n\src\hash_sort_table_port.cpp" "$n\src\generated\AnimationHelperRuntimeKernel.cpp" "$n\src\generated\LightBlendKernel.cpp" "$n\src\generated\PlayerDistanceKernel.cpp" "$n\src\generated\StageSwitchLifetimeKernel.cpp" "$p\Petari\src\Game\Player\MarioHolder.cpp" "$n\src\generated\PlacementMatrixKernel.cpp" "$n\src\vector_blend_port.cpp" "$n\src\native_bck_animation.cpp" '
$providers+=' "$n\src\j3d_no_animation_base.cpp" "$n\src\j3d_draw_buffer_cleanup.cpp" '
$providers+='  "$n\src\generated\VectorProjectionKernel.cpp" "$n\src\generated\ModelOrientationKernel.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" '
$providers+=' "$n\build\actor-runtime-probe\J3DModelX.obj" '
$providers+=' "$n\build\actor-runtime-probe\GDPixel.obj" "$n\build\actor-runtime-probe\GDTev.obj" "$n\build\actor-runtime-probe\GDIndirect.obj" "$n\build\actor-runtime-probe\GDLight.obj" '
$providers+=' "$p\Petari\src\Game\Util\SchedulerUtil.cpp" "$p\Petari\src\Game\Map\LightDirector.cpp" "$p\Petari\src\Game\AreaObj\LightAreaHolder.cpp" "$p\Petari\src\Game\AreaObj\LightArea.cpp" "$p\Petari\src\Game\AreaObj\AreaForm.cpp" "$p\Petari\src\Game\AreaObj\AreaObjFollower.cpp" "$p\Petari\src\Game\Map\SleepController.cpp" "$n\src\generated\SleepControllerHolderKernel.cpp" "$p\Petari\src\Game\Util\BaseMatrixFollowTargetHolder.cpp" "$p\Petari\src\Game\Util\JMapLinkInfo.cpp" "$p\Petari\src\Game\Map\LightPointCtrl.cpp" "$p\Petari\src\Game\Map\LightDataHolder.cpp" '
$providers+=' "$p\Petari\src\Game\LiveActor\HitSensor.cpp" "$p\Petari\src\Game\LiveActor\HitSensorKeeper.cpp" "$p\Petari\src\Game\LiveActor\HitSensorInfo.cpp" "$p\Petari\src\Game\LiveActor\SensorHitChecker.cpp" '
$providers+=' "$n\src\generated\ActorGroupsKernel.cpp" '
$providers+=' "$p\Petari\src\Game\Player\GroupChecker.cpp" '
$providers+=' "$p\Petari\src\Game\Util\CollisionPartsFilter.cpp" "$p\Petari\src\Game\LiveActor\MirrorCamera.cpp" "$p\Petari\src\Game\LiveActor\MirrorActor.cpp" '
$providers+=' '
$providers+=' "$n\src\generated\PartsModelKernel.cpp" "$n\src\generated\FixedPositionKernel.cpp" "$p\Petari\src\Game\LiveActor\ModelObj.cpp" "$p\Petari\src\Game\LiveActor\LodCtrl.cpp" "$p\Petari\src\Game\LiveActor\ShadowController.cpp" "$p\Petari\src\Game\Util\ActorShadowUtil.cpp" "$p\Petari\src\Game\LiveActor\ShadowDrawer.cpp" "$p\Petari\src\Game\Util\ActorShadowLocalUtil.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeDrawer.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeBox.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeCylinder.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeFlatModel.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeLine.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeOval.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeSphere.cpp" "$p\Petari\src\Game\LiveActor\ShadowSurfaceDrawer.cpp" "$p\Petari\src\Game\LiveActor\ShadowSurfaceCircle.cpp" "$p\Petari\src\Game\LiveActor\ShadowSurfaceOval.cpp" "$p\Petari\src\Game\LiveActor\ShadowSurfaceBox.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeOvalPole.cpp" "$p\Petari\src\Game\LiveActor\ShadowVolumeModel.cpp" "$n\src\generated\ShadowDrawKernel.cpp" "$n\src\generated\DirectDrawKernel.cpp" "$n\src\generated\GXProjectKernel.cpp" "$p\Petari\src\Game\LiveActor\ActorJointCtrl.cpp" "$n\src\generated\DynamicJointCtrlKernel.cpp" "$p\Petari\src\Game\LiveActor\FaceJointCtrl.cpp" "$n\src\generated\IKJointCtrlKernel.cpp" "$p\Petari\src\Game\Util\JointController.cpp" "$p\Petari\src\Game\Util\IKJoint.cpp" "$n\src\generated\DirectionBlendKernel.cpp" "$n\src\generated\DirectionTurnKernel.cpp" "$n\src\generated\RandomKernel.cpp" "$n\src\generated\GravityRoutingKernel.cpp" "$n\src\generated\PlanetGravityManagerKernel.cpp" "$p\Petari\src\Game\Gravity\GravityInfo.cpp" "$p\Petari\src\Game\Gravity\PlanetGravity.cpp" "$p\Petari\src\Game\Gravity\PointGravity.cpp" '
$providers+=' "$n\src\generated\ModelFogKernel.cpp" '
$providers+=' "$n\build\actor-runtime-probe\ShapePacketUserData.obj" '
$providers+='  "$n\src\generated\VectorProjectionKernel.cpp" "$n\src\generated\ModelOrientationKernel.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" '
$providers+=' "$n\build\actor-runtime-probe\CaptureScreenDirector.obj" "$n\build\actor-runtime-probe\JUTTexture.obj" "$n\build\actor-runtime-probe\JUTPalette.obj" "$n\build\actor-runtime-probe\JUTVideo.obj" "$n\build\actor-runtime-probe\JUTXfb.obj" "$n\build\actor-runtime-probe\JUTDirectPrint.obj" "$n\build\actor-runtime-probe\MainLoopFramework.obj" "$n\src\generated\DisplayCopyScaleKernel.cpp" "$n\src\generated\DisplayCopyFilterKernel.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" "$n\src\generated\DisplayCopySetupKernel.cpp" "$n\src\generated\DisplayCopyEmitKernel.cpp" "$n\src\gx_display_copy_port.cpp" "$n\src\generated\TextureCopyEmitKernel.cpp" "$n\src\gx_texture_copy_port.cpp"  "$n\src\generated\CameraAccessKernel.cpp" "$p\Petari\src\Game\Camera\CameraContext.cpp" '
$providers+= ' "$p\Petari\src\Game\Util\FurMulti.cpp" "$p\Petari\src\Game\Util\FurCtrl.cpp" "$n\src\generated\FurDrawer.cpp" "$p\Petari\src\Game\Util\FurShader.cpp" "$p\Petari\src\Game\Map\Flag.cpp" "$p\Petari\src\Game\Map\NamePosHolder.cpp" "$n\src\generated\EffectKeeper.cpp" "$p\Petari\src\Game\Effect\SyncBckEffectChecker.cpp" "$p\Petari\src\Game\Effect\SyncBckEffectInfo.cpp" "$p\Petari\src\Game\System\WPadRumble.cpp" "$p\Petari\src\Game\System\WPadRumbleData.cpp" "$p\Petari\src\Game\Scene\StopSceneController.cpp" "$p\Petari\src\Game\Scene\SceneExecutor.cpp" "$p\Petari\src\Game\Util\EffectUtil.cpp" "$p\Petari\src\Game\Effect\EffectSystemUtil.cpp" "$p\Petari\src\Game\Effect\ParticleEmitter.cpp" "$n\src\generated\ParticleEmitterHolder.cpp" "$p\Petari\src\Game\Effect\ParticleCalcExecutor.cpp" "$p\Petari\src\Game\Effect\ParticleDrawExecutor.cpp" "$p\Petari\src\Game\Effect\EffectSystem.cpp" "$p\Petari\src\Game\Effect\SingleEmitter.cpp" "$p\Petari\src\Game\Effect\MultiEmitterParticleCallBack.cpp" "$p\Petari\src\Game\Effect\AutoEffectGroupHolder.cpp" "$n\src\generated\AutoEffectInfo.cpp" "$n\src\generated\MultiEmitter.cpp" "$p\Petari\src\Game\Effect\MultiEmitterAccess.cpp" "$p\Petari\src\Game\Effect\MultiEmitterCallBack.cpp" "$p\Petari\src\Game\Effect\ParticleResourceHolder.cpp" "$n\src\generated\JPAEmitterManager.cpp" "$n\src\generated\JPAMath.cpp" "$n\src\generated\JPABillboardCameraKernel.cpp" "$p\Petari\src\JSystem\JParticle\JPAEmitter.cpp" "$p\Petari\src\JSystem\JParticle\JPAParticle.cpp" "$n\src\generated\JPAResourceManager.cpp" "$n\src\generated\JPAResource.cpp" "$n\src\generated\JPAResourceLoader.cpp" "$n\src\generated\JPATexture.cpp" "$n\src\generated\JPAKeyBlock.cpp" "$n\src\generated\JPAExTexShape.cpp" "$n\src\generated\JPADynamicsBlock.cpp" "$n\src\generated\JPAChildShape.cpp" "$n\src\generated\JPAExtraShape.cpp" "$n\src\generated\JPABaseShape.cpp" "$n\src\generated\JPAFieldBlock.cpp" "$n\src\generated\JPAFieldOverrideKernel.cpp" "$n\src\generated\JPADrawingOverrideKernel.cpp" "$n\src\generated\Vector2NormalizeKernel.cpp" "$n\src\generated\CameraShaker.cpp" "$p\Petari\src\Game\Camera\CameraShakeTask.cpp" "$p\Petari\src\Game\Camera\CameraShakePatternImpl.cpp" "$p\Petari\src\Game\MapObj\BenefitItemObj.cpp" "$p\Petari\src\Game\LiveActor\FlashingCtrl.cpp" "$p\Petari\src\Game\MapObj\BenefitItemLifeUp.cpp" "$p\Petari\src\Game\MapObj\BenefitItemInvincible.cpp" "$p\Petari\src\Game\MapObj\CoinHolder.cpp" "$p\Petari\src\Game\MapObj\StarPieceDirector.cpp" "$p\Petari\src\Game\MapObj\Coin.cpp" "$p\Petari\src\Game\MapObj\StarPiece.cpp" "$n\src\generated\PowerStar.cpp" "$p\Petari\src\Game\MapObj\PowerStarHolder.cpp" "$p\Petari\src\Game\MapObj\PowerStarAppearPoint.cpp" "$p\Petari\src\Game\Util\EventUtil.cpp" "$p\Petari\src\Game\Util\AreaObjUtil.cpp" "$n\src\generated\AreaObj.cpp" "$n\src\generated\AreaObjContainer.cpp" "$p\Petari\src\JSystem\J2DGraph\J2DOrthoGraph.cpp" "$p\Petari\src\JSystem\J2DGraph\J2DGrafContext.cpp" "$n\src\generated\JUTConsole.cpp" "$p\Petari\src\JSystem\JGadget\linklist.cpp" "$p\Petari\src\Game\AreaObj\ImageEffectArea.cpp" "$p\Petari\src\Game\Screen\ImageEffectDirector.cpp" "$p\Petari\src\Game\Screen\ImageEffectState.cpp" "$p\Petari\src\Game\Screen\ImageEffectBase.cpp" "$p\Petari\src\Game\Util\ScreenUtil.cpp" "$p\Petari\src\Game\LiveActor\ActorCameraInfo.cpp" "$p\Petari\src\Game\Camera\CameraTargetArg.cpp" "$p\Petari\src\Game\Camera\CameraTargetMtx.cpp" "$p\Petari\src\Game\Camera\CameraTargetObj.cpp" "$p\Petari\src\Game\Camera\CameraTargetHolder.cpp" "$p\Petari\src\Game\Camera\CameraDirector.cpp" "$p\Petari\src\Game\Camera\CameraMan.cpp" "$p\Petari\src\Game\Camera\CameraManPause.cpp" "$p\Petari\src\Game\Camera\CameraManSubjective.cpp" "$p\Petari\src\Game\Camera\CameraSubjective.cpp" "$p\Petari\src\Game\Camera\Camera.cpp" "$p\Petari\src\Game\Camera\CameraLocalUtil.cpp" "$p\Petari\src\Game\Camera\CameraRegisterHolder.cpp" "$p\Petari\src\Game\Camera\CamHeliEffector.cpp" "$p\Petari\src\Game\Camera\CamKarikariEffector.cpp" "$p\Petari\src\Game\Camera\CameraHeightArrange.cpp" "$p\Petari\src\Game\Camera\CameraPoseParam.cpp" "$p\Petari\src\Game\AreaObj\CubeCamera.cpp" "$p\Petari\src\Game\Screen\GameSceneLayoutHolder.cpp" "$p\Petari\src\Game\Screen\SimpleLayout.cpp" "$p\Petari\src\Game\Screen\GameStageClearSequence.cpp"  "$p\Petari\src\Game\Screen\SystemWipeHolder.cpp" "$p\Petari\src\Game\Screen\SceneWipeHolder.cpp" "$p\Petari\src\Game\Screen\WipeHolderBase.cpp"  "$p\Petari\src\Game\Screen\WipeFade.cpp" "$n\src\generated\WipeRing.cpp" "$n\src\generated\WipeKoopa.cpp" "$p\Petari\src\Game\Screen\WipeGameOver.cpp" "$p\Petari\src\Game\Screen\InformationMessage.cpp" "$p\Petari\src\Game\Screen\YesNoLayout.cpp" "$p\Petari\src\Game\Screen\YesNoController.cpp" "$p\Petari\src\Game\Screen\IconAButton.cpp" "$p\Petari\src\Game\Screen\MarioMeter.cpp" "$p\Petari\src\Game\Screen\MarioSubMeter.cpp" "$n\src\generated\MissLayout.cpp" "$p\Petari\src\Game\Screen\OneUpBoard.cpp" "$p\Petari\src\Game\Screen\NoteCounter.cpp" "$p\Petari\src\Game\Screen\CameraInfo.cpp" "$p\Petari\src\Game\Screen\CounterLayoutController.cpp" "$p\Petari\src\Game\Screen\LayoutGroupCtrl.cpp" "$p\Petari\src\Game\Screen\CounterLayoutAppearer.cpp" "$p\Petari\src\Game\Screen\CountUpPaneRumbler.cpp" "$p\Petari\src\Game\Screen\ButtonPaneController.cpp" "$p\Petari\src\Game\Screen\CoinCounter.cpp" "$p\Petari\src\Game\Screen\CenterScreenBlur.cpp" "$p\Petari\src\Game\Screen\StarCounter.cpp" "$p\Petari\src\Game\Screen\StarPieceCounter.cpp" "$p\Petari\src\Game\Screen\PurpleCoinCounter.cpp" "$p\Petari\src\Game\Screen\BloomEffect.cpp" "$p\Petari\src\Game\Screen\BloomEffectSimple.cpp" "$p\Petari\src\Game\Screen\DepthOfFieldBlur.cpp" "$p\Petari\src\Game\Screen\CopyFilterNegater.cpp" "$p\Petari\src\Game\Screen\ImageEffectResource.cpp" "$p\Petari\src\Game\Screen\ImageEffectSystemHolder.cpp" "$p\Petari\src\Game\AreaObj\BloomArea.cpp" "$p\Petari\src\Game\AreaObj\SimpleBloomArea.cpp" "$p\Petari\src\Game\AreaObj\DepthOfFieldArea.cpp" "$p\Petari\src\Game\AreaObj\ScreenBlurArea.cpp" "$p\Petari\src\Game\Screen\ImageEffectLocalUtil.cpp" "$p\Petari\src\Game\AreaObj\GlaringLightArea.cpp" "$p\Petari\src\Game\AreaObj\WaterArea.cpp" "$p\Petari\src\Game\AreaObj\RestartCube.cpp" "$p\Petari\src\Game\System\GameDataTemporaryInGalaxy.cpp" "$p\Petari\src\Game\System\AlreadyDoneFlagInGalaxy.cpp" "$p\Petari\src\Game\Map\WaterAreaHolder.cpp" "$p\Petari\src\Game\Screen\WaterCameraFilter.cpp" "$p\Petari\src\Game\Map\WhirlPoolAccelerator.cpp" "$p\Petari\src\Game\Map\OceanBowl.cpp" "$p\Petari\src\Game\Map\OceanRing.cpp" "$p\Petari\src\Game\Map\OceanSphere.cpp" "$p\Petari\src\Game\Map\OceanBowlPoint.cpp" "$p\Petari\src\Game\Map\OceanBowlBloomDrawer.cpp" "$p\Petari\src\Game\Map\OceanRingDrawer.cpp" "$p\Petari\src\Game\Map\OceanRingPipe.cpp" "$p\Petari\src\Game\Map\OceanRingPipeInside.cpp" "$p\Petari\src\Game\Map\OceanRingPipeOutside.cpp" "$p\Petari\src\Game\Map\OceanRingBloomDrawer.cpp" "$p\Petari\src\Game\Map\OceanSpherePoint.cpp" "$p\Petari\src\Game\Map\WaterPoint.cpp"  "$p\Petari\src\Game\Scene\GameSceneFunction.cpp" "$p\Petari\src\Game\NPC\StageStateKeeper.cpp" "$p\Petari\src\Game\NPC\EventDirector.cpp" "$p\Petari\src\Game\NPC\CometEventKeeper.cpp" "$p\Petari\src\Game\NPC\CometEventExecutorTimeLimit.cpp" "$p\Petari\src\Game\Screen\GalaxyCometScreenFilter.cpp" "$p\Petari\src\Game\Ride\SwingRopePoint.cpp" '
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-luigi-left-supplier -OnlyUnits @('LuigiLeftSupplier') | Out-Null
$luigiLeftResults=Get-Content "$n\build\actor-runtime-luigi-left-supplier\results.json" -Raw|ConvertFrom-Json
if(@($luigiLeftResults|Where-Object ExitCode -ne 0).Count){throw 'Original LuigiLeftSupplier failed to compile'}
$providers+=' "$n\build\actor-runtime-luigi-left-supplier\LuigiLeftSupplier.obj" '
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-movie-subtitles -OnlyUnits @('MovieSubtitles','MovieSubtitlesDataTable') | Out-Null
$movieSubtitleResults=Get-Content "$n\build\actor-runtime-movie-subtitles\results.json" -Raw|ConvertFrom-Json
if(@($movieSubtitleResults|Where-Object ExitCode -ne 0).Count){throw 'Original movie subtitle providers failed to compile'}
$providers+=' "$n\build\actor-runtime-movie-subtitles\MovieSubtitles.obj" "$n\build\actor-runtime-movie-subtitles\MovieSubtitlesDataTable.obj" '
$moviePlayerAdded=0
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-movie-player-20261011 -OnlyUnits @('MoviePlayerSimple','THPSimplePlayerWrapper','THPDraw') | Out-Null
$moviePlayerResults=Get-Content "$n\build\actor-runtime-movie-player-20261011\results.json" -Raw|ConvertFrom-Json
if(@($moviePlayerResults|Where-Object ExitCode -ne 0).Count){throw 'Original MoviePlayerSimple provider failed to compile'}
$providers+=' "$n\build\actor-runtime-movie-player-20261011\MoviePlayerSimple.obj" "$n\build\actor-runtime-movie-player-20261011\THPSimplePlayerWrapper.obj" "$n\build\actor-runtime-movie-player-20261011\THPDraw.obj" "$n\src\thp_audio_port.cpp" "$n\src\thp_cache_port.cpp" "$n\src\thp_video_port.cpp" '
$moviePlayerAdded=5
$waterPlantAdded=0
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-water-plant-20261011 -OnlyUnits @('WaterPlant') | Out-Null
$waterPlantResults=Get-Content "$n\build\actor-runtime-water-plant-20261011\results.json" -Raw|ConvertFrom-Json
if(@($waterPlantResults|Where-Object ExitCode -ne 0).Count){throw 'Original WaterPlant provider failed to compile'}
$providers+=' "$n\build\actor-runtime-water-plant-20261011\WaterPlant.obj" '
$waterPlantAdded=1
$jutExceptionAdded=0
& "$n\compile-native-jutexception.ps1" | Out-Null
if(!(Test-Path "$n\build\actor-runtime-jutexception\JUTExceptionHost.obj")){throw 'Host JUTException startup provider failed to compile'}
$providers+=' "$n\build\actor-runtime-jutexception\JUTExceptionHost.obj" "$n\build\actor-runtime-jutexception\JUTConsoleHostDiagnosticsNoPanic.obj" '
$jutExceptionAdded=2
$sunshadeUnits=@('SunshadeMapParts','SunshadeMapHolder')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-sunshade-providers -OnlyUnits $sunshadeUnits | Out-Null
$sunshadeResults=Get-Content "$n\build\actor-runtime-sunshade-providers\results.json" -Raw|ConvertFrom-Json
if(@($sunshadeResults|Where-Object ExitCode -ne 0).Count){throw 'Original SunshadeMapParts providers failed to compile'}
$sunshadeAdded=0
foreach($sunshadeProvider in $sunshadeResults){$providers+=' "'+"$n\build\actor-runtime-sunshade-providers\$($sunshadeProvider.Unit).obj"+'" ';$sunshadeAdded++}
$homeButtonHostObject="$n\build\actor-runtime-jutexception\RsoHomeButtonHost.obj"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -ffunction-sections -fdata-sections -target x86_64-windows-gnu `
    "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" `
    -include revolution/types.h -c "$n\src\rso_home_button_host.cpp" -o $homeButtonHostObject
if($LASTEXITCODE){throw 'Host Home Button wrapper failed to compile'}
$providers+=' "'+$homeButtonHostObject+'" '
$homeButtonHostAdded=1
$j2dPictureRun='actor-runtime-j2d-picture-provider'
$j2dPictureUnits=@('J2DPicture','J2DPane','J2DScreen','J2DManage')
& "$n\probe-actor-runtime.ps1" -RunName $j2dPictureRun -OnlyUnits $j2dPictureUnits | Out-Null
$j2dPictureResults=Get-Content "$n\build\$j2dPictureRun\results.json" -Raw|ConvertFrom-Json
if(@($j2dPictureResults|Where-Object ExitCode -ne 0).Count){throw 'Original J2DPicture provider failed to compile'}
$j2dPictureAdded=0
foreach($j2dPictureProvider in $j2dPictureResults){$providers+=' "'+"$n\build\$j2dPictureRun\$($j2dPictureProvider.Unit).obj"+'" ';$j2dPictureAdded++}
$providers+=' "$n\src\drawutil_projection_2d_port.cpp" '
$drawUtilProjectionAdded=1
$stationedFileRun='actor-runtime-stationed-file-info-provider'
& "$n\probe-actor-runtime.ps1" -RunName $stationedFileRun -OnlyUnits @('StationedFileInfo') | Out-Null
$stationedFileResults=Get-Content "$n\build\$stationedFileRun\results.json" -Raw|ConvertFrom-Json
if(@($stationedFileResults|Where-Object ExitCode -ne 0).Count){throw 'Original stationed-file table provider failed to compile'}
$providers+=' "'+"$n\build\$stationedFileRun\StationedFileInfo.obj"+'" '
$stationedFileAdded=1
$results=Get-Content "$n\build\actor-runtime-probe\results.json" -Raw|ConvertFrom-Json
$providers+=' '

# Compile the original spin-driver actor with only its Wii MMIO vertex writes
# adapted to the host GD display-list stream, then link it with the serializer.
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-spin-driver -OnlyUnits @('SpinDriverPathDrawer') | Out-Null
$spinDriverStatus=Get-Content "$n\build\actor-runtime-spin-driver\results.json" -Raw|ConvertFrom-Json
if(@($spinDriverStatus|Where-Object ExitCode -ne 0).Count){throw 'Host-adapted SpinDriverPathDrawer failed to compile'}
$providers+=' "$n\build\actor-runtime-spin-driver\SpinDriverPathDrawer.obj" "$n\src\gx_immediate_vertex_port.cpp" '
$providers+= ' "$n\src\generated\BinaryDataContentAccessor.cpp" '
$providers+= ' "$n\src\nw4r_pane_default_ctor.cpp" '

# Add original actor lifecycle and animation-randomization helpers used by
# single-player actors, keeping them separately visible in the link manifest.
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-actor-animation -OnlyUnits @('ActiveActorList','AnimationRandomPlayer') | Out-Null
$actorAnimationStatus=Get-Content "$n\build\actor-runtime-actor-animation\results.json" -Raw|ConvertFrom-Json
if(@($actorAnimationStatus|Where-Object ExitCode -ne 0).Count){throw 'Original actor lifecycle/animation helper compile failed'}
$providers+=' "$n\build\actor-runtime-actor-animation\ActiveActorList.obj" "$n\build\actor-runtime-actor-animation\AnimationRandomPlayer.obj" '

# Original model-drawer constructors and draw setup are needed by actors that
# submit their J3D models to named/volume draw buffers.
$modelDrawerUnits=@('VolumeModelDrawer','SimpleJ3DModelDrawer')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-model-drawers-20261011 -OnlyUnits $modelDrawerUnits | Out-Null
$modelDrawerStatus=Get-Content "$n\build\actor-runtime-model-drawers-20261011\results.json" -Raw|ConvertFrom-Json
if(@($modelDrawerStatus|Where-Object ExitCode -ne 0).Count){throw 'Original model-drawer provider compile failed'}
foreach($modelDrawer in $modelDrawerStatus){$providers+=' "'+$n+'\build\actor-runtime-model-drawers-20261011\'+$modelDrawer.Unit+'.obj" '}

# Rail graphs define directed stage paths used by moving and guided actors.
$railGraphUnits=@('RailGraph','RailGraphNode','RailGraphEdge','RailGraphIter','RailGraphUtil')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-rail-graph-20261011 -OnlyUnits $railGraphUnits | Out-Null
$railGraphStatus=Get-Content "$n\build\actor-runtime-rail-graph-20261011\results.json" -Raw|ConvertFrom-Json
if(@($railGraphStatus|Where-Object ExitCode -ne 0).Count){throw 'Original rail-graph provider compile failed'}
foreach($railGraph in $railGraphStatus){$providers+=' "'+$n+'\build\actor-runtime-rail-graph-20261011\'+$railGraph.Unit+'.obj" '}

# Use Petari's original state lifecycle helpers used by actors and NerveExecutors.
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-actor-state-util-20261011 -OnlyUnits @('ActorStateUtil') | Out-Null
$actorStateUtilStatus=Get-Content "$n\build\actor-runtime-actor-state-util-20261011\results.json" -Raw|ConvertFrom-Json
if(@($actorStateUtilStatus|Where-Object ExitCode -ne 0).Count){throw 'Original ActorStateUtil provider compile failed'}
$providers+=' "'+$n+'\build\actor-runtime-actor-state-util-20261011\ActorStateUtil.obj" '

# Ground contact, geometry binding and posture snapshots are used by Mario's
# movement and actor-collision paths.
$movementHelperUnits=@('GroundChecker','GeometryBindUtil','PostureHolder')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-movement-helpers-20261011 -OnlyUnits $movementHelperUnits | Out-Null
$movementHelperStatus=Get-Content "$n\build\actor-runtime-movement-helpers-20261011\results.json" -Raw|ConvertFrom-Json
if(@($movementHelperStatus|Where-Object ExitCode -ne 0).Count){throw 'Original movement-helper provider compile failed'}
foreach($movementHelper in $movementHelperStatus){$providers+=' "'+$n+'\build\actor-runtime-movement-helpers-20261011\'+$movementHelper.Unit+'.obj" '}

# Demo control and actor-position camera helpers are part of original campaign
# cutscene execution and map-authored demo actors.
$demoRuntimeUnits=@('DemoCtrlBase','DemoPositionController')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-demo-controls-20261011 -OnlyUnits $demoRuntimeUnits | Out-Null
$demoRuntimeStatus=Get-Content "$n\build\actor-runtime-demo-controls-20261011\results.json" -Raw|ConvertFrom-Json
if(@($demoRuntimeStatus|Where-Object ExitCode -ne 0).Count){throw 'Original demo-control provider compile failed'}
foreach($demoRuntime in $demoRuntimeStatus){$providers+=' "'+$n+'\build\actor-runtime-demo-controls-20261011\'+$demoRuntime.Unit+'.obj" '}
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-demo-params-20261011 -OnlyUnits @('DemoParamCommonDataTable') | Out-Null
$demoParamStatus=Get-Content "$n\build\actor-runtime-demo-params-20261011\results.json" -Raw|ConvertFrom-Json
if(@($demoParamStatus|Where-Object ExitCode -ne 0).Count){throw 'Original demo-parameter table provider compile failed'}
$providers+=' "'+$n+'\build\actor-runtime-demo-params-20261011\DemoParamCommonDataTable.obj" '

# Gravity creator factories and their original shape-specific vtables are a
# missing dependency of stage-authored gravity objects; include the unchanged
# Petari implementation in the authoritative link audit.
$gravityUnits=@('GravityCreator','ConeGravity','CubeGravity','DiskGravity','DiskTorusGravity','ParallelGravity','SegmentGravity','WireGravity','GraviryFollower','GravityDust')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-gravity-providers -OnlyUnits $gravityUnits | Out-Null
$gravityStatus=Get-Content "$n\build\actor-runtime-gravity-providers\results.json" -Raw|ConvertFrom-Json
if(@($gravityStatus|Where-Object ExitCode -ne 0).Count){throw 'Original gravity provider compile failed'}
foreach($gravityProvider in $gravityStatus){$providers+=' "'+"$n\build\actor-runtime-gravity-providers\$($gravityProvider.Unit).obj"+'" '}

# The host input service already implements the game-facing KPAD queue and
# Windows keyboard/mouse sampling; include those exact providers in the link audit.
$inputProviderDir="$n\build\native-input-provider"
New-Item -ItemType Directory -Force -Path $inputProviderDir|Out-Null
$inputCommon=@('-std=c++17','-O2','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include",'-include','revolution/types.h','-include',"$n\include-runtime\revolution\os.h",'-include',"$n\include-runtime\revolution\gx\GXVert.h", "-I$p\Petari\libs\RVL_SDK\include")
foreach($inputUnit in @('native_kpad_input','native_keyboard_kpad','native_wpad_port')){
 & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @inputCommon -c "$n\src\$inputUnit.cpp" -o "$inputProviderDir\$inputUnit.obj"
 if($LASTEXITCODE -ne 0){throw "Native input provider compile failed: $inputUnit"}
 $providers+=' "'+"$inputProviderDir\$inputUnit.obj"+'" '
}

# Expose original JAudio DMA buffers through the shared Wii 32-bit address map
# and play their stereo PCM through Windows waveOut. DSP microcode execution is
# still a separate unresolved dependency; this provider does not synthesize it.
$providers+=' "'+$n+'\src\native_audio_ai.cpp" '

# Route Wii save APIs into the host save directory using the tested native
# NAND backend; include the Wii-format banner initializer used by save UI.
$nandProviderDir="$n\build\native-nand-provider"
New-Item -ItemType Directory -Force -Path $nandProviderDir|Out-Null
$nandCommon=@('-std=c++17','-O2','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include",'-include','revolution/types.h','-include',"$n\include-runtime\revolution\os.h",'-include',"$n\include-runtime\revolution\gx\GXVert.h",'-include',"$n\include-runtime\revolution\os\OSTime.h")
foreach($nandUnit in @('native_nand_port','native_nand_banner')){
 & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @nandCommon -c "$n\src\$nandUnit.cpp" -o "$nandProviderDir\$nandUnit.obj"
 if($LASTEXITCODE -ne 0){throw "Native NAND provider compile failed: $nandUnit"}
 $providers+=' "'+"$nandProviderDir\$nandUnit.obj"+'" '
}
$nwc24Provider="$nandProviderDir\native_nwc24_host.obj"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @nandCommon -c "$n\src\native_nwc24_host.cpp" -o $nwc24Provider
if($LASTEXITCODE -ne 0){throw 'Native NWC24 host adapter compile failed'}
$providers+=' "'+$nwc24Provider+'" '
$pal60Provider="$n\build\EncouragePal60Window.obj"
$pal60Common=@('-std=c++17','-O2','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\RVLFaceLib\include","-I$p\Petari\libs\nw4r\include",'-include',"$n\include-runtime\native_no_weak.h",'-include','revolution/types.h','-include',"$n\include-runtime\nw4r\ut\TagProcessorBase.h",'-include',"$n\include-runtime\nw4r\ut\Color.h")
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @pal60Common -c "$p\Petari\src\Game\Screen\EncouragePal60Window.cpp" -o $pal60Provider
if($LASTEXITCODE -ne 0){throw 'Original EncouragePal60Window provider failed to compile'}
$providers+=' "'+$pal60Provider+'" '
$jaiSoundChildProvider="$n\build\JAISoundChild.obj"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @pal60Common -c "$p\Petari\src\JSystem\JAudio2\JAISoundChild.cpp" -o $jaiSoundChildProvider
if($LASTEXITCODE -ne 0){throw 'Original JAISoundChild provider failed to compile'}
$providers+=' "'+$jaiSoundChildProvider+'" '
& "$n\build-jai-stream-manager.ps1"|Out-Null
if($LASTEXITCODE -ne 0){throw 'Original JAIStreamMgr host-adapted provider failed to compile'}
$providers+=' "'+"$n\build\actor-runtime-jai-stream-manager\JAIStreamMgr.obj"+'" '
$providers+=' "'+"$n\src\nw4r_panic_host.cpp"+'" '
$guidanceRun='actor-runtime-player-action-guidance-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $guidanceRun -OnlyUnits @('PlayerActionGuidance') | Out-Null
$guidanceStatus=Get-Content "$n\build\$guidanceRun\results.json" -Raw|ConvertFrom-Json
if(@($guidanceStatus|Where-Object ExitCode -ne 0).Count){throw 'Original PlayerActionGuidance provider failed to compile'}
$providers+=' "'+"$n\build\$guidanceRun\PlayerActionGuidance.obj"+'" '
$namePlateDrawerRun='actor-runtime-galaxy-nameplate-drawer-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $namePlateDrawerRun -OnlyUnits @('GalaxyNamePlateDrawer') | Out-Null
$namePlateDrawerStatus=Get-Content "$n\build\$namePlateDrawerRun\results.json" -Raw|ConvertFrom-Json
if(@($namePlateDrawerStatus|Where-Object ExitCode -ne 0).Count){throw 'Original GalaxyNamePlateDrawer provider failed to compile'}
$providers+=' "'+"$n\build\$namePlateDrawerRun\GalaxyNamePlateDrawer.obj"+'" '
$plantGrowthRun='actor-runtime-plant-growth-20261011'
$plantGrowthUnits=@('PlantStalk','PlantLeaf')
& "$n\probe-actor-runtime.ps1" -RunName $plantGrowthRun -OnlyUnits $plantGrowthUnits | Out-Null
$plantGrowthStatus=Get-Content "$n\build\$plantGrowthRun\results.json" -Raw|ConvertFrom-Json
if(@($plantGrowthStatus|Where-Object ExitCode -ne 0).Count){throw 'Original PlantStalk/PlantLeaf providers failed to compile'}
foreach($plantGrowth in $plantGrowthStatus){$providers+=' "'+"$n\build\$plantGrowthRun\$($plantGrowth.Unit).obj"+'" '}
$powerupActorRun='actor-runtime-mario-powerup-actors-20261011'
$powerupActorUnits=@('TornadoMario','FireMarioBall')
& "$n\probe-actor-runtime.ps1" -RunName $powerupActorRun -OnlyUnits $powerupActorUnits | Out-Null
$powerupActorStatus=Get-Content "$n\build\$powerupActorRun\results.json" -Raw|ConvertFrom-Json
if(@($powerupActorStatus|Where-Object ExitCode -ne 0).Count){throw 'Original TornadoMario/FireMarioBall providers failed to compile'}
foreach($powerupActor in $powerupActorStatus){$providers+=' "'+"$n\build\$powerupActorRun\$($powerupActor.Unit).obj"+'" '}
$playerRenderRun='actor-runtime-player-render-helpers-20261011'
$playerRenderUnits=@('JetTurtleShadow','MarineSnow')
& "$n\probe-actor-runtime.ps1" -RunName $playerRenderRun -OnlyUnits $playerRenderUnits | Out-Null
$playerRenderStatus=Get-Content "$n\build\$playerRenderRun\results.json" -Raw|ConvertFrom-Json
if(@($playerRenderStatus|Where-Object ExitCode -ne 0).Count){throw 'Original JetTurtleShadow/MarineSnow providers failed to compile'}
foreach($playerRender in $playerRenderStatus){$providers+=' "'+"$n\build\$playerRenderRun\$($playerRender.Unit).obj"+'" '}
$prologueBookRun='actor-runtime-prologue-book-and-letter-20261011'
$prologueBookUnits=@('ProloguePictureBook','PrologueLetter')
& "$n\probe-actor-runtime.ps1" -RunName $prologueBookRun -OnlyUnits $prologueBookUnits | Out-Null
$prologueBookStatus=Get-Content "$n\build\$prologueBookRun\results.json" -Raw|ConvertFrom-Json
if(@($prologueBookStatus|Where-Object ExitCode -ne 0).Count){throw 'Original ProloguePictureBook/PrologueLetter providers failed to compile'}
foreach($prologueBookProvider in $prologueBookStatus){$providers+=' "'+"$n\build\$prologueBookRun\$($prologueBookProvider.Unit).obj"+'" '}
$returnDemoRun='actor-runtime-return-demo-rail-move-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $returnDemoRun -OnlyUnits @('ReturnDemoRailMove') | Out-Null
$returnDemoStatus=Get-Content "$n\build\$returnDemoRun\results.json" -Raw|ConvertFrom-Json
if(@($returnDemoStatus|Where-Object ExitCode -ne 0).Count){throw 'Original ReturnDemoRailMove provider failed to compile'}
$providers+=' "'+"$n\build\$returnDemoRun\ReturnDemoRailMove.obj"+'" '
$surfRayTutorialRun='actor-runtime-surf-ray-tutorial-20261011'
$surfRayTutorialUnits=@('SurfRayTutorial','SurfingGuidance','PeachLetter')
& "$n\probe-actor-runtime.ps1" -RunName $surfRayTutorialRun -OnlyUnits $surfRayTutorialUnits | Out-Null
$surfRayTutorialStatus=Get-Content "$n\build\$surfRayTutorialRun\results.json" -Raw|ConvertFrom-Json
if(@($surfRayTutorialStatus|Where-Object ExitCode -ne 0).Count){throw 'Original SurfRayTutorial provider failed to compile'}
foreach($surfRayTutorialProvider in $surfRayTutorialStatus){$providers+=' "'+"$n\build\$surfRayTutorialRun\$($surfRayTutorialProvider.Unit).obj"+'" '}
$tamakoroRun='actor-runtime-tamakoro-20261011'
$tamakoroUnits=@('TamakoroTutorial','SphereAccelSensorController','SpherePadController','SphereController','AudTamakoroBgmCtrl')
& "$n\probe-actor-runtime.ps1" -RunName $tamakoroRun -OnlyUnits $tamakoroUnits | Out-Null
$tamakoroStatus=Get-Content "$n\build\$tamakoroRun\results.json" -Raw|ConvertFrom-Json
if(@($tamakoroStatus|Where-Object ExitCode -ne 0).Count){throw 'Original Tamakoro tutorial/controller/audio providers failed to compile'}
foreach($tamakoroProvider in $tamakoroStatus){$providers+=' "'+"$n\build\$tamakoroRun\$($tamakoroProvider.Unit).obj"+'" '}
$sledRopeRun='actor-runtime-sled-rope-point-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $sledRopeRun -OnlyUnits @('SledRopePoint') | Out-Null
$sledRopeStatus=Get-Content "$n\build\$sledRopeRun\results.json" -Raw|ConvertFrom-Json
if(@($sledRopeStatus|Where-Object ExitCode -ne 0).Count){throw 'Original SledRopePoint provider failed to compile'}
$providers+=' "'+"$n\build\$sledRopeRun\SledRopePoint.obj"+'" '
$scenarioTitleRun='actor-runtime-scenario-title-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $scenarioTitleRun -OnlyUnits @('ScenarioTitle') | Out-Null
$scenarioTitleStatus=Get-Content "$n\build\$scenarioTitleRun\results.json" -Raw|ConvertFrom-Json
if(@($scenarioTitleStatus|Where-Object ExitCode -ne 0).Count){throw 'Original ScenarioTitle provider failed to compile'}
$providers+=' "'+"$n\build\$scenarioTitleRun\ScenarioTitle.obj"+'" '
$pictureBookLayoutRun='actor-runtime-picture-book-layout-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $pictureBookLayoutRun -OnlyUnits @('PictureBookLayout') | Out-Null
$pictureBookLayoutStatus=Get-Content "$n\build\$pictureBookLayoutRun\results.json" -Raw|ConvertFrom-Json
if(@($pictureBookLayoutStatus|Where-Object ExitCode -ne 0).Count){throw 'Original PictureBookLayout provider failed to compile'}
$providers+=' "'+"$n\build\$pictureBookLayoutRun\PictureBookLayout.obj"+'" '
$pictureBookCloseButtonRun='actor-runtime-picture-book-close-button-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $pictureBookCloseButtonRun -OnlyUnits @('PictureBookCloseButton') | Out-Null
$pictureBookCloseButtonStatus=Get-Content "$n\build\$pictureBookCloseButtonRun\results.json" -Raw|ConvertFrom-Json
if(@($pictureBookCloseButtonStatus|Where-Object ExitCode -ne 0).Count){throw 'Original PictureBookCloseButton provider failed to compile'}
$providers+=' "'+"$n\build\$pictureBookCloseButtonRun\PictureBookCloseButton.obj"+'" '
$koopaBattlePlanetRun='actor-runtime-koopa-battle-map-planet-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $koopaBattlePlanetRun -OnlyUnits @('KoopaBattleMapPlanet') | Out-Null
$koopaBattlePlanetStatus=Get-Content "$n\build\$koopaBattlePlanetRun\results.json" -Raw|ConvertFrom-Json
if(@($koopaBattlePlanetStatus|Where-Object ExitCode -ne 0).Count){throw 'Original KoopaBattleMapPlanet provider failed to compile'}
$providers+=' "'+"$n\build\$koopaBattlePlanetRun\KoopaBattleMapPlanet.obj"+'" '
$koopaDamagePlateRun='actor-runtime-koopa-damage-plate-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $koopaDamagePlateRun -OnlyUnits @('KoopaBattleMapDamagePlate') | Out-Null
$koopaDamagePlateStatus=Get-Content "$n\build\$koopaDamagePlateRun\results.json" -Raw|ConvertFrom-Json
if(@($koopaDamagePlateStatus|Where-Object ExitCode -ne 0).Count){throw 'Original KoopaBattleMapDamagePlate provider failed to compile'}
$providers+=' "'+"$n\build\$koopaDamagePlateRun\KoopaBattleMapDamagePlate.obj"+'" '
$mapEffectRun='actor-runtime-spotmark-spinpull-20261011'
$mapEffectUnits=@('SpotMarkLight','SpinPullParticleCallBack')
& "$n\probe-actor-runtime.ps1" -RunName $mapEffectRun -OnlyUnits $mapEffectUnits | Out-Null
$mapEffectStatus=Get-Content "$n\build\$mapEffectRun\results.json" -Raw|ConvertFrom-Json
if(@($mapEffectStatus|Where-Object ExitCode -ne 0).Count){throw 'Original SpotMarkLight/SpinPullParticleCallBack providers failed to compile'}
foreach($mapEffectProvider in $mapEffectStatus){$providers+=' "'+"$n\build\$mapEffectRun\$($mapEffectProvider.Unit).obj"+'" '}
$octahedronBezierRun='actor-runtime-octahedron-bezier-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $octahedronBezierRun -OnlyUnits @('OctahedronBezierSurface') | Out-Null
$octahedronBezierStatus=Get-Content "$n\build\$octahedronBezierRun\results.json" -Raw|ConvertFrom-Json
if(@($octahedronBezierStatus|Where-Object ExitCode -ne 0).Count){throw 'Original OctahedronBezierSurface provider failed to compile'}
$providers+=' "'+"$n\build\$octahedronBezierRun\OctahedronBezierSurface.obj"+'" '
$bezierSurfaceRun='actor-runtime-bezier-surface-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $bezierSurfaceRun -OnlyUnits @('BezierSurface') | Out-Null
$bezierSurfaceStatus=Get-Content "$n\build\$bezierSurfaceRun\results.json" -Raw|ConvertFrom-Json
if(@($bezierSurfaceStatus|Where-Object ExitCode -ne 0).Count){throw 'Original BezierSurface provider failed to compile'}
$providers+=' "'+"$n\build\$bezierSurfaceRun\BezierSurface.obj"+'" '
$rushEndInfoRun='actor-runtime-rush-end-info-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $rushEndInfoRun -OnlyUnits @('RushEndInfo') | Out-Null
$rushEndInfoStatus=Get-Content "$n\build\$rushEndInfoRun\results.json" -Raw|ConvertFrom-Json
if(@($rushEndInfoStatus|Where-Object ExitCode -ne 0).Count){throw 'Original RushEndInfo provider failed to compile'}
$providers+=' "'+"$n\build\$rushEndInfoRun\RushEndInfo.obj"+'" '
$spaceInnerRun='actor-runtime-space-inner-20261011'
$spaceInnerUnits=@('SpaceInner','MirrorReflectionModel')
& "$n\probe-actor-runtime.ps1" -RunName $spaceInnerRun -OnlyUnits $spaceInnerUnits | Out-Null
$spaceInnerStatus=Get-Content "$n\build\$spaceInnerRun\results.json" -Raw|ConvertFrom-Json
if(@($spaceInnerStatus|Where-Object ExitCode -ne 0).Count){throw 'Original SpaceInner/MirrorReflectionModel providers failed to compile'}
foreach($spaceInnerProvider in $spaceInnerStatus){$providers+=' "'+"$n\build\$spaceInnerRun\$($spaceInnerProvider.Unit).obj"+'" '}
$fileSelectRun='actor-runtime-file-select-20261011'
$fileSelectUnits=@('FileSelectCameraController','FileSelectSky','FileSelectItem','FileSelectIconID','FileSelectEffect','FileSelectInfo','FileSelectButton','BrosButton','Manual2P','MiiConfirmIcon','MiiSelect','FileSelectModel','FileSelectNumber','MiiSelectIcon')
& "$n\probe-actor-runtime.ps1" -RunName $fileSelectRun -OnlyUnits $fileSelectUnits | Out-Null
$fileSelectStatus=Get-Content "$n\build\$fileSelectRun\results.json" -Raw|ConvertFrom-Json
if(@($fileSelectStatus|Where-Object ExitCode -ne 0).Count){throw 'Original file-select/Mii providers failed to compile'}
foreach($fileSelectProvider in $fileSelectStatus){$providers+=' "'+"$n\build\$fileSelectRun\$($fileSelectProvider.Unit).obj"+'" '}
$stageResultInformerRun='actor-runtime-stage-result-informer-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $stageResultInformerRun -OnlyUnits @('StageResultInformer') | Out-Null
$stageResultInformerStatus=Get-Content "$n\build\$stageResultInformerRun\results.json" -Raw|ConvertFrom-Json
if(@($stageResultInformerStatus|Where-Object ExitCode -ne 0).Count){throw 'Original StageResultInformer provider failed to compile'}
$providers+=' "'+"$n\build\$stageResultInformerRun\StageResultInformer.obj"+'" '
$playerModelSupportRun='actor-runtime-player-model-support-20261011'
$playerModelSupportUnits=@('DrawAdaptor','ModelHolder','MatrixControl')
& "$n\probe-actor-runtime.ps1" -RunName $playerModelSupportRun -OnlyUnits $playerModelSupportUnits | Out-Null
$playerModelSupportStatus=Get-Content "$n\build\$playerModelSupportRun\results.json" -Raw|ConvertFrom-Json
if(@($playerModelSupportStatus|Where-Object ExitCode -ne 0).Count){throw 'Original player/model support providers failed to compile'}
foreach($playerModelSupportProvider in $playerModelSupportStatus){$providers+=' "'+"$n\build\$playerModelSupportRun\$($playerModelSupportProvider.Unit).obj"+'" '}
$kCollisionRun='actor-runtime-kcollision-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $kCollisionRun -OnlyUnits @('KCollision') | Out-Null
$kCollisionStatus=Get-Content "$n\build\$kCollisionRun\results.json" -Raw|ConvertFrom-Json
if(@($kCollisionStatus|Where-Object ExitCode -ne 0).Count){throw 'Original KCollision provider failed to compile'}
$providers+=' "'+"$n\build\$kCollisionRun\KCollision.obj"+'" '
$iconCometRun='actor-runtime-icon-comet-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $iconCometRun -OnlyUnits @('IconComet') | Out-Null
$iconCometStatus=Get-Content "$n\build\$iconCometRun\results.json" -Raw|ConvertFrom-Json
if(@($iconCometStatus|Where-Object ExitCode -ne 0).Count){throw 'Original IconComet provider failed to compile'}
$providers+=' "'+"$n\build\$iconCometRun\IconComet.obj"+'" '
$quaternionSdkRun='actor-runtime-quaternion-sdk-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $quaternionSdkRun -OnlyUnits @('QuaternionSdkHelpers') | Out-Null
$quaternionSdkStatus=Get-Content "$n\build\$quaternionSdkRun\results.json" -Raw|ConvertFrom-Json
if(@($quaternionSdkStatus|Where-Object ExitCode -ne 0).Count){throw 'Original SDK quaternion providers failed to compile'}
$providers+=' "'+"$n\build\$quaternionSdkRun\QuaternionSdkHelpers.obj"+'" '
$jointRumblerRun='actor-runtime-joint-rumbler-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $jointRumblerRun -OnlyUnits @('JointRumbler') | Out-Null
$jointRumblerStatus=Get-Content "$n\build\$jointRumblerRun\results.json" -Raw|ConvertFrom-Json
if(@($jointRumblerStatus|Where-Object ExitCode -ne 0).Count){throw 'Original JointRumbler provider failed to compile'}
$providers+=' "'+"$n\build\$jointRumblerRun\JointRumbler.obj"+'" '

$providers+=' "$n\src\generated\AnimationSoundObjectKernel.cpp" "$n\src\generated\SoundHandlesKernel.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JAUSoundAnimator.cpp" '
$providers+=' "$p\Petari\src\Game\AudioLib\AudSoundObject.cpp" "$n\src\generated\JAUSoundObjectKernel.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JAISound.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JAISoundParams.cpp" '
$providers+=' "$n\src\generated\SoundObjectHolderKernel.cpp" '
$providers+=' "$n\src\generated\SequenceReaderKernel.cpp" '
$providers+=' "$n\src\generated\SequenceParserKernel.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASLfo.cpp" "$p\Petari\src\JSystem\JAudio2\JASCmdStack.cpp" "$n\src\generated\AudioDSPCommandKernel.cpp" "$n\src\generated\AudioDSPTaskWorkKernel.cpp" "$n\src\generated\AudioDSPCompletionKernel.cpp" "$n\src\generated\AudioDSPSendKernel.cpp" "$n\src\generated\AudioDSPDeferredKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASChannel.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASOscillator.cpp" '
$providers+=' "$n\src\generated\AudioCalcKernel.cpp" '
$providers+=' "$n\src\generated\JASSimpleWaveBankKernel.cpp" "$n\src\generated\JASBasicWaveBankKernel.cpp" '
$providers+=' "$n\src\generated\JASWaveArcLoaderKernel.cpp" '
$providers+=' "$n\src\generated\AudioHeapKernel.cpp" '
$providers+=' "$n\src\generated\AudioTaskThreadKernel.cpp" "$n\src\generated\AudioDvdThreadKernel.cpp" '
$providers+=' "$n\src\generated\AudioSystemKernel.cpp" "$n\src\generated\GameInstrumentParamKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstSense.cpp" "$p\Petari\src\JSystem\JAudio2\JASInstRand.cpp" "$n\src\bank_chunk_port.cpp" "$n\src\generated\BankEffectorKernel.cpp" "$n\src\generated\BankInstrumentKernel.cpp" "$n\src\generated\DrumSetKernel.cpp" "$n\src\generated\BankDispatchKernel.cpp" "$n\src\generated\DvdAramRipperEntryKernel.cpp" "$n\src\generated\DvdAramRipperTransferKernel.cpp" "$n\src\generated\AramStreamKernel.cpp" "$n\src\jsu_stream_lifecycle_port.cpp" "$n\src\generated\WaveSystemParserKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JAUBankTable.cpp" "$n\src\generated\AudioSectionHeapKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JAUAudioArcLoader.cpp" "$n\src\generated\AudioArcInterpreterKernel.cpp" "$n\src\generated\AudioSceneKernel.cpp" "$p\Petari\src\Game\AudioLib\AudWrap.cpp" "$n\src\generated\SoundNameConverterKernel.cpp" "$n\src\generated\SoundTableKernel.cpp" "$n\src\generated\SoundInfoKernel.cpp" "$n\src\audio_report_port.cpp" "$n\src\generated\AudioProbeKernel.cpp" "$n\src\generated\AudioDacMixKernel.cpp" "$n\src\generated\AudioFrameKernel.cpp" "$n\src\generated\AudioDSPStateKernel.cpp" "$n\src\generated\AudioDSPChannelKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASDriverIF.cpp" "$p\Petari\src\JSystem\JAudio2\JASCallback.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASBank.cpp" "$n\src\generated\BasicBankKernel.cpp" "$n\src\generated\BasicInstrumentKernel.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASSeqCtrl.cpp" "$p\Petari\src\JSystem\JAudio2\JASTrackPort.cpp" "$p\Petari\src\JSystem\JAudio2\JASRegisterParam.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JAISeMgr.cpp" "$p\Petari\src\JSystem\JAudio2\JAISe.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASSoundParams.cpp" "$p\Petari\src\JSystem\JAudio2\JAISeqDataMgr.cpp" "$p\Petari\src\JSystem\JAudio2\JASTrack.cpp" '
$providers+=' "$p\Petari\src\JSystem\JAudio2\JASAudioReseter.cpp" "$n\src\generated\AudioThreadKernel.cpp" "$n\src\player_event_sequence_port.cpp" '
$providers=$providers.Replace(' "$n\src\generated\AudioDacMixKernel.cpp"','').Replace(' "$n\src\generated\AudioFrameKernel.cpp"','').Replace(' "$n\src\generated\AudioDSPStateKernel.cpp"','').Replace(' "$n\src\generated\AudioDSPCompletionKernel.cpp"','')
$providers+=' "$n\src\dsp_address_space.cpp" '
$providers+=' "$n\src\dsp_mailbox_port.cpp" '
$providers+=' "$n\src\dsp_audio_command_hle.cpp" '
$providers+=' "$n\src\dsp_rom_boot_hle.cpp" '
$providers+=' "$n\src\dsp_boot_hle.cpp" '
$providers+=' "$n\src\generated\DspUcodeImage.cpp" '
$providers+=' "$n\build\actor-runtime-probe\StageResourceLoader.obj" '
$providers+=' "$n\build\actor-runtime-audio-sequence-20261012\JASResArcLoader.obj" '
$providers+=' "$p\Petari\src\Game\Screen\FullScreenBlur.cpp" "$n\src\jut_texture_capture_port.cpp" '
$providers+=' "$n\src\native_audio_support_port.cpp" '
$providers+=' "$n\src\native_ghost_packet_port.cpp" '
$rflUnits=@('RFL_Controller','RFL_Database','RFL_DataUtility','RFL_DefaultDatabase','RFL_Format','RFL_HiddenDatabase','RFL_Icon','RFL_MakeRandomFace','RFL_MakeTex','RFL_MiddleDatabase','RFL_Model','RFL_NANDAccess','RFL_NANDLoader','RFL_System')
foreach($rflUnit in $rflUnits){$providers+=' "'+$n+'\build\rfl-original\obj-c\'+$rflUnit+'.obj"'}
$providers+=' "$n\build\rfl-original\mem_expHeap.obj" "$n\build\rfl-original\mem_heapCommon.obj" "$n\build\rfl-original\mem_list.obj" "$n\build\rfl-original\host_c_compat_port.obj" '
$providers+=' "$n\src\rfl_matrix_support_port.cpp" '
$providers+=' "$n\src\camera_animation_pointer_port.cpp" '
$explicitProviderUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach($providerMatch in [regex]::Matches($providers,'"([^"\r\n]+\.(?:cpp|obj))"')){
 $unitName=[IO.Path]::GetFileNameWithoutExtension($providerMatch.Groups[1].Value)
 if($unitName){[void]$explicitProviderUnits.Add($unitName)}
}
foreach($r in $results){
 if($explicitProviderUnits.Contains($r.Unit)){continue}
 # These host-adapted supplemental providers replace overlapping methods in
 # the original translation units; retain one owner for each implementation.
 if($r.Unit -eq 'GravityUtil'){continue}
 if($r.Unit -in @('MarioHolder','GameDataTemporaryInGalaxy')){continue}
 if($r.Unit -eq 'StageSwitch'){continue}
 if($r.Unit -eq 'StageResourceLoader'){continue}
  if($r.Unit -in @('Camera','CameraContext','CameraDirector','CameraHeightArrange','CameraLocalUtil','CameraMan','CameraManPause','CameraManSubjective','CameraPoseParam','CameraRegisterHolder','CameraShakePatternImpl','CameraShakeTask','CameraSubjective','CameraTargetArg','CameraTargetHolder','CameraTargetMtx','CameraTargetObj','CameraPolygonCodeUtil','CameraShaker','CamHeliEffector','CamKarikariEffector')){continue}
 if($r.ExitCode){continue}
 if($r.Unit -in @('ObjUtil','LiveActorUtil','DrawUtil')){$providers+=' "'+"$n\src\generated\$($r.Unit).cpp"+'" ';continue}
 if($r.Unit -eq 'DemoUtil'){$providers+=' "$n\build\actor-runtime-probe\DemoUtil.obj" ';continue}
if($r.Unit -eq 'PlayerUtil'){$providers+=' "'+"$n\src\generated\PlayerUtilRuntime.cpp"+'" ';continue}
 if($r.Unit -eq 'WPadRumble'){continue}
 $providers+=' "'+"$n\build\actor-runtime-probe\$($r.Unit).obj"+'" '
}
# Add successful original NPC providers from the focused campaign-actor sweep.
# Keep single-definition ownership: shared NPC services already in the primary
# probe, plus the four event providers passed explicitly above, stay there.
$npcResultsPath="$n\build\actor-runtime-npc-probe\results.json"
$npcAdded=@()
if(Test-Path -LiteralPath $npcResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitNpcUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($unit in @('StageStateKeeper','EventDirector','CometEventKeeper','CometEventExecutorTimeLimit')){[void]$explicitNpcUnits.Add($unit)}
 $npcResults=Get-Content $npcResultsPath -Raw|ConvertFrom-Json
 foreach($r in $npcResults){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitNpcUnits.Contains($r.Unit)){continue}
  $providers+=' "'+"$n\build\actor-runtime-npc-probe\$($r.Unit).obj"+'" '
  $npcAdded+=,$r
 }
}
$areaResultsPath="$n\build\actor-runtime-area-probe\results.json"
$areaAdded=@()
if(Test-Path -LiteralPath $areaResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitAreaUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/\"]+)\.cpp"')){[void]$explicitAreaUnits.Add($providerMatch.Groups[1].Value)}
 $areaResults=Get-Content $areaResultsPath -Raw|ConvertFrom-Json
 foreach($r in $areaResults){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitAreaUnits.Contains($r.Unit)){continue}
  $providers+=' "'+"$n\build\actor-runtime-area-probe\$($r.Unit).obj"+'" '
  $areaAdded+=,$r
 }
}
$enemyResultsPath="$n\build\actor-runtime-enemy-probe\results.json"
$enemyAdded=@()
if(Test-Path -LiteralPath $enemyResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitEnemyUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/\"]+)\.cpp"')){[void]$explicitEnemyUnits.Add($providerMatch.Groups[1].Value)}
 $enemyResults=Get-Content $enemyResultsPath -Raw|ConvertFrom-Json
 foreach($r in $enemyResults){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitEnemyUnits.Contains($r.Unit)){continue}
  $providers+=' "'+"$n\build\actor-runtime-enemy-probe\$($r.Unit).obj"+'" '
  $enemyAdded+=,$r
 }
}
$bossResultsPath="$n\build\actor-runtime-boss-probe\results.json"
$bossAdded=@()
if(Test-Path -LiteralPath $bossResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitBossUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitBossUnits.Add($providerMatch.Groups[1].Value)}
 $bossResults=@(Get-Content $bossResultsPath -Raw|ConvertFrom-Json)
 foreach($overlayPath in @("$n\build\actor-runtime-boss-fix-probe\results.json","$n\build\actor-runtime-dodoryu-fix-probe\results.json")){
  if(!(Test-Path -LiteralPath $overlayPath)){continue}
  $overlay=Get-Content $overlayPath -Raw|ConvertFrom-Json
  foreach($fix in $overlay){
   for($i=0;$i -lt $bossResults.Count;$i++){if($bossResults[$i].Unit -eq $fix.Unit){$bossResults[$i]=$fix;break}}
  }
 }
 foreach($r in $bossResults){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitBossUnits.Contains($r.Unit)){continue}
  $bossObject=[System.IO.Path]::ChangeExtension($r.Log,'.obj')
  $providers+=' "'+$bossObject+'" '
  $bossAdded+=,$r
 }
}
$mapObjResultsPath="$n\build\actor-runtime-mapobj-final\results.json"
$mapObjAdded=@()
$mapObjResults=@()
if(Test-Path -LiteralPath $mapObjResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitMapObjUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitMapObjUnits.Add($providerMatch.Groups[1].Value)}
 $mapObjResults=@(Get-Content $mapObjResultsPath -Raw|ConvertFrom-Json)
 foreach($r in $mapObjResults){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitMapObjUnits.Contains($r.Unit)){continue}
  $mapObjObject=[System.IO.Path]::ChangeExtension($r.Log,'.obj')
  $providers+=' "'+$mapObjObject+'" '
  $mapObjAdded+=,$r
 }
}
$audParamsResultsPath="$n\build\actor-runtime-audparams-fix\results.json"
$audParamsAdded=@()
if(Test-Path -LiteralPath $audParamsResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 foreach($r in (Get-Content $audParamsResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $audParamsAdded+=,$r
 }
}
$multiEventCameraResultsPath="$n\build\actor-runtime-multievent-camera\results.json"
$multiEventCameraAdded=@()
if(Test-Path -LiteralPath $multiEventCameraResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitCameraUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitCameraUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $multiEventCameraResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitCameraUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $multiEventCameraAdded+=,$r
 }
}
$starPointerRunName='actor-runtime-pointer-effect-20261011'
& "$n\probe-actor-runtime.ps1" -RunName $starPointerRunName -OnlyUnits @('StarPointerBlur','StarPointerCommandStream') | Out-Null
$starPointerResultsPath="$n\build\$starPointerRunName\results.json"
$starPointerAdded=@()
if(Test-Path -LiteralPath $starPointerResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitPointerUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitPointerUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $starPointerResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitPointerUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $starPointerAdded+=,$r
 }
}
$screenHudRunName='actor-runtime-hud-20261011'
$screenHudUnits=@('MeterLayout','BombTimerLayout','SubMeterLayout','PlayerLeft','SuddenDeathMeter','FullnessMeter','MarioMeter','PlayerMissLeft')
& "$n\probe-actor-runtime.ps1" -RunName $screenHudRunName -OnlyUnits $screenHudUnits | Out-Null
$screenHudResultsPath="$n\build\$screenHudRunName\results.json"
$screenHudAdded=@()
if(Test-Path -LiteralPath $screenHudResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitScreenHudUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitScreenHudUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $screenHudResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitScreenHudUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $screenHudAdded+=,$r
 }
}
$koopaStairResultsPath="$n\build\actor-runtime-koopa-stair\results.json"
$koopaStairAdded=@()
if(Test-Path -LiteralPath $koopaStairResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitMapUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitMapUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $koopaStairResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitMapUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $koopaStairAdded+=,$r
 }
}
$mapRideResultsPath="$n\build\actor-runtime-map-ride-runtime-20261011\results.json"
$mapRideAdded=@()
if(Test-Path -LiteralPath $mapRideResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitMapRideUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitMapRideUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $mapRideResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitMapRideUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $mapRideAdded+=,$r
 }
}
$bootSystemResultsPath="$n\build\actor-runtime-boot-system-20261011\results.json"
$bootSystemAdded=@()
if(Test-Path -LiteralPath $bootSystemResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitBootUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitBootUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $bootSystemResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitBootUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $bootSystemAdded+=,$r
 }
}
$gameplayUtilityResultsPath="$n\build\actor-runtime-gravity-util-hostfix-20261011\results.json"
$gravityObjectResultsPath="$n\build\actor-runtime-global-gravity-20261011\results.json"
if(Test-Path -LiteralPath $gravityObjectResultsPath){
 $gravityObjectResults=@(Get-Content $gravityObjectResultsPath -Raw|ConvertFrom-Json)
 $gameplayRows=@(Get-Content $gameplayUtilityResultsPath -Raw|ConvertFrom-Json)
 foreach($row in $gravityObjectResults){if($row.ExitCode -eq 0 -and !($gameplayRows|Where-Object Unit -eq $row.Unit)){$gameplayRows+=,$row}}
 $gameplayRows|ConvertTo-Json|Set-Content $gameplayUtilityResultsPath
}
$gameplayUtilityAdded=@()
if(Test-Path -LiteralPath $gameplayUtilityResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitGameplayUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitGameplayUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $gameplayUtilityResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitGameplayUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $gameplayUtilityAdded+=,$r
 }
}
if($gameplayUtilityAdded.Count -gt 0){
 $providers=$providers.Replace(' "$n\src\generated\GravityRoutingKernel.cpp"','')
 $line=$line.Replace(' "$n\src\generated\GravityRoutingKernel.cpp"','')
}
$singlePlayerAudioResultsPath="$n\build\actor-runtime-singleplayer-audio-20261011\results.json"
$singlePlayerAudioAdded=@()
if(Test-Path -LiteralPath $singlePlayerAudioResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitAudioUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitAudioUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $singlePlayerAudioResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitAudioUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $singlePlayerAudioAdded+=,$r
 }
}
$audioSequenceUnits=@('AudBgm','AudBgmSetting','AudFader','AudAudience','AudSeStrategy','AudLimitedSound','AudSystemVolumeController','AudTrackController','AudSoundObject','AudSoundObject_Takezawa','AudSoundObject_Kawamura','AudSoundObject_Gohara','AudStageBgmWrap','AudStageBgmTable','AudRhythmWrap','AudRhythmMeSystem','AudBgmTempoAdjuster','AudChordInfo','AudMePlayer','JASAiCtrl','JASDSPInterface','JAUInitializer','JAUStreamFileTable','JAIStreamDataMgr','JASVoiceBank','JAUSeqDataBlockMgr','JAUSeqCollection','JASResArcLoader','JAUStdSoundInfo','JASAramStream','JAIAudible','JAIAudience','JAISeq','JAISeqMgr','JAIStream','JAIStreamMgr','JAISoundInfo','JAISoundStarter','JSUInputStream','JSUFileStream','JSUMemoryStream','TitleSequenceProduct')
& "$n\probe-actor-runtime.ps1" -RunName actor-runtime-audio-sequence-20261012 -OnlyUnits $audioSequenceUnits | Out-Null
$audioSequenceResultsPath="$n\build\actor-runtime-audio-sequence-20261012\results.json"
$audioSequenceAdded=@()
if(Test-Path -LiteralPath $audioSequenceResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitAudioSequenceUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitAudioSequenceUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $audioSequenceResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitAudioSequenceUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $audioSequenceAdded+=,$r
 }
}$collisionMovementResultsPath="$n\build\actor-runtime-collision-parabolic-20261011\results.json"
$collisionMovementAdded=@()
if(Test-Path -LiteralPath $collisionMovementResultsPath){
 $primaryUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($r in $results){[void]$primaryUnits.Add($r.Unit)}
 $explicitCollisionUnits=[System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
 foreach($providerMatch in [regex]::Matches($providers,'"[^"]+[\\/]([^\\/"]+)\.cpp"')){[void]$explicitCollisionUnits.Add($providerMatch.Groups[1].Value)}
 foreach($r in (Get-Content $collisionMovementResultsPath -Raw|ConvertFrom-Json)){
  if($r.ExitCode -ne 0 -or $primaryUnits.Contains($r.Unit) -or $explicitCollisionUnits.Contains($r.Unit)){continue}
  $providers+=' "'+[System.IO.Path]::ChangeExtension($r.Log,'.obj')+'" '
  $collisionMovementAdded+=,$r
 }
}
if($primaryUnits.Contains('CollisionParts') -or @($collisionMovementAdded|Where-Object Unit -eq 'CollisionParts').Count -gt 0){
 & "$n\generate-collision-query-no-parts.ps1"
 $providers=$providers.Replace(' "$n\src\generated\CollisionPartsQuery.cpp"',' "$n\src\generated\CollisionPartsQueryNoParts.cpp"')
 $line=$line.Replace(' "$n\src\generated\CollisionPartsQuery.cpp"',' "$n\src\generated\CollisionPartsQueryNoParts.cpp"')
 $providers=$providers.Replace(' "$n\src\generated\CollisionSceneKernel.cpp"',' "$n\src\generated\CollisionSceneKernelNoParts.cpp"')
 $line=$line.Replace(' "$n\src\generated\CollisionSceneKernel.cpp"',' "$n\src\generated\CollisionSceneKernelNoParts.cpp"')
 $line=$line.Replace(' "$n\src\generated\CollisionPartsQuery.cpp"','')
}
$dspBootBuild=Join-Path $env:TEMP 'galaxy-actor-runtime-dsp-boot'
New-Item -ItemType Directory -Force -Path $dspBootBuild | Out-Null
$dspBootCache=if($env:NATIVE_ZIG_CACHE_DIR){$env:NATIVE_ZIG_CACHE_DIR}else{Join-Path $env:TEMP 'galaxy-native-dsp-link-zig-cache'}
New-Item -ItemType Directory -Force -Path $dspBootCache | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=$dspBootCache
$dspBootUnits=@(
 @{Source="$n\src\dsp_boot_hle.cpp";Object=(Join-Path $dspBootBuild 'dsp_boot_hle.obj')},
 @{Source="$n\src\dsp_rom_boot_hle.cpp";Object=(Join-Path $dspBootBuild 'dsp_rom_boot_hle.obj')},
 @{Source="$n\src\generated\DspUcodeImage.cpp";Object=(Join-Path $dspBootBuild 'DspUcodeImage.obj')},
 @{Source="$n\src\native_game_entry_host.cpp";Object=(Join-Path $dspBootBuild 'native_game_entry_host.obj')}
)
$dspBootCommon=@('-std=c++17','-O2','-ffunction-sections','-fdata-sections','-target','x86_64-windows-gnu',"-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\nw4r\include",'-include','revolution/types.h','-c')
foreach($dspBootUnit in $dspBootUnits){
 & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ @dspBootCommon $dspBootUnit.Source '-o' $dspBootUnit.Object
 if($LASTEXITCODE){throw "Native DSP ROM boot provider failed to compile: $($dspBootUnit.Source)"}
 $providers+=' "'+$dspBootUnit.Object+'" '
}
$line=$line.Replace('c++ -std','c++ -shared -Wl,--export-all-symbols -I"$n\src" -DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER -std').Replace('src\original_kcl_server_tests.cpp','src\native_runtime_api.cpp').Replace(' -o ',$providers+' -lbcrypt -lole32 -lwindowscodecs -luser32 -lwinmm -o ').Replace('original-kcl-server-tests.exe','actor-runtime-link-probe.dll').Replace('original-kcl-server-tests.log','actor-runtime-link-probe.log')
$line=$line.Replace(' "$n\src\generated\KCollisionQueriesHost.cpp"','').Replace(' "$n\src\generated\CollisionBoundingBox.cpp"','')
$line=$line.Replace('-include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h"','-I"$n\build\camera-pointer-candidate\include" -include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" -include "$n\include-runtime\nw4r\ut\Color.h"')
$line=$line.Replace(' c++ ',' c++ -DGALAXY_ORIGINAL_GD_GEOMETRY -DGALAXY_SHARED_RUNTIME_MATH ')
$line=$line.Replace(' "$n\src\string_equal_case_port.cpp"','')
# Keep the growing original-runtime build below Windows command-line limits.
$expanded=$ExecutionContext.InvokeCommand.ExpandString($line)
$argumentsText=($expanded -split ' c\+\+ ',2)[1]
$argumentsText=($argumentsText -split '\s+\*>',2)[0]
$runTag=Get-Date -Format 'yyyyMMdd-HHmmss'
$response="$n\build\actor-runtime-link-$runTag.rsp"
$linkLogPath="$n\build\actor-runtime-link-probe-$runTag.log"
$statusPath="$n\build\actor-runtime-link-status-$runTag.json"
$responseEncoding=[System.Text.UTF8Encoding]::new($false)
[System.IO.File]::WriteAllText($response,$argumentsText,$responseEncoding)
$oldErrorActionPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ "@$response" *> $linkLogPath
$code=$LASTEXITCODE
$ErrorActionPreference=$oldErrorActionPreference
$successes=@($results|Where-Object ExitCode -eq 0);$failed=@($results|Where-Object ExitCode -ne 0)
$linkLog=Get-Content $linkLogPath
$undefined=@($linkLog|Where-Object {$_ -match 'undefined symbol:'})
$uniqueUndefined=@($undefined|ForEach-Object {if($_ -match 'undefined symbol: (.+)$'){$Matches[1]}}|Sort-Object -Unique)
$npcFailed=@($npcResults|Where-Object ExitCode -ne 0)
$areaFailed=@($areaResults|Where-Object ExitCode -ne 0)
$enemyFailed=@($enemyResults|Where-Object ExitCode -ne 0)
$bossFailed=@($bossResults|Where-Object ExitCode -ne 0)
$mapObjFailed=@($mapObjResults|Where-Object {$_.ExitCode -ne 0 -and $_.Unit -ne 'SpinDriverPathDrawer'})
$mapRideFailed=@((Get-Content $mapRideResultsPath -Raw|ConvertFrom-Json)|Where-Object ExitCode -ne 0)
$bootSystemFailed=@((Get-Content $bootSystemResultsPath -Raw|ConvertFrom-Json)|Where-Object ExitCode -ne 0)
$singlePlayerAudioFailed=@((Get-Content $singlePlayerAudioResultsPath -Raw|ConvertFrom-Json)|Where-Object ExitCode -ne 0)
$audioSequenceFailed=@((Get-Content $audioSequenceResultsPath -Raw|ConvertFrom-Json)|Where-Object ExitCode -ne 0)
$screenHudFailed=@((Get-Content $screenHudResultsPath -Raw|ConvertFrom-Json)|Where-Object ExitCode -ne 0)
$allSkipped=@($failed|Where-Object {$_.Unit -notin @('SpinDriverPathDrawer','JASResArcLoader')}|ForEach-Object Unit)+@($npcFailed|ForEach-Object Unit)+@($areaFailed|ForEach-Object Unit)+@($enemyFailed|ForEach-Object Unit)+@($bossFailed|ForEach-Object Unit)+@($mapObjFailed|ForEach-Object Unit)+@($mapRideFailed|ForEach-Object Unit)+@($bootSystemFailed|ForEach-Object Unit)+@($singlePlayerAudioFailed|ForEach-Object Unit)+@($audioSequenceFailed|ForEach-Object Unit)+@($screenHudFailed|ForEach-Object Unit)
$compiledProviderCount=$successes.Count+$npcAdded.Count+$areaAdded.Count+$enemyAdded.Count+$bossAdded.Count+$mapObjAdded.Count+$audParamsAdded.Count+$multiEventCameraAdded.Count+$starPointerAdded.Count+$screenHudAdded.Count+$koopaStairAdded.Count+$mapRideAdded.Count+$bootSystemAdded.Count+$singlePlayerAudioAdded.Count+$audioSequenceAdded.Count+$gameplayUtilityAdded.Count+$moviePlayerAdded+$waterPlantAdded+$jutExceptionAdded+$sunshadeAdded+$homeButtonHostAdded+$j2dPictureAdded+$drawUtilProjectionAdded+$stationedFileAdded+$gameEntryAdded+85
[pscustomobject]@{ExitCode=$code;Scope=('Original LiveActor integration with ' + $successes.Count + ' primary, ' + $npcAdded.Count + ' additional original NPC, ' + $areaAdded.Count + ' additional original AreaObj, ' + $enemyAdded.Count + ' additional original Enemy, ' + $bossAdded.Count + ' additional original Boss, ' + $mapObjAdded.Count + ' additional original MapObj, 1 host-adapted original SpinDriverPathDrawer, 1 generated model-fog provider, 2 original actor-lifecycle/animation providers, 1 host-ABI-adapted original JASResArcLoader provider, 1 original fullscreen blur and 1 native JUTTexture capture provider, 1 native audio-support provider, 1 host-native GhostPacket provider and 14 original RVLFaceLib/Mii providers and 3 original MEM heap/list providers, 1 host C runtime helper and 1 native Mii matrix provider, 1 original Luigi story provider, ' + $audParamsAdded.Count + ' original audio-configuration, ' + $multiEventCameraAdded.Count + ' original event-camera, ' + $starPointerAdded.Count + ' original pointer-UI, ' + $screenHudAdded.Count + ' original single-player HUD, 3 original movie-player plus 2 host THP support providers, ' + $koopaStairAdded.Count + ' original Koopa map, ' + $mapRideAdded.Count + ' additional original map/ride/demo/effect, ' + $bootSystemAdded.Count + ' original boot/system/controller UI, ' + $singlePlayerAudioAdded.Count + ' single-player audio, ' + $audioSequenceAdded.Count + ' title/audio-sequence, ' + $gameplayUtilityAdded.Count + ' original gameplay-utility, ' + $collisionMovementAdded.Count + ' collision/movement, 3 native KPAD/WPAD/Windows input, 2 host NAND/banner, 1 GX draw-sync callback, 1 native GX texture command, 1 native GX immediate vertex and 1 native GX display-list capture provider and 1 host default NW4R Pane constructor and 2 original movie-subtitle providers and 1 native DSP mailbox transport, 1 explicitly disabled host Wii NWC24/mail adapter, 1 original PAL60 advisory window, 1 original JAISoundChild and 1 host-adapted original JAIStreamMgr audio provider, 1 host-adapted AudMePlayer and 7 host speaker-system providers compiled/included; ' + $allSkipped.Count + ' source units skipped after compile failures; current collision runtime; link probe only, never executed');ProvidersCompiled=$compiledProviderCount;PrimaryProvidersCompiled=$successes.Count;SupplementalNpcProvidersCompiled=$npcAdded.Count;SupplementalAreaProvidersCompiled=$areaAdded.Count;SupplementalEnemyProvidersCompiled=$enemyAdded.Count;SupplementalBossProvidersCompiled=$bossAdded.Count;SupplementalMapObjProvidersCompiled=$mapObjAdded.Count;SupplementalSpinDriverPathProvidersCompiled=1;SupplementalModelFogProvidersCompiled=1;SupplementalActorAnimationProvidersCompiled=2;SupplementalAudioConfigProvidersCompiled=$audParamsAdded.Count;SupplementalEventCameraProvidersCompiled=$multiEventCameraAdded.Count;SupplementalPointerUiProvidersCompiled=$starPointerAdded.Count;SupplementalScreenHudProvidersCompiled=$screenHudAdded.Count;SupplementalMoviePlayerProvidersCompiled=$moviePlayerAdded;SupplementalKoopaMapProvidersCompiled=$koopaStairAdded.Count;SupplementalMapRideDemoEffectProvidersCompiled=$mapRideAdded.Count;SupplementalBootSystemProvidersCompiled=$bootSystemAdded.Count;SupplementalSinglePlayerAudioProvidersCompiled=$singlePlayerAudioAdded.Count;SupplementalAudioSequenceProvidersCompiled=($audioSequenceAdded.Count+1);SupplementalRenderCaptureProvidersCompiled=2;SupplementalAudioSupportProvidersCompiled=1;SupplementalGhostPacketProvidersCompiled=1;SupplementalRVLFaceLibProvidersCompiled=14;SupplementalMemHeapProvidersCompiled=3;SupplementalHostCCompatProvidersCompiled=1;SupplementalRflMatrixSupportProvidersCompiled=1;SupplementalGameplayUtilityProvidersCompiled=$gameplayUtilityAdded.Count;SupplementalCollisionMovementProvidersCompiled=$collisionMovementAdded.Count;SupplementalNativeInputProvidersCompiled=3;SupplementalNativeNandProvidersCompiled=2;SupplementalGxSyncProvidersCompiled=1;SupplementalGxTextureProvidersCompiled=1;SupplementalGxImmediateVertexProvidersCompiled=1;SupplementalGxDisplayListProvidersCompiled=1;SupplementalHostNwc24ProvidersCompiled=1;SupplementalPal60WindowProvidersCompiled=1;SupplementalJaiSoundChildProvidersCompiled=1;SupplementalJaiStreamMgrProvidersCompiled=1;ProvidersSkipped=$allSkipped;UndefinedOccurrences=$undefined.Count;UniqueUndefinedSymbols=$uniqueUndefined.Count;DuplicateDefinitions=@($linkLog|Where-Object {$_ -match 'duplicate symbol:'}).Count;Log=$linkLogPath}|ConvertTo-Json|Set-Content $statusPath
$linkStatusPath=$statusPath
$linkStatus=Get-Content $linkStatusPath -Raw|ConvertFrom-Json
$linkStatus.Scope += '; standalone Petari nativeOriginalGameMain object included/exported for host startup dependency audit (entry not executed)'
$linkStatus|Add-Member -NotePropertyName SupplementalNativeGameEntryProvidersCompiled -NotePropertyValue $gameEntryAdded -Force
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original actor-lifecycle/animation providers, ','2 original actor-lifecycle/animation providers, 7 original gravity-shape providers, 1 original GravityCreator factory provider, and 2 gravity support providers, ')
$linkStatus.Scope=$linkStatus.Scope.Replace('and 2 gravity support providers, ','and 2 gravity support providers, 1 original-state host GX initializer, ')
$linkStatus.Scope=$linkStatus.Scope.Replace('original movie-player/THP-audio providers, ','original movie-player/THP-audio providers, 1 host-adapted original WaterPlant, ')
$linkStatus.Scope=$linkStatus.Scope.Replace('original movie-player/THP-audio providers, 1 host-adapted original WaterPlant, ','original movie-player/THP-audio providers, 1 host-adapted original WaterPlant, 1 host JKRAram factory, ')
$linkStatus|Add-Member -NotePropertyName SupplementalGravityShapeProvidersCompiled -NotePropertyValue 7 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalGravityCreatorProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalGravitySupportProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalHostGxInitProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalHostGxServiceProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalHostEfbReadbackProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalScreenHudProvidersCompiled -NotePropertyValue $screenHudAdded.Count -Force
$linkStatus|Add-Member -NotePropertyName SupplementalMoviePlayerProvidersCompiled -NotePropertyValue $moviePlayerAdded -Force
$linkStatus|Add-Member -NotePropertyName SupplementalWaterPlantProvidersCompiled -NotePropertyValue $waterPlantAdded -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original actor-lifecycle/animation providers, ','2 original actor-lifecycle/animation providers, 2 original J3D model-drawer providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalModelDrawerProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 5
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original J3D model-drawer providers, ','2 original J3D model-drawer providers, 5 original rail-graph providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalRailGraphProvidersCompiled -NotePropertyValue 5 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('5 original rail-graph providers, ','5 original rail-graph providers, 1 original ActorStateUtil provider, ')
$linkStatus|Add-Member -NotePropertyName SupplementalActorStateUtilProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 3
$linkStatus.Scope=$linkStatus.Scope.Replace('1 original ActorStateUtil provider, ','1 original ActorStateUtil provider, 3 original ground/geometry/posture movement helpers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalMovementHelperProvidersCompiled -NotePropertyValue 3 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('3 original ground/geometry/posture movement helpers, ','3 original ground/geometry/posture movement helpers, 2 original demo/camera controllers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalDemoRuntimeProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original demo/camera controllers, ','2 original demo/camera controllers, 1 original demo-parameter table, ')
$linkStatus|Add-Member -NotePropertyName SupplementalDemoParamProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('1 original demo-parameter table, ','1 original demo-parameter table, 1 original PlayerActionGuidance UI provider, ')
$linkStatus|Add-Member -NotePropertyName SupplementalPlayerActionGuidanceProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('1 original PlayerActionGuidance UI provider, ','1 original PlayerActionGuidance UI provider, 1 original JointRumbler actor provider, ')
$linkStatus|Add-Member -NotePropertyName SupplementalJointRumblerProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('1 original JointRumbler actor provider, ','1 original JointRumbler actor provider, 1 original GalaxyNamePlateDrawer provider, ')
$linkStatus|Add-Member -NotePropertyName SupplementalGalaxyNamePlateDrawerProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('1 original GalaxyNamePlateDrawer provider, ','1 original GalaxyNamePlateDrawer provider, 2 original PlantStalk/PlantLeaf growth-actor providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalPlantGrowthProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original PlantStalk/PlantLeaf growth-actor providers, ','2 original PlantStalk/PlantLeaf growth-actor providers, 2 original TornadoMario/FireMarioBall player ability providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalPowerupActorProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original TornadoMario/FireMarioBall player ability providers, ','2 original TornadoMario/FireMarioBall player ability providers, 2 original jet-turtle-shadow/marine-snow player render providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalPlayerRenderProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original jet-turtle-shadow/marine-snow player render providers, ','2 original jet-turtle-shadow/marine-snow player render providers, 2 original prologue book/letter UI providers, ')
$linkStatus|Add-Member -NotePropertyName SupplementalPrologueUiProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope=$linkStatus.Scope.Replace('2 original prologue book/letter UI providers, ','2 original prologue book/letter UI providers, 1 original return-star demo-rail provider, ')
$linkStatus|Add-Member -NotePropertyName SupplementalReturnDemoRailMoveProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 3
$linkStatus.Scope += ' 3 original surfing tutorial/guidance/letter providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalSurfRayTutorialProvidersCompiled -NotePropertyValue 3 -Force
$linkStatus.ProvidersCompiled += 5
$linkStatus.Scope += ' 5 original Tamakoro tutorial/controller/pad/audio providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalTamakoroProvidersCompiled -NotePropertyValue 5 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original SledRopePoint movement provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalSledRopePointProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original ScenarioTitle campaign-UI provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalScenarioTitleProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original PictureBookLayout story provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalPictureBookLayoutProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original PictureBookCloseButton story-UI provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalPictureBookCloseButtonProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original KoopaBattleMapPlanet boss-progression provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalKoopaBattleMapPlanetProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original KoopaBattleMapDamagePlate provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalKoopaBattleMapDamagePlateProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope += ' 2 original map-mark/effect callback providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalMapMarkEffectProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original OctahedronBezierSurface geometry provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalOctahedronBezierProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original BezierSurface base-geometry provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalBezierSurfaceProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original RushEndInfo campaign-result provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalRushEndInfoProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 2
$linkStatus.Scope += ' 2 original SpaceInner/mirror-reflection world providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalSpaceInnerProvidersCompiled -NotePropertyValue 2 -Force
$linkStatus.ProvidersCompiled += 14
$linkStatus.Scope += ' 14 original file-select/Mii/manual-2P startup-UI providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalFileSelectProvidersCompiled -NotePropertyValue 14 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original StageResultInformer campaign-result UI provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalStageResultInformerProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 host Triangle collision-code query provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalCollisionCodeTriangleQueryProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 3
$linkStatus.Scope += ' 3 original player/model support providers,'
$linkStatus|Add-Member -NotePropertyName SupplementalPlayerModelSupportProvidersCompiled -NotePropertyValue 3 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 host-adapted original KCollision gameplay provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalKCollisionProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original campaign IconComet provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalIconCometProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 original SDK quaternion math provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalQuaternionSdkProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 1 native Windows AI DMA/PCM audio provider,'
$linkStatus|Add-Member -NotePropertyName SupplementalNativeAudioAiProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus.ProvidersCompiled += 3
$linkStatus.ProvidersCompiled += 1
$linkStatus.Scope += ' 3 native DSP ROM boot providers (DspBoot, DspRomBootHle and original DSP uCode image) and 1 native WBFS-bound game start export'
$linkStatus|Add-Member -NotePropertyName SupplementalDspRomBootProvidersCompiled -NotePropertyValue 3 -Force
$linkStatus|Add-Member -NotePropertyName SupplementalNativeGameStartHostProvidersCompiled -NotePropertyValue 1 -Force
$linkStatus|ConvertTo-Json -Depth 8|Set-Content $linkStatusPath
Get-Content $linkLogPath -Tail 18
exit $code






































































































































































































































































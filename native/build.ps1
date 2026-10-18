$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$compiler = Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$petari = Join-Path $root 'Petari'
$build = Join-Path $PSScriptRoot 'build'
$probeExe = if ($env:NATIVE_BUILD_EXE) { $env:NATIVE_BUILD_EXE } else { Join-Path $build 'galaxy-probe.exe' }
New-Item -ItemType Directory -Force -Path $build | Out-Null
$zigCache = if ($env:NATIVE_ZIG_CACHE_DIR) { $env:NATIVE_ZIG_CACHE_DIR } else { Join-Path $build 'zig-cache' }
New-Item -ItemType Directory -Force -Path $zigCache | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR = $zigCache
$sources = @('Game\LiveActor\Nerve.cpp','Game\LiveActor\Spine.cpp','Game\LiveActor\ActorStateKeeper.cpp','Game\LiveActor\ActorStateBase.cpp','Game\System\NerveExecutor.cpp','Game\NameObj\NameObj.cpp','Game\NameObj\NameObjRegister.cpp','Game\NameObj\NameObjAdaptor.cpp','Game\Map\SwitchWatcher.cpp','Game\Map\ActorAppearSwitchListener.cpp','Game\Util\SwitchEventFunctorListener.cpp','Game\LiveActor\HitSensor.cpp','Game\LiveActor\HitSensorInfo.cpp','Game\LiveActor\HitSensorKeeper.cpp','JSystem\JGadget\hashcode.cpp') | ForEach-Object { Join-Path "$petari\src" $_ }
& $compiler c++ -std=c++17 -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-nullability-completeness -target x86_64-windows-gnu "-I$PSScriptRoot\include" "-I$petari\include" "-I$petari\libs\JSystem\include" "$PSScriptRoot\src\main.cpp" "$PSScriptRoot\build\crash-trace.cpp" "$PSScriptRoot\src\os_mutex_port.cpp" "$PSScriptRoot\src\native_thread_identity_port.cpp" "$PSScriptRoot\src\sdk_matrix_port.cpp" "$PSScriptRoot\src\hash_sort_table_port.cpp" "$PSScriptRoot\src\jmap_port.cpp" "$PSScriptRoot\src\sensor_group_port.cpp" "$PSScriptRoot\src\sensor_hash_port.cpp" "$PSScriptRoot\src\sensor_checker_port.cpp" "$PSScriptRoot\src\sensor_routing_port.cpp" "$PSScriptRoot\src\sensor_messages_port.cpp" "$PSScriptRoot\src\live_actor_messages_port.cpp" "$PSScriptRoot\src\live_actor_nerve_port.cpp" "$PSScriptRoot\src\actor_timing_port.cpp" "$PSScriptRoot\src\actor_state_util_port.cpp" "$PSScriptRoot\src\coin_rewards_port.cpp" "$PSScriptRoot\src\player_status_port.cpp" "$PSScriptRoot\src\galaxy_progress_port.cpp" "$PSScriptRoot\src\stage_result_port.cpp" "$PSScriptRoot\src\stage_memory_port.cpp" "$PSScriptRoot\src\restart_cube_port.cpp" "$PSScriptRoot\src\area_volume_port.cpp" "$PSScriptRoot\src\death_area_port.cpp" "$PSScriptRoot\src\player_event_sequence_port.cpp" "$PSScriptRoot\src\player_death_event_port.cpp" "$PSScriptRoot\src\player_event_sequencer_init_port.cpp" "$PSScriptRoot\src\miss_layout_port.cpp" "$PSScriptRoot\src\wipe_koopa_port.cpp" "$PSScriptRoot\src\bck_settings_port.cpp" "$PSScriptRoot\src\switch_watcher_port.cpp" @sources -lbcrypt -ld3d11 -ld3dcompiler_47 -ldxgi -luser32 -ldbghelp -o "$probeExe"
if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
& "$probeExe" --self-test
if ($LASTEXITCODE -ne 0) { throw 'Native self-tests failed' }
& "$PSScriptRoot\test-dsp-rom-boot-hle.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP ROM boot protocol tests failed' }
& "$PSScriptRoot\test-dsp-rom-mailbox.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP ROM mailbox integration tests failed' }
& "$PSScriptRoot\test-dsp-boot-hle.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP boot HLE tests failed' }
& "$PSScriptRoot\test-dsp-audio-command-hle.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP command HLE validation tests failed' }
& "$PSScriptRoot\test-dsp-audio-command-hle-integration.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP command HLE integration tests failed' }
& "$PSScriptRoot\test-dsp-mailbox.ps1"
if ($LASTEXITCODE -ne 0) { throw 'DSP mailbox transport tests failed' }











































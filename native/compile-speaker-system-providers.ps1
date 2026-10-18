$ErrorActionPreference='Stop'
$PSNativeCommandUseErrorActionPreference=$false
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\zig-global-cache"
& "$n\generate-native-speaker-data.ps1"
& "$n\generate-native-speaker-wave.ps1"
& "$n\generate-native-chord-info.ps1"
$out="$n\build\actor-runtime-audio-host"
New-Item -ItemType Directory -Path $out -Force|Out-Null
$units=@(
    @{Name='SpkSystem';Path="$n\src\generated\SpkSystem.cpp"},
    @{Name='SpkMixingBuffer';Path="$n\src\generated\SpkMixingBuffer.cpp"},
    @{Name='SpkData';Path="$n\src\generated\SpkData.cpp"},
    @{Name='SpkTableConstructor';Path="$n\src\generated\SpkTableConstructor.cpp"},
    @{Name='SpkWave';Path="$n\src\generated\SpkWave.cpp"},
    @{Name='SpkSound';Path="$p\Petari\src\Game\Speaker\SpkSound.cpp"},
    @{Name='SpkSpeakerCtrlHost';Path="$n\src\spk_speaker_ctrl_host.cpp"},
    @{Name='AudMePlayer';Path="$n\src\generated\AudMePlayer.cpp"},
    @{Name='NativeMeParams';Path="$n\src\native_me_params.cpp"},
    @{Name='AudMeHandles';Path="$p\Petari\src\Game\RhythmLib\AudMeHandles.cpp"},
    @{Name='AudMeTrack';Path="$p\Petari\src\Game\RhythmLib\AudMeTrack.cpp"},
    @{Name='AudMeSeqCtrl';Path="$p\Petari\src\Game\RhythmLib\AudMeSeqCtrl.cpp"},
    @{Name='AudMeChannelMgr';Path="$p\Petari\src\Game\RhythmLib\AudMeChannelMgr.cpp"},
    @{Name='AudMeTrackCallback';Path="$p\Petari\src\Game\RhythmLib\AudMeTrackCallback.cpp"},
    @{Name='AudRhythmHolder';Path="$p\Petari\src\Game\RhythmLib\AudRhythmHolder.cpp"},
    @{Name='AudRhythmSeqParser';Path="$p\Petari\src\Game\RhythmLib\AudRhythmSeqParser.cpp"},
    @{Name='AudChordInfo';Path="$n\src\generated\AudChordInfo.cpp"},
    @{Name='NativeChordTable';Path="$n\src\native_chord_table.cpp"},
    @{Name='AudMeSeqReader';Path="$p\Petari\src\Game\RhythmLib\AudMeSeqReader.cpp"},
    @{Name='AudMeSeqParser';Path="$p\Petari\src\Game\RhythmLib\AudMeSeqParser.cpp"}
)
foreach($unit in $units){
    $previousErrorActionPreference=$ErrorActionPreference
    $ErrorActionPreference='Continue'
    try {
        & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections -Wno-nullability-completeness -Wno-nonportable-include-path -Wno-gcc-compat -Wno-deprecated-declarations -target x86_64-windows-gnu `
            "-I$n\include-runtime" "-I$n\include" "-I$n\build\camera-pointer-candidate\include" `
            "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" `
            -include revolution/types.h -c $unit.Path -o "$out\$($unit.Name).obj" *> "$out\$($unit.Name).log"
        $compileExit=$LASTEXITCODE
    } finally { $ErrorActionPreference=$previousErrorActionPreference }
    if($compileExit){Get-Content "$out\$($unit.Name).log" -Tail 20;exit $compileExit}
}
@($units|ForEach-Object { [pscustomobject]@{Unit=$_.Name;Object="$out\$($_.Name).obj";ExitCode=0} })|ConvertTo-Json|Set-Content "$out\results.json"
Write-Output "Compiled $($units.Count) native audio host providers separately for the full link audit."

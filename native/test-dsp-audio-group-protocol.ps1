$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\dsp-audio-group-protocol'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$hostSource=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioDSPInterfaceHost.cpp'))
$mapSource='#include <JSystem/JAudio2/JASDSPInterface.hpp>'+"`n"+'JASDsp::TChannel* JASDsp::CH_BUF;'+"`n"
# Retain these complete production bodies without pulling unrelated boot/heap/
# effects methods from the much larger interface unit into this focused fixture.
foreach($signature in @('u16 DSP_CreateMap2(u32 param_0) {','bool JASDsp::TChannel::isActive() const {')){
 $begin=$hostSource.IndexOf($signature)
 if($begin-lt 0){throw 'Production channel-map method missing'}
 $open=$hostSource.IndexOf('{',$begin);$depth=1;$end=$open+1
 while($end-lt $hostSource.Length-and $depth){if($hostSource[$end]-eq '{'){$depth++}elseif($hostSource[$end]-eq '}'){$depth--};$end++}
 if($depth){throw 'Production channel-map method boundary invalid'}
 $mapSource+=$hostSource.Substring($begin,$end-$begin)+"`n"
}
$mapPath=Join-Path $out 'OriginalChannelMap.cpp'
[IO.File]::WriteAllText($mapPath,$mapSource,[Text.UTF8Encoding]::new($false))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$exe=Join-Path $out 'group-protocol-tests.exe';$log=Join-Path $out 'compile.txt'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections '-Wl,--gc-sections' -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include math_types.hpp "$n\src\dsp_audio_group_protocol_tests.cpp" "$n\src\dsp_audio_command_hle.cpp" "$n\src\dsp_mailbox_port.cpp" "$n\src\dsp_address_space.cpp" "$n\src\dsp_rom_boot_hle.cpp" "$n\src\generated\DspUcodeImage.cpp" "$n\src\generated\AudioDSPCommandKernel.cpp" "$n\src\generated\AudioDSPSendKernel.cpp" "$n\src\generated\AudioDSPDeferredKernel.cpp" "$n\src\generated\AudioDSPTaskWorkKernel.cpp" $mapPath -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 25;exit 1}
& $exe *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE
Get-Content -LiteralPath (Join-Path $out 'run.txt')
exit $code

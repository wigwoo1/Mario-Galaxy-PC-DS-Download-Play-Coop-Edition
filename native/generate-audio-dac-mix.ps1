$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAiCtrl.cpp" -Raw
$names=@('setOutputRate','getDacRate','getSubFrames','getDacSize','getFrameSamples','getSubFrameCounter','mixMonoTrack','mixMonoTrackWide','mixExtraTrack','mixInterleaveTrack','readDspBuffer','registerMixCallback','registDSPBufCallback')
$out='#include <JSystem/JAudio2/JASAiCtrl.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/JASCalc.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/JASProbe.hpp>'+[Environment]::NewLine+'u32 JASDriver::sOutputRate; f32 JASDriver::sDacRate=32028.5f; u32 JASDriver::sSubFrames=7; volatile u32 JASDriver::sSubFrameCounter;'+[Environment]::NewLine
$out+='static u8 sDspDacBufferSize=3; s16** JASDriver::sDspDacBuffer; s32 JASDriver::sDspDacWriteBuffer; s32 JASDriver::sDspDacReadBuffer;'+[Environment]::NewLine
$out+='JASDriver::MixCallback JASDriver::extMixCallback; JASMixMode JASDriver::sMixMode=MIX_MODE_EXTRA; JASDriver::DSPBufCallback JASDriver::sDspDacCallback;'+[Environment]::NewLine
foreach($match in [regex]::Matches($source,'(?:void|f32|u32) JASDriver::(\w+)\(')){if($names -notcontains $match.Groups[1].Value){continue};$start=$match.Index;$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++};$out+=$source.Substring($start,$end-$start)+[Environment]::NewLine}
Set-Content "$n\src\generated\AudioDacMixKernel.cpp" $out



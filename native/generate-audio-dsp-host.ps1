$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function ReplaceOnce([string]$text,[string]$old,[string]$new,[string]$label){
 $count=([regex]::Matches($text,[regex]::Escape($old))).Count
 if($count -ne 1){throw "$label anchor count $count (expected 1)"}
 return $text.Replace($old,$new)
}

# Preserve the original provider bodies while replacing only Wii address
# truncation at the DSP boundary. The shared registry is consumed by the
# eventual host DSP implementation; these mappings alone do not execute DSP.
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAiCtrl.cpp" -Raw
$old='    JASDsp::syncFrame(getSubFrames(), u32(sDspDacBuffer[sDspDacWriteBuffer]), u32(sDspDacBuffer[sDspDacWriteBuffer] + r27));'
$new=@'
    s16* dspSamples = sDspDacBuffer[sDspDacWriteBuffer];
    const auto dspBegin = Native::nativeDspAddressSpace().map(dspSamples, getDacSize() * sizeof(s16));
    const auto dspEnd = Native::nativeDspAddressSpace().addressOf(dspSamples + r27, 0);
    if (!dspBegin || !dspEnd) {
        sDspStatus = 0;
        return;
    }
    JASDsp::syncFrame(getSubFrames(), dspBegin, dspEnd);
'@
$source=ReplaceOnce $source $old $new 'JAS DMA frame pointer mapping'
$source='#include <Native/DspAddressSpace.hpp>'+[Environment]::NewLine+$source
Set-Content "$n\src\generated\AudioAiCtrlHost.cpp" $source

$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDSPInterface.cpp" -Raw
$old='    DsetupTable(0x40, u32(CH_BUF), u32(&DSPRES_FILTER), u32(&DSPADPCM_FILTER), u32(FX_BUF));'
$new='    DsetupTable(0x40, Native::nativeDspAddressSpace().map(CH_BUF, sizeof(TChannel) * 64), Native::nativeDspAddressSpace().map(DSPRES_FILTER, sizeof(DSPRES_FILTER)), Native::nativeDspAddressSpace().map(DSPADPCM_FILTER, sizeof(DSPADPCM_FILTER)), Native::nativeDspAddressSpace().map(FX_BUF, sizeof(FxBuf) * 4));'
$source=ReplaceOnce $source $old $new 'JAudio DSP table pointer mapping'
$source='#include <Native/DspAddressSpace.hpp>'+[Environment]::NewLine+$source
Set-Content "$n\src\generated\AudioDSPInterfaceHost.cpp" $source

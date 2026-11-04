$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function ReplaceOnce([string]$text,[string]$old,[string]$new,[string]$label){
 $count=([regex]::Matches($text,[regex]::Escape($old))).Count
 if($count -ne 1){throw "$label anchor count $count (expected 1)"}
 return $text.Replace($old,$new)
}

# Preserve original providers with native pointer mapping and bounded bus setup.
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAiCtrl.cpp" -Raw
$old='void JASDriver::initAI(void (*param_0)(void)) {'
$new=@'
void JASDriver::initAI(void (*param_0)(void)) {
    // A previous worker's DAC buffer may belong to a released native heap.
    // Restart begins with the newly initialized DMA buffer, never that span.
    lastRspMadep = nullptr;
'@
$source=ReplaceOnce $source $old $new 'Native DAC initialization discards retired output pointer'
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
$old='    AIInitDMA((uintptr_t)sDmaDacBuffer[2], size);'
$new=@'
    const auto dmaAddress = Native::nativeDspAddressSpace().map(sDmaDacBuffer[2], size);
    if (!dmaAddress) throw std::runtime_error("Cannot map initial JAudio DMA buffer");
    AIInitDMA(dmaAddress, size);
'@
$source=ReplaceOnce $source $old $new 'JAudio initial AI DMA buffer mapping'
$old='        AIInitDMA((uintptr_t)r30, getDacSize() * 2);'
$new=@'
        const u32 dmaBytes = getDacSize() * sizeof(s16);
        const auto dmaAddress = Native::nativeDspAddressSpace().map(r30, dmaBytes);
        if (!dmaAddress) throw std::runtime_error("Cannot map rotated JAudio DMA buffer");
        AIInitDMA(dmaAddress, dmaBytes);
'@
$source=ReplaceOnce $source $old $new 'JAudio rotated AI DMA buffer mapping'
# This complete provider does not use the Wii assertion UI header. Avoid its
# platform-specific va-list declaration and preserve the original BOOL values.
$source=ReplaceOnce $source '#include "JSystem/JUtility/JUTAssert.hpp"' '' 'Unused Wii assertion UI include'
$source=ReplaceOnce $source 'isOutputRate = FALSE;' 'isOutputRate = 0;' '32k AI sample-rate mode'
$source=ReplaceOnce $source 'isOutputRate = TRUE;' 'isOutputRate = 1;' '48k AI sample-rate mode'
$source='#include <stdexcept>'+[Environment]::NewLine+'#include <Native/DspAddressSpace.hpp>'+[Environment]::NewLine+$source
Set-Content "$n\src\generated\AudioAiCtrlHost.cpp" $source

$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDSPInterface.cpp" -Raw
# The Wii boot-once flag outlives a stopped/released native renderer. Preserve
# idempotence while it is active, but perform the real handshake after stop.
$source=ReplaceOnce $source '    static bool sDspInit = true;' '' 'Native DSP boot lifetime flag'
$source=ReplaceOnce $source '    if (sDspInit) {' '    if (!nativeDspAudioRuntimeActive()) {' 'Native DSP active-runtime boot guard'
$source=ReplaceOnce $source '        sDspInit = false;' '' 'Native DSP obsolete one-time latch'
$old='    DsetupTable(0x40, u32(CH_BUF), u32(&DSPRES_FILTER), u32(&DSPADPCM_FILTER), u32(FX_BUF));'
$new='    DsetupTable(0x40, Native::nativeDspAddressSpace().map(CH_BUF, sizeof(TChannel) * 64), Native::nativeDspAddressSpace().map(DSPRES_FILTER, sizeof(DSPRES_FILTER)), Native::nativeDspAddressSpace().map(DSPADPCM_FILTER, sizeof(DSPADPCM_FILTER)), Native::nativeDspAddressSpace().map(FX_BUF, sizeof(FxBuf) * 4));'
$source=ReplaceOnce $source $old $new 'JAudio DSP table pointer mapping'
$old='    DCFlushRange(puVar3, sizeof(FxBuf));'
$new=@'
    // Register the complete native effects ring at its original configuration
    // boundary; a DSP consumer must resolve it instead of truncating _4.
    if (puVar3->_0 && !Native::nativeDspAddressSpace().map(puVar3->_4, std::size_t(puVar3->_2) * 0xA0)) {
        return 0;
    }
    DCFlushRange(puVar3, sizeof(FxBuf));
'@
$source=ReplaceOnce $source $old $new 'JAudio effects ring pointer mapping'
$old='void JASDsp::TChannel::setBusConnect(u8 param_0, u8 param_1) {'
$new=@'
void JASDsp::TChannel::setBusConnect(u8 param_0, u8 param_1) {
    if (param_0 >= 6) throw std::out_of_range("DSP mixer slot outside channel");
    // JASChannel's0xffff automatic-mixer sentinel has no explicit bus route.
    // Dolby/automatic mixing ignores this table. Never index it with0xff.
    if (param_1 == 0xff && _58) { _10[param_0][0] = 0; return; }
    if (param_1 >= 12) throw std::out_of_range("Unknown DSP mixer bus");
'@
$source=ReplaceOnce $source $old $new 'JAudio bus bounds and automatic-mixer sentinel'
$source='#include <stdexcept>'+[Environment]::NewLine+'#include <Native/DspAudioRuntime.hpp>'+[Environment]::NewLine+'#include <Native/DspAddressSpace.hpp>'+[Environment]::NewLine+$source
Set-Content "$n\src\generated\AudioDSPInterfaceHost.cpp" $source

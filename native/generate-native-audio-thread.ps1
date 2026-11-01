$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASAudioThread.cpp')).Replace("`r`n","`n")
function Replace-AudioThreadOnce([string]$text,[string]$old,[string]$new){
 if([regex]::Matches($text,[regex]::Escape($old)).Count-ne 1){throw 'Original audio-thread adaptation anchor changed'}
 return $text.Replace($old,$new)
}
$startup=@'
#include <stdint.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace {
std::mutex sAudioThreadStartupMutex;
std::condition_variable sAudioThreadStartupChanged;
bool sAudioThreadDspReady = false;
}
'@
$source=Replace-AudioThreadOnce $source '#include <stdint.h>' $startup
$source=Replace-AudioThreadOnce $source '    pAudioThread->resume();' @'
    {
        std::lock_guard<std::mutex> lock(sAudioThreadStartupMutex);
        sAudioThreadDspReady = false;
    }
    pAudioThread->resume();
    std::unique_lock<std::mutex> lock(sAudioThreadStartupMutex);
    if (!sAudioThreadStartupChanged.wait_for(lock, std::chrono::seconds(10), [] { return sAudioThreadDspReady; })) {
        throw std::runtime_error("JAS audio thread did not finish DSP startup");
    }
'@
$source=Replace-AudioThreadOnce $source '    JASDsp::boot(DSPCallback);' @'
    nativeConfigureDspAudioRuntime();
    JASDsp::boot(DSPCallback);
    if(!nativeDspAudioRuntimeActive())throw std::runtime_error("JAS audio thread has no bound native DSP renderer");
'@
$source=Replace-AudioThreadOnce $source '    JASDriver::startDMA();' @'
    JASDriver::startDMA();
    {
        std::lock_guard<std::mutex> lock(sAudioThreadStartupMutex);
        sAudioThreadDspReady = true;
    }
    sAudioThreadStartupChanged.notify_all();
'@
$source=Replace-AudioThreadOnce $source '        msg = waitMessageBlock();' @'
        nativeDspServiceHostAudio();
        if(nativeDspAudioRuntimeFailed())throw std::runtime_error("Native DSP audio rendering/completion failed");
        msg = waitMessageBlock();
'@
$source=Replace-AudioThreadOnce $source "        case AUDIOMSG_STOP:`n            JASDriver::stopDMA();" "        case AUDIOMSG_STOP:`n            JASDriver::stopDMA();`n            nativeStopDspAudioRuntime();"
$source="#include <Native/HeapPlacement.hpp>`n#include <Native/DspAudioRuntime.hpp>`n#include <Native/DspMailbox.hpp>`n"+$source
[IO.File]::WriteAllText((Join-Path $n 'src\generated\AudioThreadKernel.cpp'),$source,[Text.UTF8Encoding]::new($false))

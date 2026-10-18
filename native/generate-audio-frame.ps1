$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAiCtrl.cpp" -Raw
$start=$source.IndexOf('void JASDriver::updateDSP()');if($start -lt 0){throw 'Missing pinned original updateDSP'}
$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
$includes=$source.Substring(0,$source.IndexOf('static u8 sDspDacBufferSize'))
Set-Content "$n\src\generated\AudioFrameKernel.cpp" ($includes.Replace('#include "JSystem/JUtility/JUTAssert.hpp"','')+$source.Substring($start,$end-$start))

$source=Get-Content "$p\Petari\src\JSystem\JAudio2\dspproc.cpp" -Raw
Set-Content "$n\src\generated\AudioDSPCommandKernel.cpp" ($source.Replace('FALSE','0').Replace('TRUE','1'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\dsptask.cpp" -Raw
$start=$source.IndexOf('typedef struct');if($start -lt 0){throw 'Missing original DSP taskwork struct'}
Set-Content "$n\src\generated\AudioDSPTaskWorkKernel.cpp" ('#include <JSystem/JAudio2/dsptask.hpp>'+[Environment]::NewLine+'#include <revolution/os.h>'+[Environment]::NewLine+$source.Substring($start))
$start=$source.IndexOf('int DSPSendCommands2(');$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
$out='#include <JSystem/JAudio2/dsptask.hpp>'+[Environment]::NewLine+'#include <revolution/dsp.h>'+[Environment]::NewLine+'#include <revolution/os.h>'+[Environment]::NewLine+'int Dsp_Running_Check(); int DspStartWork(u32,void(*)(u16));'+[Environment]::NewLine
Set-Content "$n\src\generated\AudioDSPSendKernel.cpp" ($out+$source.Substring($start,$end-$start).Replace('TRUE','1').Replace('FALSE','0').Replace('s32 startWorkStatus;','s32 startWorkStatus=0;'))
$path="$n\src\generated\AudioDSPSendKernel.cpp"
$generated=Get-Content $path -Raw
$block='    if (pCallback != NULL) {'+[Environment]::NewLine+'        startWorkStatus = DspStartWork(pMessages[0], pCallback);'+[Environment]::NewLine+'    }'
if(!$generated.Contains($block)){throw 'Missing original callback registration block'}
$generated=$generated.Replace($block,'')
$guard='    if (pCallback != NULL) {'+[Environment]::NewLine+'        startWorkStatus = DspStartWork(pMessages[0], pCallback);'+[Environment]::NewLine+'        if (!startWorkStatus) { OSRestoreInterrupts(interruptFlag); return -1; }'+[Environment]::NewLine+'    }'+[Environment]::NewLine
Set-Content $path ($generated.Replace('    DSPSendMailToDSP(param_2);',$guard+'    DSPSendMailToDSP(param_2);'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\osdsp_task.cpp" -Raw
$start=$source.IndexOf('static u32 sync_stack[5];');if($start -lt 0){throw 'Missing original deferred frame state'}
$out='#include <JSystem/JAudio2/dspproc.hpp>'+[Environment]::NewLine+'#include <revolution/os.h>'+[Environment]::NewLine+'static volatile u8 DspRunningStatus; static u8 lbl_806B75B9;'+[Environment]::NewLine
Set-Content "$n\src\generated\AudioDSPDeferredKernel.cpp" ($out+$source.Substring($start).Replace('TRUE','1').Replace('FALSE','0'))

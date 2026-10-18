$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDspInterface.cpp" -Raw
$names=@('init','playStart','playStop','replyFinishRequest','forceStop','isActive','isFinish','setWaveInfo','setOscInfo','initAutoMixer','setAutoMixer','setPitch','setPauseFlag','flush','initFilter','setFilterMode','setIIRFilterParam','setFIR8FilterParam','setDistFilter','setMixerInitVolume','setMixerVolume','setBusConnect')
$out='#include <JSystem/JAudio2/JASDSPInterface.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/JASWaveInfo.hpp>'+[Environment]::NewLine+'#include <revolution/os.h>'+[Environment]::NewLine
$out+='u32 JASWaveInfo::one=1;'+[Environment]::NewLine
$matches=[regex]::Matches($source,'(?:void|bool) JASDsp::TChannel::(\w+)\(')
foreach($match in $matches){if($names -notcontains $match.Groups[1].Value){continue};$start=$match.Index;$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++};$out+=$source.Substring($start,$end-$start)+[Environment]::NewLine}
$match=[regex]::Match($source,'void JASDsp::setFilterTable\(');$start=$match.Index;$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++};$out+=$source.Substring($start,$end-$start)
$out+='JASDsp::TChannel* JASDsp::CH_BUF; JASDsp::FxBuf* JASDsp::FX_BUF;'+[Environment]::NewLine
foreach($name in @('getDSPHandle','flushBuffer','invalChannelAll','setDSPMixerLevel','getDSPMixerLevel','setFXLine')){
 $match=[regex]::Match($source,'(?:JASDsp::TChannel\*|void|f32|int) JASDsp::'+$name+'\(');if(!$match.Success){throw "Missing original $name"};$start=$match.Index;$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++};$out+=$source.Substring($start,$end-$start)+[Environment]::NewLine
}
 $out='#include <JSystem/JAudio2/JASDspInterface.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/dspproc.hpp>'+[Environment]::NewLine+'f32 JASDsp::sDSPVolume;'+[Environment]::NewLine+$out
$out='#include <JSystem/JAudio2/JASCalc.hpp>'+[Environment]::NewLine+'#include <JSystem/JAudio2/JASCriticalSection.hpp>'+[Environment]::NewLine+$out
$out+='u16 JASDsp::SEND_TABLE[]={0x0D00,0x0D60,0x0DC8,0x0E28,0x0E88,0x0EE8,0x0CA0,0x0F40,0x0FA0,0x0B00,0x09A0,0};'+[Environment]::NewLine
$out=$out.Replace('FxBuf* puVar3 = FX_BUF + param_0;', 'if (!FX_BUF || param_0 >= 4 || (param_2 && (param_2->_2 >= 12 || param_2->_6 >= 12 || param_2->_C > 65535))) return 0;'+[Environment]::NewLine+'    FxBuf* puVar3 = FX_BUF + param_0;')
$out=$out.Replace('puVar3->_0 = param_2->_0;', 'puVar3->_0 = param_2 ? param_2->_0 : 0;')
Set-Content "$n\src\generated\AudioDSPStateKernel.cpp" $out
$header=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASDspInterface.hpp" -Raw
$header=$header.Replace('u16 _10[1][4];  // array size unknown','u16 _10[8][4];  // Eight rows occupy the original 0x10..0x50 region.').Replace('/* 0x018 */ u8 _18[0x050 - 0x018];','')
Set-Content "$n\include\JSystem\JAudio2\JASDspInterface.hpp" $header






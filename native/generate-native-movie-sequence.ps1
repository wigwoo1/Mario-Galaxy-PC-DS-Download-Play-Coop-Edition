$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
foreach($unit in @('MoviePlayingSequence','MovieSubtitles','LogoFader','MoviePlayerSimple','THPSimplePlayerWrapper')){
 $s=Get-Content "$p\Petari\src\Game\Screen\$unit.cpp" -Raw
 $a=$s.IndexOf('namespace {');if($a -ge 0){$s=$s.Insert($a,"#undef ATTRIBUTE_WEAK`r`n#define ATTRIBUTE_WEAK`r`n")}
 $s=$s.Replace('GXColor fillColor = {0, 0, 0, mRate * 255.0f};','GXColor fillColor = {0, 0, 0, static_cast<u8>(mRate * 255.0f)};')
 if($unit -eq 'THPSimplePlayerWrapper'){
  $s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s
  $s=$s.Replace('mReadBuffer[index].isValid == TRUE','mReadBuffer[index].isValid == true')
  $s='#include <Native/ThpEndian.hpp>'+[Environment]::NewLine+$s
  $s=$s.Replace('memcpy(&mHeader, WorkBuffer, sizeof(mHeader));','NativeThp::header(mHeader, WorkBuffer);')
  $start=$s.IndexOf('u32 THPSimplePlayerWrapper::calcNeedMemory()');$end=$s.IndexOf('bool THPSimplePlayerWrapper::setBuffer(', $start)
  if($start -lt 0 -or $end -lt 0){throw 'Missing THP allocation anchors'}
  $s=$s.Substring(0,$start)+'u32 THPSimplePlayerWrapper::calcNeedMemory(){return mOpen?NativeThp::workBytes(mHeader,mVideoInfo,mAudioExist!=0):0;}'+[Environment]::NewLine+$s.Substring($end)
  $s=$s.Replace('bool THPSimplePlayerWrapper::setBuffer(u8* pBuffer) {','bool THPSimplePlayerWrapper::setBuffer(u8* pBuffer) {'+[Environment]::NewLine+'    if(!pBuffer || !calcNeedMemory()) return false;')
  $s=$s.Replace('memcpy(&mFrameComp, WorkBuffer, sizeof(mFrameComp));','NativeThp::components(mFrameComp, WorkBuffer);')
  $s=$s.Replace('memcpy(&mVideoInfo, WorkBuffer, sizeof(mVideoInfo));','NativeThp::video(mVideoInfo, WorkBuffer);')
  $s=$s.Replace('memcpy(&mAudioInfo, WorkBuffer, sizeof(mAudioInfo));','NativeThp::audio(mAudioInfo, WorkBuffer);')
  $s=$s.Replace('*(u32*)mReadBuffer[mReadIndex].ptr','NativeThp::read32(mReadBuffer[mReadIndex].ptr)').Replace('*(s32*)mReadBuffer[mReadIndex].ptr','static_cast<s32>(NativeThp::read32(mReadBuffer[mReadIndex].ptr))')
  $s=$s.Replace('u32* compSize = (u32*)mReadBuffer[mNextDecodeIndex].ptr + 2;','const u8* compSize = mReadBuffer[mNextDecodeIndex].ptr + 8;').Replace('(*compSize)','NativeThp::read32(compSize)').Replace('ptr += *compSize;','ptr += NativeThp::read32(compSize);').Replace('compSize++;','compSize += 4;')
  $s='#include <Native/ThpAudio.hpp>'+[Environment]::NewLine+$s
  $s='#include <Native/ThpVideo.hpp>'+[Environment]::NewLine+$s
  $videoCasePattern='(?m)^([ \t]*)case 0:\r?\n([ \t]*)if \(!videoDecode\(ptr\)\) \{\r?\n([ \t]*)return 1;\r?\n([ \t]*)\}\r?\n([ \t]*)break;'
  if([regex]::Matches($s,$videoCasePattern).Count -ne 2){throw 'Expected both THP video component decode cases'}
  $nl=[Environment]::NewLine
  $videoCaseReplacement='$1case 0: {'+$nl+'$2NativeThp::VideoFrameScope videoFrame(ptr, NativeThp::read32(compSize), mVideoInfo.xSize, mVideoInfo.ySize);'+$nl+'$2if (!videoDecode(ptr)) {'+$nl+'$3return 1;'+$nl+'$4}'+$nl+'$5break;'+$nl+'$1}'
  $s=[regex]::Replace($s,$videoCasePattern,$videoCaseReplacement)
  $s=$s.Replace('u32 sample = THPAudioDecode(mAudioBuffer[mAudioDecodeIndex].buffer, ptr + NativeThp::read32(compSize) * audio, 0);','u32 sample = NativeThp::decodeAudio(mAudioBuffer[mAudioDecodeIndex].buffer, ptr + NativeThp::read32(compSize) * audio, NativeThp::read32(compSize), mHeader.audioMaxSamples, 0);')
 }
 Set-Content "$n\src\generated\$unit.cpp" $s
}






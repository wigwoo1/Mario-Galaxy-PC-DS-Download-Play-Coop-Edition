$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$vendor=Join-Path $n 'third_party\dolphin-zelda'
$pins=Get-Content -LiteralPath (Join-Path $vendor 'SOURCE-HASHES.json') -Raw | ConvertFrom-Json
foreach($pin in $pins){
 $path=Join-Path $vendor $pin.File
 if((Get-Item -LiteralPath $path).Length-ne $pin.Bytes-or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash-ne $pin.SHA256){throw "Pinned Dolphin source/license changed: $($pin.File)"}
}
$source=[IO.File]::ReadAllText((Join-Path $vendor 'Zelda.cpp'))
$header=[IO.File]::ReadAllText((Join-Path $vendor 'Zelda.h'))
function Replace-ZeldaOnce([string]$text,[string]$old,[string]$new){
 if([regex]::Matches($text,[regex]::Escape($old)).Count-ne 1){throw 'Pinned Zelda renderer adaptation anchor changed'}
 return $text.Replace($old,$new)
}
$begin=$header.IndexOf('class ZeldaAudioRenderer');$end=$header.IndexOf('class ZeldaUCode final',$begin)
if($begin-lt 0-or $end-le $begin){throw 'Renderer header boundaries changed'}
$header=$header.Substring($begin,$end-$begin)
$header=$header.Replace('Core::System& system','ZeldaAudioMemory& memory').Replace('Core::System& m_system','ZeldaAudioMemory& m_memory')
$header=Replace-ZeldaOnce $header '  void DoState(PointerWrap& p);' ''
$header=Replace-ZeldaOnce $header '  u32 m_flags;' '  u32 m_flags = 0x403; // Verified upstream SMG flags.'
$header=Replace-ZeldaOnce $header '  u32 m_vpb_base_addr;' '  u32 m_vpb_base_addr = 0;'
# SMG stores four effect windows as8 history +80 live samples in DSP DRAM.
# Buffer IDs ending in8 address the live part, not an alias of the window base.
$header=$header.Replace('std::array<s16, N>* dst, const std::array<s16, N>& src','s16* dst, const std::array<s16, N>& src').Replace('      (*dst)[i] +=','      dst[i] +=')
$header=Replace-ZeldaOnce $header '  typedef std::array<s16, 0x50> MixingBuffer;' "  typedef std::array<s16, 0x50> MixingBuffer;`n  typedef std::array<s16, 0x58> ReverbBuffer;"
foreach($name in @('front_left','front_right','unk0','unk1')){$header=Replace-ZeldaOnce $header "  MixingBuffer m_buf_${name}_reverb{};" "  ReverbBuffer m_buf_${name}_reverb{};"}
$header=Replace-ZeldaOnce $header '  MixingBuffer* BufferForID(u16 buffer_id);' "  s16* BufferForID(u16 buffer_id);`n  void PrepareSmgReverbBuffers();`n  void ApplySmgReverb(bool post_rendering);"
$notice="// SPDX-License-Identifier: GPL-2.0-or-later`n// Derived from Dolphin Zelda audio renderer.`n// Copyright 2008 Dolphin Emulator Project. See native/third_party/dolphin-zelda.`n"
$header=$notice+'#pragma once'+"`n"+'#include <Native/ZeldaAudioMemory.hpp>'+"`n"+'#include <algorithm>'+"`n"+'#include <array>'+"`n"+'namespace Native {'+"`n"+$header+"`n}`n"
[IO.File]::WriteAllText((Join-Path $n 'include\Native\ZeldaAudioRenderer.hpp'),$header,[Text.UTF8Encoding]::new($false))
$begin=$source.IndexOf('enum ZeldaUCodeFlag');$end=$source.IndexOf('static const std::map',$begin)
if($begin-lt 0-or $end-le $begin){throw 'Renderer flags boundary changed'}
$flags=$source.Substring($begin,$end-$begin)
$begin=$source.IndexOf('#define DEFINE_32BIT_ACCESSOR');$end=$source.IndexOf('void ZeldaAudioRenderer::DoState',$begin)
if($begin-lt 0-or $end-le $begin){throw 'Renderer implementation boundaries changed'}
$source=$source.Substring($begin,$end-$begin)
$source=$source.Replace('Core::System& system','ZeldaAudioMemory& memory').Replace('m_system(system)','m_memory(memory)').Replace('m_system.GetMemory()','m_memory')
$begin=$source.IndexOf('static u8* GetARAMPointerForRange(');$end=$source.IndexOf('template <typename T>',$begin)
if($begin-lt 0-or $end-le $begin){throw 'Sample address helper boundaries changed'}
$helper=@'
static u8* GetARAMPointerForRange(const ZeldaAudioMemory& memory,u32 base,u32 offset,size_t bytes) {
    return memory.GetSamplePointer(base,offset,bytes);
}

'@
$source=$source.Substring(0,$begin)+$helper+"`n"+$source.Substring($end)
$source=$source.Replace('m_system, m_aram_base_addr','m_memory, m_aram_base_addr').Replace('Common::FromBigEndian<T>','ZeldaAudioMemory::FromBigEndian<T>')
$old=@'
  union
  {
    BitField<0, 5, u16> variable_fir_filter_size;
    BitField<5, 1, u16> enable_biquad_filter;
  };
'@
$normalized=$source.Replace("`r`n","`n")
$normalized=Replace-ZeldaOnce $normalized $old.Replace("`r`n","`n") '  u16 filter_flags;'
$source=$normalized.Replace('vpb.variable_fir_filter_size','(vpb.filter_flags & 0x1F)').Replace('vpb.enable_biquad_filter','(vpb.filter_flags & 0x20)')
$source=Replace-ZeldaOnce $source 'void ZeldaAudioRenderer::AddVoice(u16 voice_id)' 'void ZeldaAudioRenderer::AddVoice(u16 voice_id)'
$source=Replace-ZeldaOnce $source "void ZeldaAudioRenderer::AddVoice(u16 voice_id)`n{" "void ZeldaAudioRenderer::AddVoice(u16 voice_id)`n{`n  if(voice_id>=64)throw std::out_of_range(`"SMG DSP voice index exceeds 64 channels`");"
$source=Replace-ZeldaOnce $source '    if (!rpb.enabled)' '    if(rpb.enabled && !rpb.circular_buffer_size)throw std::invalid_argument("Enabled DSP reverb has zero buffer size");'
# Keep the original disabled-RPB branch as well as the new invalid-size guard.
$source=$source.Replace('    if(rpb.enabled && !rpb.circular_buffer_size)throw std::invalid_argument("Enabled DSP reverb has zero buffer size");',"    if(rpb.enabled && !rpb.circular_buffer_size)throw std::invalid_argument(`"Enabled DSP reverb has zero buffer size`");`n    if (!rpb.enabled)")
$source=Replace-ZeldaOnce $source "void ZeldaAudioRenderer::FinalizeFrame()`n{" "void ZeldaAudioRenderer::FinalizeFrame()`n{`n  m_memory.CheckRange(m_output_lbuf_addr,0xA0);`n  m_memory.CheckRange(m_output_rbuf_addr,0xA0);"
$source=Replace-ZeldaOnce $source '  ApplyReverb(false);' "  if(m_flags==0x403)PrepareSmgReverbBuffers();`n  ApplyReverb(false);"
$source=Replace-ZeldaOnce $source "void ZeldaAudioRenderer::ApplyReverb(bool post_rendering)`n{" "void ZeldaAudioRenderer::ApplyReverb(bool post_rendering)`n{`n  if(m_flags==0x403){ApplySmgReverb(post_rendering);return;}"
$source=Replace-ZeldaOnce $source '  MixingBuffer* reverb_buffers[4] = {' '  ReverbBuffer* reverb_buffers[4] = {'
$source=Replace-ZeldaOnce $source '      MixingBuffer* buffer = reverb_buffers[rpb_idx];' '      ReverbBuffer* buffer = reverb_buffers[rpb_idx];'
$source=Replace-ZeldaOnce $source 'memory.CopyToEmuSwapped(mram_addr, buffer->data(), sizeof(*buffer));' 'memory.CopyToEmuSwapped(mram_addr, buffer->data(), 0x50*sizeof(s16));'
$source=$source.Replace('MixingBuffer* dest_buffer = BufferForID','s16* dest_buffer = BufferForID').Replace('MixingBuffer* dst_buffer = BufferForID','s16* dst_buffer = BufferForID').Replace('AddBuffersWithVolume(dest_buffer->data(),','AddBuffersWithVolume(dest_buffer,')
$source=Replace-ZeldaOnce $source 'ZeldaAudioRenderer::MixingBuffer* ZeldaAudioRenderer::BufferForID(u16 buffer_id)' 's16* ZeldaAudioRenderer::BufferForID(u16 buffer_id)'
$begin=$source.IndexOf('s16* ZeldaAudioRenderer::BufferForID(');$end=$source.IndexOf('void ZeldaAudioRenderer::ApplyLowPassFilter',$begin)
$buffer=$source.Substring($begin,$end-$begin)
$buffer=[regex]::Replace($buffer,'return &m_buf_([a-z0-9_]+);','return m_buf_$1.data();')
$buffer=Replace-ZeldaOnce $buffer "{`n  switch (buffer_id)" @'
{
  if(m_flags==0x403){
    switch(buffer_id){
    case 0x0DC8:return m_buf_unk0_reverb.data()+8;
    case 0x0E28:return m_buf_unk1_reverb.data()+8;
    case 0x0E88:return m_buf_front_left_reverb.data()+8;
    case 0x0EE8:return m_buf_front_right_reverb.data()+8;
    }
  }
  switch (buffer_id)
'@
$source=$source.Substring(0,$begin)+$buffer+$source.Substring($end)
$begin=$source.IndexOf('    struct'+"`n"+'    {'+$([char]10)+'      MixingBuffer* buffer;');$end=$source.IndexOf('    vpb.dolby_volume_current',$begin)
if($begin-lt 0-or $end-le $begin){throw 'Dolby buffer-view adaptation boundary missing'}
$dolby=$source.Substring($begin,$end-$begin).Replace('MixingBuffer* buffer;','s16* buffer;').Replace('buffer.buffer->size()','input_samples.size()')
$dolby=[regex]::Replace($dolby,'&m_buf_([a-z0-9_]+),','m_buf_$1.data(),')
$source=$source.Substring(0,$begin)+$dolby+$source.Substring($end)
$preamble=@'
#include <Native/ZeldaAudioRenderer.hpp>
#include <stdexcept>
#define STRICT_ZELDA_HLE 1
namespace Native {
template<class... Args>[[noreturn]] static void ZeldaUnsupported(const char* message,Args&&...){throw std::runtime_error(message);}
#define PanicAlertFmt(...) ZeldaUnsupported(__VA_ARGS__)
#define ERROR_LOG_FMT(category,...) ZeldaUnsupported(__VA_ARGS__)
'@
$smg=[IO.File]::ReadAllText((Join-Path $n 'src\dsp_smg_reverb.inc'))
$source=$notice+$preamble+"`n"+$flags+$source+"`n"+$smg+"`n} // namespace Native`n"
if($source.Contains('m_system')-or $source.Contains('Core::')-or $source.Contains('BitField<')){throw 'Unadapted renderer dependency remains'}
[IO.File]::WriteAllText((Join-Path $n 'src\generated\ZeldaAudioRenderer.cpp'),$source,[Text.UTF8Encoding]::new($false))

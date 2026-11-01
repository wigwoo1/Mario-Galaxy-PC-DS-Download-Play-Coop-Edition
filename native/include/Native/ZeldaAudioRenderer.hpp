// SPDX-License-Identifier: GPL-2.0-or-later
// Derived from Dolphin Zelda audio renderer.
// Copyright 2008 Dolphin Emulator Project. See native/third_party/dolphin-zelda.
#pragma once
#include <Native/ZeldaAudioMemory.hpp>
#include <algorithm>
#include <array>
namespace Native {
class ZeldaAudioRenderer
{
public:
  explicit ZeldaAudioRenderer(ZeldaAudioMemory& memory);
  ZeldaAudioRenderer(const ZeldaAudioRenderer&) = delete;
  ZeldaAudioRenderer(ZeldaAudioRenderer&&) = delete;
  ZeldaAudioRenderer& operator=(const ZeldaAudioRenderer&) = delete;
  ZeldaAudioRenderer& operator=(ZeldaAudioRenderer&&) = delete;
  ~ZeldaAudioRenderer();

  void PrepareFrame();
  void AddVoice(u16 voice_id);
  void FinalizeFrame();

  void SetFlags(u32 flags) { m_flags = flags; }
  void SetSineTable(std::array<s16, 0x80>&& sine_table) { m_sine_table = sine_table; }
  void SetConstPatterns(std::array<s16, 0x100>&& patterns) { m_const_patterns = patterns; }
  void SetResamplingCoeffs(std::array<s16, 0x100>&& coeffs) { m_resampling_coeffs = coeffs; }
  void SetAfcCoeffs(std::array<s16, 0x20>&& coeffs) { m_afc_coeffs = coeffs; }
  void SetVPBBaseAddress(u32 addr) { m_vpb_base_addr = addr; }
  void SetReverbPBBaseAddress(u32 addr) { m_reverb_pb_base_addr = addr; }
  void SetOutputVolume(u16 volume) { m_output_volume = volume; }
  void SetOutputLeftBufferAddr(u32 addr) { m_output_lbuf_addr = addr; }
  void SetOutputRightBufferAddr(u32 addr) { m_output_rbuf_addr = addr; }
  void SetARAMBaseAddr(u32 addr) { m_aram_base_addr = addr; }


private:
  struct VPB;

  // See Zelda.cpp for the list of possible flags.
  u32 m_flags = 0x403; // Verified upstream SMG flags.

  // Utility functions for audio operations.

  // Apply volume to a buffer. The volume is a fixed point integer, usually
  // 1.15 or 4.12 in the DAC UCode.
  template <size_t N, size_t B>
  static void ApplyVolumeInPlace(std::array<s16, N>* buf, u16 vol)
  {
    for (size_t i = 0; i < N; ++i)
    {
      s32 tmp = (u32)(*buf)[i] * (u32)vol;
      tmp >>= 16 - B;

      (*buf)[i] = (s16)std::clamp(tmp, -0x8000, 0x7FFF);
    }
  }
  template <size_t N>
  void ApplyVolumeInPlace_1_15(std::array<s16, N>* buf, u16 vol)
  {
    ApplyVolumeInPlace<N, 1>(buf, vol);
  }
  template <size_t N>
  void ApplyVolumeInPlace_4_12(std::array<s16, N>* buf, u16 vol)
  {
    ApplyVolumeInPlace<N, 4>(buf, vol);
  }

  // Mixes two buffers together while applying a volume to one of them. The
  // volume ramps up/down in N steps using the provided step delta value.
  //
  // Note: On a real GC, the stepping happens in 32 steps instead. But hey,
  // we can do better here with very low risk. Why not? :)
  template <size_t N>
  static s32 AddBuffersWithVolumeRamp(s16* dst, const std::array<s16, N>& src,
                                      s32 vol, s32 step)
  {
    if (!vol && !step)
      return vol;

    for (size_t i = 0; i < N; ++i)
    {
      dst[i] += ((vol >> 16) * src[i]) >> 16;
      vol += step;
    }

    return vol;
  }

  // Does not use std::array because it needs to be able to process partial
  // buffers. Volume is in 1.15 format.
  static void AddBuffersWithVolume(s16* dst, const s16* src, size_t count, u16 vol)
  {
    while (count--)
    {
      s32 vol_src = ((s32)*src++ * (s32)vol) >> 15;
      *dst++ += std::clamp(vol_src, -0x8000, 0x7FFF);
    }
  }

  // Whether the frame needs to be prepared or not.
  bool m_prepared = false;

  // MRAM addresses where output samples should be copied.
  u32 m_output_lbuf_addr = 0;
  u32 m_output_rbuf_addr = 0;

  // Output volume applied to buffers before being uploaded to RAM.
  u16 m_output_volume = 0;

  // Mixing buffers.
  typedef std::array<s16, 0x50> MixingBuffer;
  typedef std::array<s16, 0x58> ReverbBuffer;
  MixingBuffer m_buf_front_left{};
  MixingBuffer m_buf_front_right{};
  MixingBuffer m_buf_back_left{};
  MixingBuffer m_buf_back_right{};
  ReverbBuffer m_buf_front_left_reverb{};
  ReverbBuffer m_buf_front_right_reverb{};
  MixingBuffer m_buf_back_left_reverb{};
  MixingBuffer m_buf_back_right_reverb{};
  ReverbBuffer m_buf_unk0_reverb{};
  ReverbBuffer m_buf_unk1_reverb{};
  MixingBuffer m_buf_unk0{};
  MixingBuffer m_buf_unk1{};
  MixingBuffer m_buf_unk2{};

  // Maps a buffer "ID" (really, their address in the DSP DRAM...) to our
  // buffers. Returns nullptr if no match is found.
  s16* BufferForID(u16 buffer_id);
  void PrepareSmgReverbBuffers();
  void ApplySmgReverb(bool post_rendering);

  // Base address where VPBs are stored linearly in RAM.
  u32 m_vpb_base_addr = 0;
  void FetchVPB(u16 voice_id, VPB* vpb);
  void StoreVPB(u16 voice_id, VPB* vpb);

  // Sine table transferred from MRAM. Contains sin(x) values for x in
  // [0.0;pi/4] (sin(x) in [1.0;0.0]), in 1.15 fixed format.
  std::array<s16, 0x80> m_sine_table{};

  // Const patterns used for some voice samples source. 4 x 0x40 samples.
  std::array<s16, 0x100> m_const_patterns{};

  // Fills up a buffer with the input samples for a voice, represented by its
  // VPB.
  void LoadInputSamples(MixingBuffer* buffer, VPB* vpb);

  // Raw samples (pre-resampling) that need to be generated to result in 0x50
  // post-resampling input samples.
  static u16 NeededRawSamplesCount(const VPB& vpb);

  // Resamples raw samples to 0x50 input samples, using the resampling ratio
  // and current position information from the VPB.
  void Resample(VPB* vpb, const s16* src, MixingBuffer* dst);

  // Coefficients used for resampling.
  std::array<s16, 0x100> m_resampling_coeffs{};

  // On the Wii, base address of the MRAM or ExRAM region replacing ARAM.
  u32 m_aram_base_addr = 0;

  // Downloads PCM encoded samples from ARAM. Handles looping and other
  // parameters appropriately.
  template <typename T>
  void DownloadPCMSamplesFromARAM(s16* dst, VPB* vpb, u16 requested_samples_count);

  // Downloads AFC encoded samples from ARAM and decode them. Handles looping
  // and other parameters appropriately.
  void DownloadAFCSamplesFromARAM(s16* dst, VPB* vpb, u16 requested_samples_count);
  void DecodeAFC(VPB* vpb, s16* dst, size_t block_count);
  std::array<s16, 0x20> m_afc_coeffs{};

  // Downloads samples from MRAM while handling appropriate length / looping
  // behavior.
  void DownloadRawSamplesFromMRAM(s16* dst, VPB* vpb, u16 requested_samples_count);

  static void ApplyLowPassFilter(MixingBuffer* buf, VPB* vpb);
  static void ApplyBiquadFilter(MixingBuffer* buf, VPB* vpb);

  // Applies the reverb effect to Dolby mixed voices based on a set of
  // per-buffer parameters. Is called twice: once before frame rendering and
  // once after.
  void ApplyReverb(bool post_rendering);
  std::array<u16, 4> m_reverb_pb_frames_count{};
  std::array<s16, 8> m_buf_unk0_reverb_last8{};
  std::array<s16, 8> m_buf_unk1_reverb_last8{};
  std::array<s16, 8> m_buf_front_left_reverb_last8{};
  std::array<s16, 8> m_buf_front_right_reverb_last8{};
  u32 m_reverb_pb_base_addr = 0;

  ZeldaAudioMemory& m_memory;
};


}

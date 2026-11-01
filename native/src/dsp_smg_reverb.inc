// SPDX-License-Identifier: GPL-2.0-or-later
// SMG DSP DRAM effects windows, audited against jdsp HashEctor D643001F.
// See audit-smg-dsp-effects.ps1 and build/smg-dsp-effects-audit/RESULT.txt.
void ZeldaAudioRenderer::PrepareSmgReverbBuffers(){
    // 013D..0154 copy the last8 words (base+0x50) to the history prefix;
    // 0156..0168 clear80 words from base+8. Four windows, stride0x60.
    for(auto* buffer:{&m_buf_unk0_reverb,&m_buf_unk1_reverb,
                      &m_buf_front_left_reverb,&m_buf_front_right_reverb}){
        std::copy(buffer->begin()+0x50,buffer->end(),buffer->begin());
        std::fill(buffer->begin()+8,buffer->end(),0);
    }
}
void ZeldaAudioRenderer::ApplySmgReverb(bool post_rendering){
    if(!m_reverb_pb_base_addr)throw std::invalid_argument("SMG reverb parameters unavailable");
    ReverbBuffer* buffers[]={&m_buf_unk0_reverb,&m_buf_unk1_reverb,
                             &m_buf_front_left_reverb,&m_buf_front_right_reverb};
    for(unsigned line=0;line<4;++line){
        ReverbPB parameters{};
        m_memory.CopyFromEmuSwapped(reinterpret_cast<u16*>(&parameters),m_reverb_pb_base_addr+line*0x20,0x20);
        if(!parameters.enabled)continue;
        if(!parameters.circular_buffer_size)throw std::invalid_argument("Enabled SMG reverb has zero buffer size");
        auto& index=m_reverb_pb_frames_count[line];
        if(index>=parameters.circular_buffer_size)throw std::out_of_range("SMG reverb index exceeds reconfigured ring");
        const u64 address=u64(parameters.GetCircularBufferBase())+u64(index)*0xA0;
        if(address>0xFFFFFFFFu)throw std::out_of_range("SMG reverb ring address overflow");
        auto& buffer=*buffers[line];
        if(post_rendering){
            // 0231..023C DMA the first80 words from the window's base.
            m_memory.CopyToEmuSwapped(u32(address),buffer.data(),0xA0);
            index=(index+1)%parameters.circular_buffer_size;
            continue;
        }
        std::array<s16,0x50> samples{};
        m_memory.CopyFromEmuSwapped(samples.data(),u32(address),0xA0);
        // 0178..0182 adds the downloaded block at base+8 with gain0x7FFF.
        AddBuffersWithVolume(buffer.data()+8,samples.data(),0x50,0x7FFF);
        auto filter=[&]{
            for(unsigned i=0;i<0x50;++i){
                s64 value=0;
                for(unsigned tap=0;tap<8;++tap)value+=s32(buffer[i+tap])*s32(parameters.filter_coeffs[tap]);
                buffer[i]=s16(std::clamp<s64>(value>>15,-32768,32767));
            }
        };
        if(parameters.enabled&1)filter();
        // Destination IDs are DRAM addresses. In particular base+8 routes
        // into the live portion, preserving overlap/feedback and history.
        for(const auto& destination:parameters.dest){
            if(!destination.buffer_id)continue;
            auto* target=BufferForID(destination.buffer_id);
            if(!target)throw std::runtime_error("SMG reverb destination is unmapped");
            AddBuffersWithVolume(target,buffer.data(),0x50,destination.volume);
        }
        if(parameters.enabled&2)filter();
    }
}

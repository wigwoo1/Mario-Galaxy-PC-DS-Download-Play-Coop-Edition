#include <JSystem/JAudio2/JASCalc.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
f32 JASDsp::sDSPVolume;
#include <JSystem/JAudio2/JASDSPInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <revolution/os.h>
u32 JASWaveInfo::one=1;
void JASDsp::TChannel::init() {
    mPauseFlag = 0;
    mIsFinished = 0;
    mForcedStop = 0;
    mIsActive = 0;
    _58 = 0;
    _68 = 0;
    initFilter();
}
void JASDsp::TChannel::playStart() {
    _10C = 0;
    _60 = 0;
    _8 = 1;
    _66 = 0;
    int i;
    for (i = 0; i < 4; i++) {
        _78[i] = 0;
        _A8[i] = 0;
    }
    for (i = 0; i < 20; i++) {
        _80[i] = 0;
    }
    mIsActive = 1;
}
void JASDsp::TChannel::playStop() {
    mIsActive = 0;
}
void JASDsp::TChannel::replyFinishRequest() {
    mIsFinished = 0;
    mIsActive = 0;
}
void JASDsp::TChannel::forceStop() {
    mForcedStop = 1;
}
bool JASDsp::TChannel::isActive() const {
    return mIsActive != 0;
}
bool JASDsp::TChannel::isFinish() const {
    return mIsFinished != 0;
}
void JASDsp::TChannel::setWaveInfo(JASWaveInfo const& waveInfo, u32 param_1, u32 param_2) {
    int i;
    _118 = param_1;
    static const u8 COMP_BLOCKSAMPLES[8] = {0x10, 0x10, 0x01, 0x01, 0x01, 0x10, 0x10, 0x01};
    _64 = COMP_BLOCKSAMPLES[waveInfo.mFormat];
    static const u8 COMP_BLOCKBYTES[8] = {0x09, 0x05, 0x08, 0x10, 0x01, 0x01, 0x01, 0x01};
    _100 = COMP_BLOCKBYTES[waveInfo.mFormat];
    _68 = 0;
    if (_100 >= 4) {
        _11C = waveInfo.mSampleCount;
        _102 = waveInfo.mLoopFlags;
        if (_102 != 0) {
            if (param_2 == 1) {
                param_2 = waveInfo.mSampleLoopStart;
            }
            _110 = waveInfo.mSampleLoopStart;
            _114 = waveInfo.mSampleLoopEnd;
            _104 = waveInfo.mpLastSample;
            _106 = waveInfo.mpPenultSample;
        } else {
            _114 = _11C;
        }
        if (param_2 && _114 > param_2) {
            switch (waveInfo.mFormat) {
            case 0:
            case 1:
                _68 = param_2;
                _118 += param_2 * _100 >> 4;
                _110 -= param_2;
                _114 -= param_2;
                break;
            case 2:
            case 3:
                _68 = param_2;
                break;
            }
        }
        for (i = 0; i < 16; i++) {
            _B0[i] = 0;
        }
    }
}
void JASDsp::TChannel::setOscInfo(u32 param_0) {
    _118 = 0;
    _64 = 16;
    _100 = param_0;
}
void JASDsp::TChannel::initAutoMixer() {
    if (_58) {
        _54 = _56;
    } else {
        _54 = 0;
        _58 = 1;
    }
}
void JASDsp::TChannel::setAutoMixer(u16 param_0, u8 param_1, u8 param_2, u8 param_3, u8 param_4) {
    _50 = (param_1 << 8) | param_2;
    _52 = param_3 << 8 | param_3 << 1;
    _56 = param_0;
    _58 = 1;
}
void JASDsp::TChannel::setPitch(u16 pitch) {
    if (pitch >= 0x7fff) {
        pitch = 0x7fff;
    }
    mPitch = pitch;
}
void JASDsp::TChannel::setMixerInitVolume(u8 param_0, s16 param_1) {
    u16* tmp = _10[param_0];
    tmp[2] = param_1;
    tmp[1] = param_1;
    tmp[3] = 0;
}
void JASDsp::TChannel::setMixerVolume(u8 param_0, s16 param_1) {
    u16* tmp;
    if (mForcedStop == 0) {
        tmp = _10[param_0];
        tmp[1] = param_1;
        tmp[3] &= 0xff;
    }
}
void JASDsp::TChannel::setPauseFlag(u8 flag) {
    mPauseFlag = flag;
}
void JASDsp::TChannel::flush() {
    DCFlushRange(this, sizeof(*this));
}
void JASDsp::TChannel::initFilter() {
    u32 i;
    for (i = 0; i < 8; i++) {
        fir_filter_params[i] = 0;
    }
    fir_filter_params[0] = 0x7fff;
    for (i = 0; i < 8; i++) {
        iir_filter_params[i] = 0;
    }
    iir_filter_params[0] = 0x7fff;
    iir_filter_params[4] = 0;
}
void JASDsp::TChannel::setFilterMode(u16 param_0) {
    u8 r30 = param_0 & 0x20;
    u8 r31 = param_0 & 0x1f;
    if (r30) {
        if (r31 > 0x14) {
            r31 = 0x14;
        }
    } else {
        if (r31 > 0x18) {
            r31 = 0x18;
        }
    }
    mFilterMode = r30 + r31;
}
void JASDsp::TChannel::setIIRFilterParam(s16* param_0) {
    setFilterTable(iir_filter_params, param_0, 8);
}
void JASDsp::TChannel::setFIR8FilterParam(s16* param_0) {
    setFilterTable(fir_filter_params, param_0, 8);
}
void JASDsp::TChannel::setDistFilter(s16 param_0) {
    iir_filter_params[4] = param_0;
}
void JASDsp::TChannel::setBusConnect(u8 param_0, u8 param_1) {
    u16* tmp = _10[param_0];
    static u16 const connect_table[12] = {
        0x0000, 0x0D00, 0x0D60, 0x0DC0, 0x0E20, 0x0E80, 0x0EE0, 0x0CA0, 0x0F40, 0x0FA0, 0x0B00, 0x09A0,
    };
    u16 r30 = 0;
    r30 = connect_table[param_1];
    tmp[0] = r30;
}
void JASDsp::setFilterTable(s16* param_0, s16* param_1, u32 param_2) {
    for (int i = 0; i < param_2; i++) {
        *param_0++ = *param_1++;
    }
}JASDsp::TChannel* JASDsp::CH_BUF; JASDsp::FxBuf* JASDsp::FX_BUF;
JASDsp::TChannel* JASDsp::getDSPHandle(int param_0) {
    return CH_BUF + param_0;
}
void JASDsp::flushBuffer() {
    DCFlushRange(CH_BUF, sizeof(TChannel) * 64);
    DCFlushRange(FX_BUF, sizeof(FxBuf) * 4);
}
void JASDsp::invalChannelAll() {
    DCInvalidateRange(CH_BUF, sizeof(TChannel) * 64);
}
void JASDsp::setDSPMixerLevel(f32 dsp_level) {
    sDSPVolume = dsp_level;
    dsp_level *= 4.0f;
    DsetMixerLevel(dsp_level);
}
f32 JASDsp::getDSPMixerLevel() {
    return sDSPVolume;
}
int JASDsp::setFXLine(u8 param_0, s16* buffer, JASDsp::FxlineConfig_* param_2) {
    if (!FX_BUF || param_0 >= 4 || (param_2 && (param_2->_2 >= 12 || param_2->_6 >= 12 || param_2->_C > 65535))) return 0;
    FxBuf* puVar3 = FX_BUF + param_0;
    JASCriticalSection aJStack_20;
    puVar3->_0 = 0;
    if (param_2 != NULL) {
        puVar3->_A = param_2->_4;
        puVar3->_8 = SEND_TABLE[param_2->_2];
        puVar3->_E = param_2->_8;
        puVar3->_C = SEND_TABLE[param_2->_6];
        puVar3->_2 = param_2->_C;
        setFilterTable((s16*)puVar3->_10, param_2->_10, 8);
    }
    if (buffer != NULL && param_2 != NULL) {
        u32 bufsize = param_2->_C * 0xa0;
        puVar3->_4 = buffer;
        JASCalc::bzero(buffer, bufsize);
        DCFlushRange(buffer, bufsize);
    } else if (param_2 == NULL || buffer != NULL) {
        puVar3->_4 = buffer;
    }

    if (puVar3->_4 != NULL) {
        puVar3->_0 = param_2 ? param_2->_0 : 0;
    } else {
        puVar3->_0 = 0;
    }
    DCFlushRange(puVar3, sizeof(FxBuf));
    return 1;
}
u16 JASDsp::SEND_TABLE[]={0x0D00,0x0D60,0x0DC8,0x0E28,0x0E88,0x0EE8,0x0CA0,0x0F40,0x0FA0,0x0B00,0x09A0,0};


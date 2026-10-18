#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
#include <JSystem/JAudio2/JASProbe.hpp>
u32 JASDriver::sOutputRate; f32 JASDriver::sDacRate=32028.5f; u32 JASDriver::sSubFrames=7; volatile u32 JASDriver::sSubFrameCounter;
static u8 sDspDacBufferSize=3; s16** JASDriver::sDspDacBuffer; s32 JASDriver::sDspDacWriteBuffer; s32 JASDriver::sDspDacReadBuffer;
JASDriver::MixCallback JASDriver::extMixCallback; JASMixMode JASDriver::sMixMode=MIX_MODE_EXTRA; JASDriver::DSPBufCallback JASDriver::sDspDacCallback;
void JASDriver::setOutputRate(JASOutputRate param_0) {
    sOutputRate = param_0;
    if (param_0 == 0) {
        sSubFrames = 7;
        sDacRate = 32000.0f;
    } else {
        sSubFrames = 10;
        sDacRate = 48000.0f;
    }
    sDacRate *= 1.0008897f;
}
void JASDriver::readDspBuffer(s16* param_0, u32 param_1) {
    s32 nbuf = sDspDacReadBuffer + 1;
    if (nbuf == sDspDacBufferSize) {
        nbuf = 0;
    }
    if (nbuf == sDspDacWriteBuffer && sDspDacBufferSize >= 3) {
        s16 r25 = (s16)sDspDacBuffer[sDspDacReadBuffer][param_1 / 2 - 1];
        s16 r24 = (s16)sDspDacBuffer[sDspDacReadBuffer][param_1 - 1];
        for (int i = 0; i < param_1; i++) {
            sDspDacBuffer[sDspDacReadBuffer][i] = (s16)r25;
        }
        for (int i = param_1; i < param_1 * 2; i++) {
            sDspDacBuffer[sDspDacReadBuffer][i] = (s16)r24;
        }
    } else {
        sDspDacReadBuffer = nbuf;
        DCInvalidateRange(sDspDacBuffer[sDspDacReadBuffer], param_1 * 2 * 2);
    }
    s16* dacBuffer = sDspDacBuffer[sDspDacReadBuffer];
    s16* endDacBuffer = dacBuffer + param_1;
    JASCalc::imixcopy(endDacBuffer, dacBuffer, param_0, param_1);
}
void JASDriver::registerMixCallback(MixCallback param_0, JASMixMode param_1) {
    extMixCallback = param_0;
    sMixMode = param_1;
}
void JASDriver::registDSPBufCallback(DSPBufCallback cb) {
    sDspDacCallback = cb;
}
f32 JASDriver::getDacRate() {
    return sDacRate;
}
u32 JASDriver::getSubFrames() {
    return sSubFrames;
}
u32 JASDriver::getDacSize() {
    return sSubFrames * 0x50 * 2;
}
u32 JASDriver::getFrameSamples() {
    return sSubFrames * 0x50;
}
void JASDriver::mixMonoTrack(s16* buffer, u32 param_1, MixCallback callback) {
    JASProbe::start(5, "MONO-MIX");
    s16* r26 = callback(param_1);
    if (r26 == nullptr) {
        return;
    }
    JASProbe::stop(5);
    s16* pTrack = buffer;
    s16* r28 = r26;
    for (u32 i = param_1; i != 0; i--) {
        s32 src = pTrack[0];
        src += r28[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(src);
        pTrack++;
        src = pTrack[0];
        src += r28[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(src);
        pTrack++;
        r28++;
    }
}
void JASDriver::mixMonoTrackWide(s16* buffer, u32 param_1, MixCallback callback) {
    JASProbe::start(5, "MONO(W)-MIX");
    s16* r26 = callback(param_1);
    if (!r26) {
        return;
    }
    JASProbe::stop(5);
    s16* pTrack = buffer;
    s16* r28 = r26;
    for (u32 i = param_1; i != 0; i--) {
        s32 src = pTrack[0];
        src += r28[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(src);
        pTrack++;
        src = pTrack[0];
        src -= r28[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(src);
        pTrack++;
        r28++;
    }
}
void JASDriver::mixExtraTrack(s16* buffer, u32 param_1, MixCallback callback) {
    JASProbe::start(5, "DSPMIX");
    s16* r27 = callback(param_1);
    if (!r27) {
        return;
    }
    JASProbe::stop(5);
    JASProbe::start(6, "MIXING");
    s16* pTrack = buffer;
    s16* r29 = r27;
    s16* r28 = r27 + getFrameSamples();
    for (u32 i = param_1; i != 0; i--) {
        s32 r25 = pTrack[0] + r28[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(r25);
        pTrack++;
        r25 = pTrack[0] + r29[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(r25);
        pTrack++;
        r28++;
        r29++;
    }
    JASProbe::stop(6);
}
void JASDriver::mixInterleaveTrack(s16* buffer, u32 param_1, MixCallback callback) {
    s16* r31 = callback(param_1);
    if (!r31) {
        return;
    }
    s16* pTrack = buffer;
    s16* r30 = r31;
    for (u32 i = param_1 * 2; i != 0; i--) {
        s32 r26 = pTrack[0] + r30[0];
        pTrack[0] = JASCalc::clamp< s16, s32 >(r26);
        pTrack++;
        r30++;
    }
}
u32 JASDriver::getSubFrameCounter() {
    return sSubFrameCounter;
}


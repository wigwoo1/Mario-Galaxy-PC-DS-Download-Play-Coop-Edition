#include "Game/System/WPadRumble.hpp"
#include "Game/System/WPadRumbleData.hpp"
void RumbleChannel::clear() {
    _0 = nullptr;
    _8 = 0;
    _C = 0;
    _E = false;
    _4 = false;
    _10 = nullptr;
}

void RumbleChannel::update() {
    if (_0 == nullptr) {
        return;
    }

    if (_C >= _0->mFrame) {
        if (_4 == true) {
            _E = _0->mPattern[0];
            _C = 1;
        } else {
            clear();
        }
    } else {
        _E = _0->mPattern[_C];
        _C++;
    }
}

void RumbleChannel::setPattern(const void* pParam1, const RumblePattern& rParam2, u32 param3, bool param4) {
    _8 = param3;
    _C = 0;
    _0 = &rParam2;
    _10 = pParam1;
    _4 = param4;
}



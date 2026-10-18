#include <Game/LiveActor/EffectKeeper.hpp>
#include <Game/Effect/SyncBckEffectChecker.hpp>
void EffectKeeper::changeBck() {
    if (_20 != nullptr) {
        _20->reset();
    }
}

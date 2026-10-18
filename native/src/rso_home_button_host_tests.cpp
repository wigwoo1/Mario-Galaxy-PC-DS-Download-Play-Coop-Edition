#include <Game/System/HomeButtonMenuWrapper.hpp>
#include <cassert>

int main() {
    RSO::setupRsoHomeButtonMenu();
    RSO::HBMCreate(nullptr);
    RSO::HBMInit();
    RSO::HBMCalc(nullptr);
    assert(RSO::HBMGetSelectBtnNum() == HBM_SELECT_NULL);
    RSO::HBMDraw();
    RSO::HBMSetAdjustFlag(1);
    RSO::HBMStartBlackOut();
    assert(RSO::HBMGetSelectBtnNum() == HBM_SELECT_NULL);
    return 0;
}

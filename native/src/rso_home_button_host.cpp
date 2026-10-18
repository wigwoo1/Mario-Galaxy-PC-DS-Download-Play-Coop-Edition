#include <Game/System/HomeButtonMenuWrapper.hpp>

// The original Wii overlay is supplied as a separately linked RSO. On the
// desktop host, Escape is translated to the game's normal pause button, so
// the Wii system Home overlay is intentionally unavailable.
void RSO::setupRsoHomeButtonMenu() {}
void RSO::HBMCreate(const HBMDataInfo*) {}
void RSO::HBMInit() {}
void RSO::HBMCalc(const HBMControllerData*) {}
void RSO::HBMDraw() {}
HBMSelectBtnNum RSO::HBMGetSelectBtnNum() { return HBM_SELECT_NULL; }
void RSO::HBMSetAdjustFlag(int) {}
void RSO::HBMStartBlackOut() {}

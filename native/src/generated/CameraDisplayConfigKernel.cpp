#include <revolution/sc.h>
namespace MR {
bool isAspectRatioFlag16Per9();
    bool isAspectRatioFlag16Per9() {
        return SCGetAspectRatio() == SC_ASPECT_RATIO_16x9;
    }
    bool isScreen16Per9() {
        return isAspectRatioFlag16Per9();
    }
s32 getScreenWidth() {
        return isScreen16Per9() ? 832 : 608;
    }
}

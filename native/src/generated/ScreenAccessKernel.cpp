#include <Game/Util/ScreenUtil.hpp>
#include <Game/Util/SystemUtil.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/System/GameSystem.hpp>
#include <Game/System/GameSystemObjHolder.hpp>
#include <Game/Screen/CaptureScreenDirector.hpp>
namespace {
CaptureScreenDirector* getCaptureScreenDirector() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mObjHolder->mCaptureScreenDirector;
    }
}
namespace MR {
s32 getScreenWidth() {
        return isScreen16Per9() ? 832 : 608;
    }
void startToCaptureScreen(const char* pParam1) {
        ::getCaptureScreenDirector()->requestCaptureTiming(pParam1);
    }
void endToCaptureScreen(const char* pParam1) {
        ::getCaptureScreenDirector()->invalidateCaptureTiming(pParam1);
    }
void captureScreenIfAllow(const char* pParam1) {
        ::getCaptureScreenDirector()->captureIfAllow(pParam1);
    }
const ResTIMG* getScreenResTIMG() {
        return ::getCaptureScreenDirector()->getResTIMG();
    }
u8* getScreenTexImage() {
        return ::getCaptureScreenDirector()->getTexImage();
    }
}


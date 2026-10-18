#include "Game/System/GameSystem.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/GameSystemException.hpp"
#include "Game/System/FileRipper.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <nw4r/lyt/init.h>
#include <revolution.h>
#include <Native/GpuResources.hpp>
extern "C" void galaxyNativeBootTrace(const char* step);
extern "C" __declspec(dllexport) void nativeOriginalGameMain(void) {
    galaxyNativeBootTrace("entry: starting original startup sequence");
    Native::registerExecutableImageResources(reinterpret_cast<const void*>(&nativeOriginalGameMain));
    OSInitFastCast();
    galaxyNativeBootTrace("entry: OSInitFastCast complete");
    DVDInit();
    galaxyNativeBootTrace("entry: DVDInit complete");
    VIInit();
    galaxyNativeBootTrace("entry: VIInit complete");
    HeapMemoryWatcher::createRootHeap();
    galaxyNativeBootTrace("entry: root heap complete");
    OSInitMutex(&MR::MutexHolder< 0 >::sMutex);
    galaxyNativeBootTrace("entry: system mutex 0 complete");
    OSInitMutex(&MR::MutexHolder< 1 >::sMutex);
    galaxyNativeBootTrace("entry: system mutex 1 complete");
    OSInitMutex(&MR::MutexHolder< 2 >::sMutex);
    galaxyNativeBootTrace("entry: system mutex 2 complete");
    nw4r::lyt::LytInit();
    galaxyNativeBootTrace("entry: layout initialization complete");
    MR::setLayoutDefaultAllocator();
    galaxyNativeBootTrace("entry: layout allocator configured");
    SingletonHolder< HeapMemoryWatcher >::init();
    galaxyNativeBootTrace("entry: HeapMemoryWatcher singleton complete");
    SingletonHolder< HeapMemoryWatcher >::get()->setCurrentHeapToStationedHeap();
    FileRipper::setup(0x20000, MR::getStationedHeapNapa());
    GameSystemException::init();
    MR::initAcosTable();
    SingletonHolder< GameSystem >::init();
    galaxyNativeBootTrace("entry: GameSystem singleton complete");
    SingletonHolder< GameSystem >::get()->init();
    galaxyNativeBootTrace("entry: GameSystem init complete");

    GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();

    galaxyNativeBootTrace("entry: entering main frame loop");
    while (true) {
        pGameSystem->frameLoop();
    }
}


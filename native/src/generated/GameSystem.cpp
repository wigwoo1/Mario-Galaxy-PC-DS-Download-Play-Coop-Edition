#include "Game/System/GameSystem.hpp"
#include <Native/GameInput.hpp>
#include <Native/DisplayCopyBinding.hpp>
#include <Native/TextureCopyBinding.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GpuResources.hpp>
#include <Native/XfbResourceRegistry.hpp>
#include "native_d3d_game_host.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObjRegister.hpp"
#include "Game/Screen/HomeButtonLayout.hpp"
#include "Game/Screen/SystemWipeHolder.hpp"
#include "Game/System/AudSystemWrapper.hpp"
#include "Game/System/DrawSyncManager.hpp"
#include "Game/System/FileRipper.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemDimmingWatcher.hpp"
#include "Game/System/GameSystemErrorWatcher.hpp"
#include "Game/System/GameSystemException.hpp"
#include "Game/System/GameSystemFontHolder.hpp"
#include "Game/System/GameSystemFrameControl.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemResetAndPowerProcess.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/GameSystemStationedArchiveLoader.hpp"
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/System/HomeButtonStateNotifier.hpp"
#include "Game/System/MainLoopFramework.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/MutexHolder.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include <JSystem/JUtility/JUTXfb.hpp>
#include <nw4r/lyt/init.h>
#include <revolution.h>

#define GX_FIFO_SIZE 0x80000

#define INIT_AUDIO_KEY "オーディオ初期化"  // "Audio Initialization"

#if defined(GALAXY_BOOT_TRACE)
extern "C" void galaxyNativeBootTrace(const char* step);
#define GALAXY_GAME_TRACE(step) galaxyNativeBootTrace(step)
#else
#define GALAXY_GAME_TRACE(step) ((void)0)
#endif

namespace NrvGameSystem {
    NEW_NERVE(GameSystemInitializeAudio, GameSystem, InitializeAudio);
    NEW_NERVE(GameSystemInitializeLogoScene, GameSystem, InitializeLogoScene);
    NEW_NERVE(GameSystemLoadStationedArchive, GameSystem, LoadStationedArchive);
    NEW_NERVE(GameSystemWaitForReboot, GameSystem, WaitForReboot);
    NEW_NERVE(GameSystemNormal, GameSystem, Normal);
};  // namespace NrvGameSystem

extern "C" void galaxyNativeBootTrace(const char* step);
extern "C" void nativeOriginalGameMain(void) {
    galaxyNativeBootTrace("entry: starting GameSystem.cpp startup sequence");
    OSInitFastCast();
    galaxyNativeBootTrace("entry: GameSystem OSInitFastCast complete");
    DVDInit();
    galaxyNativeBootTrace("entry: GameSystem DVDInit complete");
    VIInit();
    galaxyNativeBootTrace("entry: GameSystem VIInit complete");
    HeapMemoryWatcher::createRootHeap();
    galaxyNativeBootTrace("entry: GameSystem root heap complete");
    OSInitMutex(&MR::MutexHolder< 0 >::sMutex);
    OSInitMutex(&MR::MutexHolder< 1 >::sMutex);
    OSInitMutex(&MR::MutexHolder< 2 >::sMutex);
    nw4r::lyt::LytInit();
    MR::setLayoutDefaultAllocator();
    SingletonHolder< HeapMemoryWatcher >::init();
    SingletonHolder< HeapMemoryWatcher >::get()->setCurrentHeapToStationedHeap();
    FileRipper::setup(0x20000, MR::getStationedHeapNapa());
    GameSystemException::init();
    MR::initAcosTable();
    SingletonHolder< GameSystem >::init();
    galaxyNativeBootTrace("entry: GameSystem singleton complete");
    SingletonHolder< GameSystem >::get()->init();
    galaxyNativeBootTrace("entry: GameSystem init complete");

    GameSystem* pGameSystem = SingletonHolder< GameSystem >::get();

    galaxyNativeBootTrace("entry: GameSystem entering frame loop");
    while (true) {
        pGameSystem->frameLoop();
    }
}

GameSystem::GameSystem()
    : NerveExecutor("GameSystem"), mFifoBase(nullptr), mSequenceDirector(nullptr), mErrorWatcher(nullptr), mFontHolder(nullptr),
      mFrameControl(nullptr), mObjHolder(nullptr), mSceneController(nullptr), mStationedArchiveLoader(nullptr), mHomeButtonLayout(nullptr),
      mSystemWipeHolder(nullptr), mHomeButtonStateNotifier(nullptr), mIsExecuteLoadSystemArchive(false) {
}

void GameSystem::init() {
    GALAXY_GAME_TRACE("GameSystem::init begin");
    JKRAram::create(0xE00000, 0xFFFFFFFF, 8, 7, 3);
    GALAXY_GAME_TRACE("GameSystem::init ARAM create complete");
    mObjHolder = new GameSystemObjHolder();
    GALAXY_GAME_TRACE("GameSystem::init ObjHolder allocated");
    mFontHolder = new GameSystemFontHolder();
    GALAXY_GAME_TRACE("GameSystem::init FontHolder allocated");
    mFontHolder->createFontFromEmbeddedData();
    GALAXY_GAME_TRACE("GameSystem::init embedded font loaded");
    initNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));
    mSequenceDirector = new GameSequenceDirector();
    GALAXY_GAME_TRACE("GameSystem::init SequenceDirector allocated");
    initGX();
    GALAXY_GAME_TRACE("GameSystem::init GX initialized");
    DrawSyncManager::start(0x300, 15);
    GALAXY_GAME_TRACE("GameSystem::init draw sync started");
    mSceneController = new GameSystemSceneController();
    GALAXY_GAME_TRACE("GameSystem::init SceneController allocated");
    mObjHolder->init();
    GALAXY_GAME_TRACE("GameSystem::init ObjHolder initialized");
    auto* nativeVideoManager = JUTVideo::getManager();
    if (!nativeVideoManager || !nativeVideoManager->getRenderMode())
        throw std::logic_error("Native game host requires an initialized video mode");
    const auto* nativeMode = nativeVideoManager->getRenderMode();
    NativeD3dGameHost::initialize(nativeMode->fbWidth, nativeMode->efbHeight);
    GALAXY_GAME_TRACE("GameSystem::init D3D host initialized");
    mErrorWatcher = new GameSystemErrorWatcher();
    mFrameControl = new GameSystemFrameControl();
    SingletonHolder< GameSystemResetAndPowerProcess >::init();
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->initWithoutIter();
    mStationedArchiveLoader = new GameSystemStationedArchiveLoader();
    mHomeButtonLayout = new HomeButtonLayout();
    mHomeButtonStateNotifier = new HomeButtonStateNotifier();
    mDimmingWatcher = new GameSystemDimmingWatcher();
    setNerve(GET_NERVE(GameSystem, GameSystemInitializeAudio));
    GALAXY_GAME_TRACE("GameSystem::init complete");
}

bool GameSystem::isExecuteLoadSystemArchive() const {
    return mIsExecuteLoadSystemArchive;
}

bool GameSystem::isDoneLoadSystemArchive() const {
    return isNerve(GET_NERVE(GameSystem, GameSystemNormal));
}

void GameSystem::startToLoadSystemArchive() {
    mIsExecuteLoadSystemArchive = true;

    SingletonHolder< HeapMemoryWatcher >::get()->setCurrentHeapToStationedHeap();
    SingletonHolder< NameObjRegister >::get()->setCurrentHolder(mObjHolder->mObjHolder);
    setNerve(GET_NERVE(GameSystem, GameSystemLoadStationedArchive));
}

void GameSystem::exeInitializeAudio() {
    if (MR::isFirstStep(this)) {
        MR::startFunctionAsyncExecute(MR::Functor(mObjHolder, &GameSystemObjHolder::createAudioSystem), 14, INIT_AUDIO_KEY);
    }

    updateSceneController();

    if (MR::isEndFunctionAsyncExecute(INIT_AUDIO_KEY) && mObjHolder->mAudioSystem->isLoadDoneWaveDataAtSystemInit()) {
        MR::waitForEndFunctionAsyncExecute(INIT_AUDIO_KEY);
        setNerve(GET_NERVE(GameSystem, GameSystemInitializeLogoScene));
    }
}

void GameSystem::exeInitializeLogoScene() {
    if (GameSystemFunction::isResetProcessing()) {
        setNerve(GET_NERVE(GameSystem, GameSystemWaitForReboot));
    } else {
        if (MR::isFirstStep(this)) {
            MR::requestChangeScene("Logo");
        }

        updateSceneController();
    }
}

void GameSystem::exeLoadStationedArchive() {
    mStationedArchiveLoader->update();
    updateSceneController();

    if (mStationedArchiveLoader->isDone()) {
        setNerve(GET_NERVE(GameSystem, GameSystemNormal));
    }
}

void GameSystem::exeWaitForReboot() {
}

void GameSystem::exeNormal() {
    updateSceneController();
    mStationedArchiveLoader->update();
}

void GameSystem::initGX() {
    if (mFifoBase == nullptr) {
        mFifoBase = new (32) u8[GX_FIFO_SIZE];
    }

    GXInit(mFifoBase, GX_FIFO_SIZE);
}

void GameSystem::initAfterStationedResourceLoaded() {
    mFontHolder->createFontFromFile();
    mObjHolder->initAfterStationedResourceLoaded();
    mHomeButtonLayout->initWithoutIter();
    mErrorWatcher->initAfterResourceLoaded();
    mSystemWipeHolder = MR::createSystemWipeHolder();
    mSceneController->initAfterStationedResourceLoaded();
    mSequenceDirector->initAfterResourceLoaded();
}

void GameSystem::prepareReset() {
    mStationedArchiveLoader->prepareReset();
}

inline bool isSystemWaitForReboot(const GameSystem* pGameSystem) {
    return pGameSystem->isNerve(GET_NERVE(GameSystem, GameSystemWaitForReboot));
}

inline bool isSystemNormal(const GameSystem* pGameSystem) {
    return pGameSystem->isNerve(GET_NERVE(GameSystem, GameSystemNormal));
}

bool GameSystem::isPreparedReset() const {
    return isSystemWaitForReboot(this) || isSystemNormal(this) || mStationedArchiveLoader->isPreparedReset();
}

void GameSystem::frameLoop() {
    static Native::XfbResourceRegistry xfbResources;
    auto* xfbManager = JUTXfb::getManager();
    auto* videoManager = JUTVideo::getManager();
    if (!xfbManager || !videoManager || !videoManager->getRenderMode())
        throw std::logic_error("Native game frame loop requires initialized video/XFB managers");
    const auto* mode = videoManager->getRenderMode();
    auto& renderHost = NativeD3dGameHost::instance();
    if (!renderHost.pumpMessages()) throw std::runtime_error("Native game window closed");
    xfbResources.refresh(renderHost.resources(),
        {xfbManager->mXfbs[0], xfbManager->mXfbs[1], xfbManager->mXfbs[2]},
        static_cast<unsigned>(xfbManager->getBufferNum()), mode->fbWidth, mode->xfbHeight);
    static Native::GxCommandState displayCopyState;
    Native::DisplayCopyBinding displayCopyBinding(displayCopyState, renderHost.resources());
    Native::TextureCopyBinding textureCopyBinding(displayCopyState, renderHost.resources());
    nativePumpGameInputFrame();
    MainLoopFramework::sManager->beginRender();
    draw();
    MainLoopFramework::sManager->endRender();
    update();
    calcAnim();
    mObjHolder->captureIfAllowForScreenPreserver();
    MainLoopFramework::sManager->endFrame();
    MainLoopFramework::sManager->waitForRetrace();
}

void GameSystem::draw() {
    mSceneController->drawScene();
    mSequenceDirector->draw();
    mObjHolder->drawStarPointer();
    mObjHolder->drawBeforeEndRender();

    if (mSystemWipeHolder != nullptr) {
        mSystemWipeHolder->draw();
    }

    mErrorWatcher->draw();
    mHomeButtonLayout->draw();
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->draw();
}

void GameSystem::update() {
    SingletonHolder< GameSystemResetAndPowerProcess >::get()->movement();
    mSceneController->checkRequestAndChangeScene();
    mObjHolder->update();
    mHomeButtonLayout->movement();

    if (!mHomeButtonLayout->isActive()) {
        mErrorWatcher->movement();
    }

    mDimmingWatcher->_5 = mErrorWatcher->isWarning() || mHomeButtonLayout->isActive() || GameSequenceFunction::isActiveSaveDataHandleSequence();
    mDimmingWatcher->update();
    updateNerve();
}

void GameSystem::updateSceneController() {
    bool isSceneUpdate = true;
    bool isResetProcessing = SingletonHolder< GameSystemResetAndPowerProcess >::get()->isActive();

    mObjHolder->updateAudioSystem();

    if (isResetProcessing) {
        isSceneUpdate = false;
    }

    if (mHomeButtonLayout->isActive()) {
        isSceneUpdate = false;
    }

    if (GameSystemFunction::isOccurredSystemWarning()) {
        isSceneUpdate = false;
    }

    mHomeButtonStateNotifier->update(mHomeButtonLayout->isActive() || GameSystemFunction::isOccurredSystemWarning());

    if (isSceneUpdate || isResetProcessing) {
        mSequenceDirector->update();
    }

    if (isSceneUpdate || mSceneController->isFirstUpdateSceneNerveNormal()) {
        if (mSystemWipeHolder != nullptr) {
            mSystemWipeHolder->movement();
        }

        mSceneController->updateScene();
    }

    if (isResetProcessing) {
        mSceneController->updateSceneDuringResetProcessing();
    }
}

void GameSystem::calcAnim() {
    mSceneController->calcAnimScene();

    if (mSystemWipeHolder != nullptr) {
        mSystemWipeHolder->calcAnim();
    }
}


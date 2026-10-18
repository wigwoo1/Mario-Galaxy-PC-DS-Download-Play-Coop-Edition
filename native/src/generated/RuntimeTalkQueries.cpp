#include "Game/Util/TalkUtil.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TalkNodeCtrl.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/YesNoController.hpp"
#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace MR {
    bool tryTalkForce(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce() && pCtrl->startTalkForce()) {
            ret = true;
        }

        return ret;
    }
    const MtxPtr getMessageBalloonFollowMatrix(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mMtx;
    }
    const TVec3f& getMessageBalloonFollowOffset(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mMsgBalloonFollowOffs;
    }
    bool isTalkTalking(const TalkMessageCtrl* pCtrl) {
        return pCtrl->_18 == 3;
    }
    bool isShortTalk(const TalkMessageCtrl* pCtrl) {
        return TalkFunction::isShortTalk(pCtrl);
    }
    bool isNearPlayer(const TalkMessageCtrl* pCtrl, f32 dist) {
        return pCtrl->isNearPlayer(dist);
    }
    TalkMessageCtrl* createTalkCtrl(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
        pCtrl->createMessage(rIter, pMessageID);
        return pCtrl;
    }
    bool tryTalkForceWithoutDemoMarioPuppetableAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForceWithoutDemoPuppetable();
        }

        return false;
    }
    void registerBranchFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerBranchFunc(rFunc);
    }
    void registerEventFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerEventFunc(rFunc);
    }
    void registerAnimeFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerAnimeFunc(rFunc);
    }
    void registerKillFunc(TalkMessageCtrl* pCtrl, const TalkMessageFuncBase& rFunc) {
        pCtrl->registerKillFunc(rFunc);
    }
    void setMessageArg(TalkMessageCtrl* pCtrl, int value) {
        CustomTagArg arg(value, CustomTagArg::Type_Int);
        pCtrl->setMessageArg(arg);
    }
    void resetNode(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->resetFlowNode();
    }
    void forwardNode(TalkMessageCtrl* pCtrl) {
        TalkFunction::onTalkStateNone(pCtrl);
        pCtrl->mNodeCtrl->forwardFlowNode();
        pCtrl->mNodeCtrl->recordTempFlowNode();
    }
    bool tryForwardNode(TalkMessageCtrl* pCtrl) {
        if (pCtrl->mNodeCtrl->isExistNextNode()) {
            TalkFunction::onTalkStateNone(pCtrl);
            TalkFunction::onTalkStateNone(pCtrl);
            pCtrl->mNodeCtrl->forwardFlowNode();
            pCtrl->mNodeCtrl->recordTempFlowNode();
            return true;
        }

        return false;
    }
    bool isExistNextNode(const TalkMessageCtrl* pCtrl) {
        return pCtrl->mNodeCtrl->isExistNextNode();
    }
    void setDistanceToTalk(TalkMessageCtrl* pCtrl, f32 dist) {
        pCtrl->mTalkDistance = dist;
    }
    bool tryTalkNearPlayer(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalk()) {
            if (pCtrl->startTalk()) {
                ret = true;
            }
        }

        return ret;
    }
    bool tryTalkNearPlayerAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            pCtrl->requestTalk();
            return true;
        } else {
            if (pCtrl->requestTalk()) {
                pCtrl->startTalk();
            }

            return false;
        }
    }
    bool tryTalkForceAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForce();
        }

        return false;
    }
    bool tryTalkForceWithoutDemo(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        bool ret = false;

        if (pCtrl->requestTalkForce() && pCtrl->startTalkForceWithoutDemo()) {
            ret = true;
        }

        return ret;
    }
    bool tryTalkForceWithoutDemoAtEnd(TalkMessageCtrl* pCtrl) {
        if (MR::isTimeKeepDemoActive()) {
            return false;
        }

        if (pCtrl->endTalk()) {
            return true;
        }

        if (pCtrl->requestTalkForce()) {
            pCtrl->startTalkForceWithoutDemo();
        }

        return false;
    }
    bool tryTalkSelectLeft(TalkMessageCtrl* pCtrl) {
        bool ret = false;

        if (MR::isYesNoSelected()) {
            if (MR::isYesNoSelectedYes()) {
                ret = true;
            }
        }

        return ret;
    }
    TalkMessageCtrl* createTalkCtrlDirect(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = new TalkMessageCtrl(pActor, rOffset, pMtx);
        pCtrl->createMessageDirect(rIter, pMessageID);
        return pCtrl;
    }
    TalkMessageCtrl* createTalkCtrlDirectOnRootNodeAutomatic(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID,
                                                             const TVec3f& rOffset, MtxPtr pMtx) {
        TalkMessageCtrl* pCtrl = createTalkCtrlDirect(pActor, rIter, pMessageID, rOffset, pMtx);
        onRootNodeAutomatic(pCtrl);
        return pCtrl;
    }
    void setMessageBalloonFollowOffset(TalkMessageCtrl* pCtrl, const TVec3f& rOffs) {
        pCtrl->setMessageBallonFollowOffs(rOffs);
    }
    void onRootNodeAutomatic(TalkMessageCtrl* pCtrl) {
        pCtrl->mIsOnRootNodeAuto = true;
    }
}


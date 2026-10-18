#include "Game/Util/NPCUtil.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/NPC/NPCFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <cstdio>
namespace MR {
    void startNPCTalkCamera(const TalkMessageCtrl* pTalk, MtxPtr pActorMtx, f32 distanceScale, s32 frames) {
        startNPCTalkCamera(pTalk, pActorMtx, getPlayerBaseMtx(), distanceScale, frames);
    }
    void startNPCTalkCamera(const TalkMessageCtrl* pTalk, MtxPtr pActorMtx, MtxPtr pPlayerMtx, f32 distanceScale, s32 frames) {
        TVec3f actorPos;
        TVec3f up;
        TVec3f side;
        TVec3f playerPos;
        TVec3f offset(getMessageBalloonFollowOffset(pTalk));
        if (getMessageBalloonFollowMatrix(pTalk)) {
            pActorMtx = getMessageBalloonFollowMatrix(pTalk);
        }

        up.set< f32 >(pPlayerMtx[0][1], pPlayerMtx[1][1], pPlayerMtx[2][1]);
        actorPos.set< f32 >(pActorMtx[0][3], pActorMtx[1][3], pActorMtx[2][3]);
        playerPos.set< f32 >(pPlayerMtx[0][3], pPlayerMtx[1][3], pPlayerMtx[2][3]);
        if (normalizeOrZero(&up)) {
            up.set< f32 >(0.0f, 1.0f, 0.0f);
        }

        f32 distance = playerPos.distance(actorPos);
        f32 angle = MR::atan2(1.0f, cosDegree(67.5f));
        f32 height = vecKillElement(actorPos - playerPos, up, &side);
        f32 scale = (height + offset.y) / distance / 0.75f;
        scale = scale >= 1.0f ? scale : 1.0f;
        distance *= angle;
        distance *= scale;
        f32 cameraDistance = 2.0f * distance;
        f32 cameraHeight = height / 6.0f + offset.y;
        cameraDistance *= distanceScale;
        cameraDistance = cameraDistance >= 450.0f ? cameraDistance : 450.0f;
        startTalkCamera(actorPos, up, cameraHeight, cameraDistance, frames);
    }
}

#include <revolution.h>

#include "nw4r/lyt/common.h"
#include "nw4r/lyt/pane.h"
#include "nw4r/lyt/group.h"
#include "nw4r/lyt/material.h"
#include "nw4r/lyt/animation.h"
#include "nw4r/lyt/layout.h"
#include "nw4r/lyt/resourceAccessor.h"
#include <new>

const f32 R_SAME_TOLERANCE = 1.0e-5F;
const f32 R_FRAME_TOLERANCE = 0.001F;

namespace nw4r {
    namespace lyt {

        namespace {

            inline bool RIsSame(const f32 a, const f32 b, const f32 tolerance = R_SAME_TOLERANCE) {
                f32 c = a - b;
                return (-tolerance < c && c < tolerance);
            }

            u16 GetStepCurveValue(f32 frame, const res::StepKey* keyArray, u32 keySize) {
                if (keySize == 0) {
                    return 0;
                }
                const u8* const keyBytes = reinterpret_cast<const u8*>(keyArray);
                auto keyFrame = [keyBytes](u32 index) {
                    return detail::ReadBigEndianF32(keyBytes + sizeof(res::StepKey) * index +
                                                    offsetof(res::StepKey, frame));
                };
                auto keyValue = [keyBytes](u32 index) {
                    return detail::ReadBigEndianU16(keyBytes + sizeof(res::StepKey) * index +
                                                    offsetof(res::StepKey, value));
                };
                if (keySize == 1 || frame <= keyFrame(0)) {
                    return keyValue(0);
                } else if (frame >= keyFrame(keySize - 1)) {
                    return keyValue(keySize - 1);
                }

                int ikeyL = 0;
                int ikeyR = static_cast<int>(keySize) - 1;
                while (ikeyL != ikeyR - 1 && ikeyL != ikeyR) {
                    int ikeyCenter = (ikeyL + ikeyR) / 2;
                    if (frame < keyFrame(static_cast<u32>(ikeyCenter))) {
                        ikeyR = ikeyCenter;
                    } else {
                        ikeyL = ikeyCenter;
                    }
                }

                if (RIsSame(frame, keyFrame(static_cast<u32>(ikeyR)), R_FRAME_TOLERANCE)) {
                    return keyValue(static_cast<u32>(ikeyR));
                } else {
                    return keyValue(static_cast<u32>(ikeyL));
                }
            }

            f32 GetHermiteCurveValue(f32 frame, const res::HermiteKey* keyArray, u32 keySize) {
                if (keySize == 0) {
                    return 0.0F;
                }
                const u8* const keyBytes = reinterpret_cast<const u8*>(keyArray);
                struct HostHermiteKey { f32 frame; f32 value; f32 slope; };
                auto keyAt = [keyBytes](u32 index) {
                    const u8* const bytes = keyBytes + sizeof(res::HermiteKey) * index;
                    return HostHermiteKey{
                        detail::ReadBigEndianF32(bytes + offsetof(res::HermiteKey, frame)),
                        detail::ReadBigEndianF32(bytes + offsetof(res::HermiteKey, value)),
                        detail::ReadBigEndianF32(bytes + offsetof(res::HermiteKey, slope))};
                };
                if (keySize == 1 || frame <= keyAt(0).frame) {
                    return keyAt(0).value;
                } else if (frame >= keyAt(keySize - 1).frame) {
                    return keyAt(keySize - 1).value;
                }

                int ikeyL = 0;
                int ikeyR = static_cast<int>(keySize) - 1;
                while (ikeyL != ikeyR - 1 && ikeyL != ikeyR) {
                    int ikeyCenter = (ikeyL + ikeyR) / 2;
                    if (frame <= keyAt(static_cast<u32>(ikeyCenter)).frame) {
                        ikeyR = ikeyCenter;
                    } else {
                        ikeyL = ikeyCenter;
                    }
                }

                const HostHermiteKey key0 = keyAt(static_cast<u32>(ikeyL));
                const HostHermiteKey key1 = keyAt(static_cast<u32>(ikeyR));
                if (RIsSame(frame, key1.frame, R_FRAME_TOLERANCE)) {
                    if (ikeyR < static_cast<int>(keySize) - 1 && key1.frame == keyAt(static_cast<u32>(ikeyR + 1)).frame) {
                        return keyAt(static_cast<u32>(ikeyR + 1)).value;
                    } else {
                        return key1.value;
                    }
                }
                f32 t1 = frame - key0.frame;
                f32 t2 = 1.0F / (key1.frame - key0.frame);
                f32 v0 = key0.value;
                f32 v1 = key1.value;
                f32 s0 = key0.slope;
                f32 s1 = key1.slope;

                f32 t1t1t2 = t1 * t1 * t2;
                f32 t1t1t2t2 = t1t1t2 * t2;
                f32 t1t1t1t2t2 = t1 * t1t1t2t2;
                f32 t1t1t1t2t2t2 = t1t1t1t2t2 * t2;

                return v0 * (2.0F * t1t1t1t2t2t2 - 3.0F * t1t1t2t2 + 1.0F) + v1 * (-2.0F * t1t1t1t2t2t2 + 3.0F * t1t1t2t2) +
                       s0 * (t1t1t1t2t2 - 2.0F * t1t1t2 + t1) + s1 * (t1t1t1t2t2 - t1t1t2);
            }

            void AnimatePaneSRT(Pane* pPane, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u8 target = targetBytes[offsetof(res::AnimationTarget, target)];
                    if (target >= 8) {
                        continue;
                    }
                    const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                    const res::HermiteKey* keys = reinterpret_cast<const res::HermiteKey*>(targetBytes + keysOffset);
                    pPane->SetSRTElement(target,
                        GetHermiteCurveValue(frame, keys, detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum))));
                }
            }

            void AnimateVisibility(Pane* pPane, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                    const res::StepKey* keys = reinterpret_cast<const res::StepKey*>(targetBytes + keysOffset);
                    pPane->SetVisible(0 != GetStepCurveValue(frame, keys,
                        detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum))));
                }
            }

            void AnimateVertexColor(Pane* pPane, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                    const res::HermiteKey* keys = reinterpret_cast<const res::HermiteKey*>(targetBytes + keysOffset);
                    f32 value = GetHermiteCurveValue(frame, keys,
                        detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum)));
                    value += 0.5f;
                    u8 u8Val;
                    OSf32tou8(&value, &u8Val);
                    pPane->SetColorElement(targetBytes[offsetof(res::AnimationTarget, target)], u8Val);
                }
            }

            void AnimateMaterialColor(Material* pMaterial, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                using namespace nw4r;

                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                    const res::HermiteKey* keys = reinterpret_cast<const res::HermiteKey*>(targetBytes + keysOffset);
                    f32 value = GetHermiteCurveValue(frame, keys,
                        detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum)));
                    value += 0.5f;
                    s16 s16Val;
                    OSf32tos16(&value, &s16Val);

                    s16Val = ut::Min(ut::Max(s16Val, s16(-1024)), s16(1023));
                    pMaterial->SetColorElement(targetBytes[offsetof(res::AnimationTarget, target)], s16Val);
                }
            }

            void AnimateTextureSRT(Material* pMaterial, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u8 id = targetBytes[offsetof(res::AnimationTarget, id)];
                    if (id < pMaterial->GetTexSRTCap()) {
                        const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                        const res::HermiteKey* keys = reinterpret_cast<const res::HermiteKey*>(targetBytes + keysOffset);
                        pMaterial->SetTexSRTElement(id, targetBytes[offsetof(res::AnimationTarget, target)], GetHermiteCurveValue(frame, keys,
                            detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum))));
                    }
                }
            }

            void AnimateTexturePattern(Material* pMaterial, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame,
                                       void** tpls) {
                for (u32 j = 0; j < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++j) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * j);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u8 id = targetBytes[offsetof(res::AnimationTarget, id)];
                    if (id < pMaterial->GetTextureNum()) {
                        const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                        const res::StepKey* keys = reinterpret_cast<const res::StepKey*>(targetBytes + keysOffset);
                        const u16 fileIdx = GetStepCurveValue(frame, keys,
                            detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum)));

                        if (tpls[fileIdx]) {
                            pMaterial->GetTexturePtr(id)->ReplaceImage(static_cast< TPLPalettePtr >(tpls[fileIdx]));
                        }
                    }
                }
            }

            void AnimateIndTexSRT(Material* pMaterial, const u8* animInfoBytes, const u8* animTargetOffsets, f32 frame) {
                for (u32 i = 0; i < animInfoBytes[offsetof(res::AnimationInfo, num)]; ++i) {
                    const u32 targetOffset = detail::ReadBigEndianU32(animTargetOffsets + sizeof(u32) * i);
                    const u8* const targetBytes = animInfoBytes + targetOffset;
                    const u8 id = targetBytes[offsetof(res::AnimationTarget, id)];
                    if (id < pMaterial->GetIndTexSRTCap()) {
                        const u32 keysOffset = detail::ReadBigEndianU32(targetBytes + offsetof(res::AnimationTarget, keysOffset));
                        const res::HermiteKey* keys = reinterpret_cast<const res::HermiteKey*>(targetBytes + keysOffset);
                        pMaterial->SetIndTexSRTElement(id, targetBytes[offsetof(res::AnimationTarget, target)], GetHermiteCurveValue(frame, keys,
                            detail::ReadBigEndianU16(targetBytes + offsetof(res::AnimationTarget, keyNum))));
                    }
                }
            }

            inline bool IsBindAnimation(Pane* pPane, AnimTransform* pAnimTrans) {
#if 1

                (void)pPane;
                (void)pAnimTrans;

#else

                if (pPane->FindAnimationLinkSelf(pAnimTrans)) {
                    return true;
                }

#endif

                return false;
            }

            inline bool IsBindAnimation(Material* pMaterial, AnimTransform* pAnimTrans) {
#if 1

                (void)pMaterial;
                (void)pAnimTrans;

#else

                if (pMaterial->FindAnimationLink(pAnimTrans)) {
                    return true;
                }

#endif

                return false;
            }

        }

        AnimTransform::AnimTransform() : mpRes(0), mFrame(0) {
        }

        AnimTransform::~AnimTransform() {
        }

        u16 AnimTransform::GetFrameSize() const {
            return detail::ReadBigEndianU16(reinterpret_cast<const u8*>(mpRes) + offsetof(res::AnimationBlock, frameSize));
        }

        bool AnimTransform::IsLoopData() const {
            return mpRes->loop != 0;
        }

        AnimTransformBasic::AnimTransformBasic() : mpFileResAry(0), mAnimLinkAry(0), mAnimLinkNum(0) {
        }

        AnimTransformBasic::~AnimTransformBasic() {
            Layout::DeleteArray(mAnimLinkAry, mAnimLinkNum);
            Layout::DeletePrimArray(mpFileResAry);
        }

        void AnimTransformBasic::SetResource(const res::AnimationBlock* pRes, ResourceAccessor* pResAccessor) {
            const u16 animCount = detail::ReadBigEndianU16(
                reinterpret_cast<const u8*>(pRes) + offsetof(res::AnimationBlock, animContNum));
            SetResource(pRes, pResAccessor, animCount);
        }

        void AnimTransformBasic::SetResource(const res::AnimationBlock* pBlock, ResourceAccessor* pResAccessor, u16 animNum) {
            mpRes = pBlock;
            mpFileResAry = nullptr;
            const u8* const blockBytes = reinterpret_cast<const u8*>(pBlock);
            const u16 fileCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, fileNum));

            if (fileCount > 0) {
                mpFileResAry = static_cast< void** >(Layout::AllocMemory(sizeof(void*) * fileCount));
                if (mpFileResAry != nullptr) {
                    const u8* const fileNameOffsets = blockBytes + sizeof(*mpRes);
                    for (u16 i = 0; i < fileCount; i++) {
                        const u32 nameOffset = detail::ReadBigEndianU32(fileNameOffsets + sizeof(u32) * i);
                        const char* const name = reinterpret_cast<const char*>(fileNameOffsets + nameOffset);
                        mpFileResAry[i] = pResAccessor->GetResource('timg', name, 0);
                    }
                }
            }

            mAnimLinkAry = static_cast< AnimationLink* >(Layout::AllocMemory(sizeof(AnimationLink) * animNum));
            if (mAnimLinkAry != nullptr) {
                mAnimLinkNum = animNum;
                memset(mAnimLinkAry, 0, animNum * sizeof(AnimationLink));

                for (u16 i = 0; i < animNum; i++) {
                    new (&mAnimLinkAry[i]) AnimationLink();
                }
            }
        }

        void AnimTransformBasic::Bind(Pane* pPane, bool bRecursive) {
            AnimationLink* pCrAnimLink = 0;

            const u8* const blockBytes = reinterpret_cast<const u8*>(mpRes);
            const u16 animCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, animContNum));
            const u32 offsetsOffset = detail::ReadBigEndianU32(blockBytes + offsetof(res::AnimationBlock, animContOffsetsOffset));
            const u8* const animContOffsets = blockBytes + offsetsOffset;
            for (u16 i = 0; i < animCount; ++i) {
                const u32 contentOffset = detail::ReadBigEndianU32(animContOffsets + sizeof(u32) * i);
                const res::AnimationContent& animCont = *reinterpret_cast<const res::AnimationContent*>(blockBytes + contentOffset);
                if (animCont.type == ANIMCONTENTTYPE_PANE) {
                    if (Pane* const pFindPane = pPane->FindPaneByName(animCont.name, bRecursive)) {
                        if (!IsBindAnimation(pFindPane, this)) {
                            pCrAnimLink = Bind(pFindPane, pCrAnimLink, i);
                            if (!pCrAnimLink) {
                                break;
                            }
                        }
                    }
                } else {
                    if (Material* const pFindMat = pPane->FindMaterialByName(animCont.name, bRecursive)) {
                        if (!IsBindAnimation(pFindMat, this)) {
                            pCrAnimLink = Bind(pFindMat, pCrAnimLink, i);
                            if (!pCrAnimLink) {
                                break;
                            }
                        }
                    }
                }
            }
        }

        void AnimTransformBasic::Bind(Material* pMaterial) {
            AnimationLink* pCrAnimLink = 0;

            const u8* const blockBytes = reinterpret_cast<const u8*>(mpRes);
            const u16 animCount = detail::ReadBigEndianU16(blockBytes + offsetof(res::AnimationBlock, animContNum));
            const u32 offsetsOffset = detail::ReadBigEndianU32(blockBytes + offsetof(res::AnimationBlock, animContOffsetsOffset));
            const u8* const animContOffsets = blockBytes + offsetsOffset;
            for (u16 i = 0; i < animCount; ++i) {
                const u32 contentOffset = detail::ReadBigEndianU32(animContOffsets + sizeof(u32) * i);
                const res::AnimationContent& animCont = *reinterpret_cast<const res::AnimationContent*>(blockBytes + contentOffset);
                if (animCont.type == ANIMCONTENTTYPE_MATERIAL) {
                    if (detail::EqualsMaterialName(pMaterial->GetName(), animCont.name)) {
                        if (!IsBindAnimation(pMaterial, this)) {
                            pCrAnimLink = Bind(pMaterial, pCrAnimLink, i);
                            if (!pCrAnimLink) {
                                break;
                            }
                        }
                    }
                }
            }
        }

        template < typename T >
        AnimationLink* AnimTransformBasic::Bind(T* pTarget, AnimationLink* pAnimLink, u16 idx) {
            pAnimLink = FindUnbindLink(pAnimLink);
            if (!pAnimLink) {
                return 0;
            }

            pAnimLink->Set(this, idx);
            pTarget->AddAnimationLink(pAnimLink);

            return ++pAnimLink;
        }

        void AnimTransformBasic::Animate(u32 idx, Pane* pPane) {
            const u8* const blockBytes = reinterpret_cast<const u8*>(mpRes);
            const u32 offsetsOffset = detail::ReadBigEndianU32(blockBytes + offsetof(res::AnimationBlock, animContOffsetsOffset));
            const u8* const animContOffsets = blockBytes + offsetsOffset;
            const u32 contentOffset = detail::ReadBigEndianU32(animContOffsets + sizeof(u32) * idx);
            const u8* const animContBytes = blockBytes + contentOffset;
            const u8* const animInfoOffsets = animContBytes + sizeof(res::AnimationContent);
            for (u32 i = 0; i < animContBytes[offsetof(res::AnimationContent, num)]; ++i) {
                const u32 infoOffset = detail::ReadBigEndianU32(animInfoOffsets + sizeof(u32) * i);
                const u8* const animInfoBytes = animContBytes + infoOffset;
                const u8* const animTargetOffsets = animInfoBytes + sizeof(res::AnimationInfo);

                switch (detail::GetSignatureInt(reinterpret_cast<const char*>(animInfoBytes + offsetof(res::AnimationInfo, kind)))) {
                case res::ANIMATIONTYPE_RLPA:
                    AnimatePaneSRT(pPane, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                case res::ANIMATIONTYPE_RLVI:
                    AnimateVisibility(pPane, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                case res::ANIMATIONTYPE_RLVC:
                    AnimateVertexColor(pPane, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                }
            }
        }

        void AnimTransformBasic::Animate(u32 idx, Material* pMaterial) {
            const u8* const blockBytes = reinterpret_cast<const u8*>(mpRes);
            const u32 offsetsOffset = detail::ReadBigEndianU32(blockBytes + offsetof(res::AnimationBlock, animContOffsetsOffset));
            const u8* const animContOffsets = blockBytes + offsetsOffset;
            const u32 contentOffset = detail::ReadBigEndianU32(animContOffsets + sizeof(u32) * idx);
            const u8* const animContBytes = blockBytes + contentOffset;
            const u8* const animInfoOffsets = animContBytes + sizeof(res::AnimationContent);
            for (u32 i = 0; i < animContBytes[offsetof(res::AnimationContent, num)]; ++i) {
                const u32 infoOffset = detail::ReadBigEndianU32(animInfoOffsets + sizeof(u32) * i);
                const u8* const animInfoBytes = animContBytes + infoOffset;
                const u8* const animTargetOffsets = animInfoBytes + sizeof(res::AnimationInfo);

                switch (detail::GetSignatureInt(reinterpret_cast<const char*>(animInfoBytes + offsetof(res::AnimationInfo, kind)))) {
                case res::ANIMATIONTYPE_RLMC:
                    AnimateMaterialColor(pMaterial, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                case res::ANIMATIONTYPE_RLTS:
                    AnimateTextureSRT(pMaterial, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                case res::ANIMATIONTYPE_RLTP:
                    if (mpFileResAry) {
                        AnimateTexturePattern(pMaterial, animInfoBytes, animTargetOffsets, mFrame, mpFileResAry);
                    }
                    break;
                case res::ANIMATIONTYPE_RLIM:
                    AnimateIndTexSRT(pMaterial, animInfoBytes, animTargetOffsets, mFrame);
                    break;
                }
            }
        }

        AnimationLink* AnimTransformBasic::FindUnbindLink(AnimationLink* pLink) const {
            if (pLink == 0) {
                pLink = mAnimLinkAry;
            }

            while (pLink < mAnimLinkAry + mAnimLinkNum) {
                if (pLink->GetAnimTransform() == 0) {
                    return pLink;
                }

                ++pLink;
            }

            return 0;
        }

        AnimResource::AnimResource() {
            Init();
        }

        void AnimResource::Set(const void* anmResBuf) {
            Init();

            const res::BinaryFileHeader* const pFileHeader = static_cast<const res::BinaryFileHeader*>(anmResBuf);
            const u8* const fileBytes = static_cast<const u8*>(anmResBuf);

            if (!detail::TestFileHeader(*pFileHeader, res::FILESIGNATURE_RLAN)) {
                return;
            }

            if (!detail::TestFileVersion(*pFileHeader)) {
                return;
            }

            mpFileHeader = pFileHeader;

            const u16 headerSize = detail::ReadBigEndianU16(fileBytes + offsetof(res::BinaryFileHeader, headerSize));
            const u16 blockCount = detail::ReadBigEndianU16(fileBytes + offsetof(res::BinaryFileHeader, dataBlocks));
            const u8* blockBytes = fileBytes + headerSize;
            for (u16 i = 0; i < blockCount; ++i) {
                const res::DataBlockHeader* const pDataBlockHead = reinterpret_cast<const res::DataBlockHeader*>(blockBytes);
                switch (detail::GetSignatureInt(pDataBlockHead->kind)) {
                case res::DATABLOCKKIND_PANEANIMTAG:
                    mpTagBlock = reinterpret_cast< const res::AnimationTagBlock* >(pDataBlockHead);
                    break;

                case res::DATABLOCKKIND_PANEANIMINFO:
                    mpResBlock = reinterpret_cast< const res::AnimationBlock* >(pDataBlockHead);
                    break;
                }

                const u32 blockSize = detail::ReadBigEndianU32(blockBytes + offsetof(res::DataBlockHeader, size));
                if (blockSize < sizeof(res::DataBlockHeader)) {
                    Init();
                    return;
                }
                blockBytes += blockSize;
            }
        }

        void AnimResource::Init() {
            mpFileHeader = 0;
            mpResBlock = 0;
            mpTagBlock = 0;
        }

        namespace detail {

            AnimationLink* FindAnimationLink(AnimationList* pAnimList, AnimTransform* pAnimTrans) {
                for (AnimationList::Iterator it = pAnimList->GetBeginIter(); it != pAnimList->GetEndIter(); ++it) {
                    if (pAnimTrans == it->GetAnimTransform()) {
                        return &(*it);
                    }
                }

                return 0;
            }

            AnimationLink* FindAnimationLink(AnimationList* pAnimList, const AnimResource& animRes) {
                for (AnimationList::Iterator it = pAnimList->GetBeginIter(); it != pAnimList->GetEndIter(); ++it) {
                    if (animRes.GetResourceBlock() == it->GetAnimTransform()->GetAnimResource()) {
                        return &(*it);
                    }
                }

                return 0;
            }

            void UnbindAnimationLink(AnimationList* pAnimList, AnimTransform* pAnimTrans) {
                for (AnimationList::Iterator it = pAnimList->GetBeginIter(); it != pAnimList->GetEndIter();) {
                    AnimationList::Iterator currIt = it++;
                    if (!pAnimTrans || currIt->GetAnimTransform() == pAnimTrans) {
                        pAnimList->Erase(currIt);
                        currIt->Reset();
                    }
                }
            }

        }

    }
}

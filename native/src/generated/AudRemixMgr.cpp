#include <Native/HeapPlacement.hpp>
#include <Native/RemixSequenceData.hpp>
#include "Game/AudioLib/AudRemixMgr.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>

AudRemixMgr::AudRemixMgr(JKRHeap* pHeap) {
    mHeap = pHeap;
    mGroups = nullptr;
    mSoundObj = nullptr;
    mRemixSeq = nullptr;
}

void AudRemixMgr::init() {
    mSoundObj = new (mHeap, 0) AudSoundObject(nullptr, 0x10, mHeap);
    mRemixSeq = new (mHeap, 0) AudRemixSequencer();
}

void AudRemixMgr::update() {
    mRemixSeq->update();
}

void AudRemixMgr::setRemixSeqResource(void* ptr, u32 byteCount) {
    mGroupCount = 0;
    mGroups = nullptr;

    Native::RemixSequenceData decoded;
    if (!Native::decodeRemixSequenceData(ptr, byteCount, decoded) || decoded.groups.size() > 0x7FFFFFFFu) {
        return;
    }

    const auto groupCount = static_cast<s32>(decoded.groups.size());
    auto* groups = groupCount ? new (mHeap, 0) RemixNoteGroupData[groupCount] : nullptr;
    if (groupCount && !groups) return;

    for (s32 i = 0; i < groupCount; ++i) {
        const auto& sourceGroup = decoded.groups[std::size_t(i)];
        auto& targetGroup = groups[i];
        if (sourceGroup.noteOrder.size() > 0x7FFFFFFFu || sourceGroup.tracks.size() > 0x7FFFFFFFu) return;
        targetGroup.mIndex = i;
        targetGroup.mNoteCount = static_cast<s32>(sourceGroup.noteOrder.size());
        targetGroup.mTrackCount = static_cast<s32>(sourceGroup.tracks.size());

        targetGroup._C = targetGroup.mNoteCount
            ? new (mHeap, 0) u32[targetGroup.mNoteCount]
            : nullptr;
        if (targetGroup.mNoteCount && !targetGroup._C) return;
        for (s32 note = 0; note < targetGroup.mNoteCount; ++note)
            targetGroup._C[note] = sourceGroup.noteOrder[std::size_t(note)];

        targetGroup.mRemixTracks = targetGroup.mTrackCount
            ? new (mHeap, 0) RemixNoteTrackData[targetGroup.mTrackCount]
            : nullptr;
        if (targetGroup.mTrackCount && !targetGroup.mRemixTracks) return;
        for (s32 track = 0; track < targetGroup.mTrackCount; ++track) {
            const auto& sourceTrack = sourceGroup.tracks[std::size_t(track)];
            auto& targetTrack = targetGroup.mRemixTracks[track];
            targetTrack._0 = sourceTrack.index;
            targetTrack._4 = targetGroup.mNoteCount
                ? new (mHeap, 0) RemixNoteData[targetGroup.mNoteCount]
                : nullptr;
            if (targetGroup.mNoteCount && !targetTrack._4) return;
            for (s32 note = 0; note < targetGroup.mNoteCount; ++note) {
                const auto& sourceNote = sourceTrack.notes[std::size_t(note)];
                targetTrack._4[note]._0 = sourceNote.fields[0];
                targetTrack._4[note]._4 = sourceNote.fields[1];
                targetTrack._4[note]._8 = sourceNote.fields[2];
                targetTrack._4[note]._C = sourceNote.fields[3];
            }
        }
    }

    mGroups = groups;
    mGroupCount = groupCount;
}

RemixNoteGroupData* AudRemixMgr::getRemixNoteGroupDataFromMelodyNo(s32 melodyNo) const {
    for (int i = 0; i < mGroupCount; i++) {
        if (melodyNo == mGroups[i].mIndex) {
            return &mGroups[i];
        }
    }
    return nullptr;
}


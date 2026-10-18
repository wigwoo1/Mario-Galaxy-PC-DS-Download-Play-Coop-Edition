#include "Game/Util/StringUtil.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#define CENTISEC_PER_SEC 100
#define SEC_PER_MIN 60
#define FRAME_PER_SEC 60
#define FRAME_PER_MIN 3600
#define FRAME_PER_HOUR 216000
namespace MR {
    void makeClearTimeString(wchar_t* pDst, u32 frame) {
        s32 minute;
        s32 second;
        s32 centisecond;

        if (frame >= FRAME_PER_HOUR) {
            minute = 59;
            second = 59;
            centisecond = 99;
        } else {
            s32 signedFrame = frame;

            minute = signedFrame / FRAME_PER_MIN;
            second = (signedFrame / FRAME_PER_SEC) % SEC_PER_MIN;
            centisecond = ((signedFrame % FRAME_PER_SEC) * CENTISEC_PER_SEC) / FRAME_PER_SEC;
        }

        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time000");

        ReplaceTagFunction::ReplaceArgs(pDst, 9, pMessage, 0, minute, second, centisecond);
    }

    void makeMinuteAndSecondString(wchar_t* pDst, u32 frame) {
        s32 minute;
        s32 second;

        if (frame >= FRAME_PER_HOUR) {
            minute = 59;
            second = 59;
        } else {
            minute = frame / FRAME_PER_MIN;
            second = (frame % FRAME_PER_MIN) / FRAME_PER_SEC;
        }

        const wchar_t* pMessage = MR::getGameMessageDirect("System_Time001");

        ReplaceTagFunction::ReplaceArgs(pDst, 16, pMessage, 0, minute, second);
    }

    void makeRaceBestTimeString(wchar_t* pDst, int raceId) {
        MR::makeClearTimeString(pDst, getRaceBestTime(raceId));
    }

    void makeRaceCurrentTimeString(wchar_t* pDst) {
        MR::makeClearTimeString(pDst, getRaceCurrentTime());
    }


}

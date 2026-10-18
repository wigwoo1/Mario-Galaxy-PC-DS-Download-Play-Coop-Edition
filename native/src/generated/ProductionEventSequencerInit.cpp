#include "Game/Player/PlayerEvent.hpp"
#include "Game/Player/PlayerEventAbyss.hpp"
#include "Game/Player/PlayerEventDown.hpp"
#include "Game/Player/PlayerEventFireDown.hpp"
#include "Game/Player/PlayerEventGameOver.hpp"
#include "Game/Player/PlayerEventGhostRaceDown.hpp"
#include "Game/Player/PlayerEventRaceDown.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"

void EventSequencer::init(const JMapInfoIter& rIter) {
    addEventSequence< EventDown >("マリオダウン");
    addEventSequence< EventAbyss >("マリオ奈落");
    addEventSequence< EventFireDown >("マリオ炎ダウン");
    addEventSequence< EventRaceDown >("レース負け");
    addEventSequence< EventGhostRaceDown >("ゴーストレース負け");
    addEventSequence< EventGameOver >("ゲームオーバー");

    MR::connectToSceneMapObjMovement(this);
}



#include <Game/Util/ModelUtil.hpp>
#include <Game/Animation/XanimePlayer.hpp>
namespace MR {
    bool isBckPlaying(XanimePlayer* pAnimePlayer, const char* pBckName) {
        bool running = pAnimePlayer->_24[pAnimePlayer->_54].checkState(1) ? false : true;
        return running && pAnimePlayer->isRun(pBckName) && pAnimePlayer->getRate() != 0.0f;
    }
}

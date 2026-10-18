#include "Game/System/WPadHolder.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/System/WPad.hpp"
namespace WPadFunction {
    WPadRumble* getWPadRumble(s32 channel) {
        return MR::getWPad(channel)->getRumbleInstance();
    }
};  // namespace WPadFunction


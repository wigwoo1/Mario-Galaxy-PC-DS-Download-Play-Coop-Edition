#pragma once

#include <Game/Util/MathUtil.hpp>

namespace MR {
inline s32 getRandom(long minimum, long maximum) {
    return getRandom(static_cast<s32>(minimum), static_cast<s32>(maximum));
}
}  // namespace MR

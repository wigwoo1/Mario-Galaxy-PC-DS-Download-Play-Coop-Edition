#include <Game/Util/MathUtil.hpp>
#include <Game/System/GameSystem.hpp>
#include <Game/System/GameSystemObjHolder.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cstring>
namespace MR {
    f32 getRandom() {
        u32 rand = SingletonHolder< GameSystem >::get()->mObjHolder->mRandom.rand();
        u32 value = (rand >> 9) | 0x3F800000;

        f32 result; std::memcpy(&result, &value, sizeof(result)); return result - 1.0f;
    }
    f32 getRandom(f32 min, f32 max) {
        return min + (max - min) * getRandom();
    }
    s32 getRandom(s32 min, s32 max) {
        return getRandom(static_cast< f32 >(min), static_cast< f32 >(max));
    }
    f32 getRandomDegree() {
        return getRandom(0.0f, 360.0f);
    }
    bool isHalfProbability() {
        return getRandom() < 0.5f;
    }
}

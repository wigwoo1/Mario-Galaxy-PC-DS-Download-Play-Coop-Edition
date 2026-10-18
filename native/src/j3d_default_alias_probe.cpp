// Compile-only check: aliases refer to the same const original default objects.
#include "JSystem/J3DGraphBase/J3DTevs.hpp"
#include <type_traits>
static_assert(std::is_const<typename std::remove_reference<decltype(J3DTevsDefault::j3dDefaultTevStageInfo)>::type>::value);
static_assert(&J3DTevsDefault::j3dDefaultTevStageInfo==&::j3dDefaultTevStageInfo);
static_assert(&J3DTevsDefault::j3dDefaultIndTevStageInfo==&::j3dDefaultIndTevStageInfo);
static_assert(&J3DTevsDefault::j3dDefaultTevSwapMode==&::j3dDefaultTevSwapMode);

#include <Game/NameObj/NameObj.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Scene/SceneFunction.hpp>
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>

// Observe the real adapter at its scene-service boundary without creating a
// live game, renderer, or collision scene.
NameObj::NameObj(const char* name) : mName(name), mFlag(0), mExecutorIdx(-1) {}
NameObj::~NameObj() = default;
void NameObj::init(const JMapInfoIter&) {}
void NameObj::initAfterPlacement() {}
void NameObj::movement() {}
void NameObj::draw() const {}
void NameObj::calcAnim() {}
void NameObj::calcViewAndEntry() {}

namespace {
NameObj* registered = nullptr;
std::array<int, 4> categories{};
std::vector<int> events;
}
namespace MR {
void registerNameObjToExecuteHolder(NameObj* object, int movement, int animation, int buffer, int draw) {
    registered = object;
    categories = {movement, animation, buffer, draw};
    events.push_back(1);
}
void connectToSceneTemporarily(NameObj* object) {
    assert(object == registered);
    events.push_back(2);
}
void connectToDrawTemporarily(NameObj* object) {
    assert(object == registered);
    events.push_back(3);
}
}
int main() {
    NameObj actor("scene connection fixture");
    for (const auto& input : {std::array<int, 4>{MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_Layout},
                              std::array<int, 4>{MR::MovementType_Camera, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None},
                              std::array<int, 4>{MR::MovementType_None, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_WipeLayout}}) {
        events.clear();
        MR::connectToScene(&actor, input[0], input[1], input[2], input[3]);
        assert(registered == &actor && categories == input);
        assert((events == std::vector<int>{1, 2, 3}));
    }
    std::puts("PASS native NameObj scene adapter: registration, movement/draw connection order, and category passthrough");
}

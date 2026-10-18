#include <Game/Map/RailGraph.hpp>
#include <Game/Map/RailGraphEdge.hpp>
#include <Game/Map/RailGraphIter.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>

// RailGraphEdge::setArgs shares its object file with the graph methods under
// test. The full native runtime supplies these JMapInfo functions; this focused
// test never calls setArgs, so these inert link shims keep its scope explicit.
s32 JMapInfo::searchItemInfo(const char*) const { return -1; }
bool JMapInfo::getValueFast(int, int, s32*) const { return false; }

int main() {
    RailGraph graph;
    const s32 a = graph.addNode(TVec3f(0.0f, 0.0f, 0.0f));
    const s32 b = graph.addNode(TVec3f(3.0f, 4.0f, 0.0f));
    const s32 c = graph.addNode(TVec3f(0.0f, 0.0f, 12.0f));
    graph.connectNodeTwoWay(a, b, nullptr);
    graph.connectNodeTwoWay(a, c, nullptr);
    graph.connectNodeTwoWay(b, c, nullptr);

    assert(graph._8 == 3 && graph._14 == 3);
    assert(graph.isValidEdge(0) && graph.isValidEdge(1) && graph.isValidEdge(2));
    assert(!graph.isValidEdge(-1) && !graph.isValidEdge(3));
    assert(std::abs(graph.getEdge(0)->mDistance - 5.0f) < 0.0001f);
    assert(std::abs(graph.getEdge(1)->mDistance - 12.0f) < 0.0001f);
    assert(std::abs(graph.getEdge(2)->mDistance - 13.0f) < 0.0001f);
    assert(graph.getEdge(0)->getNextNode(a) == b);
    assert(graph.getEdge(0)->getNextNode(b) == a);
    assert(graph.getEdge(0)->getNextNode(c) == -1);

    RailGraphIter iter(&graph);
    iter.setNode(a);
    iter.watchStartEdge();
    assert(!iter.isWatchEndEdge());
    assert(iter.getWatchNode()->_0.x == 3.0f);
    iter.selectEdge();
    assert(iter.isSelectedEdge());
    assert(iter.getNextNode()->_0.y == 4.0f);
    iter.moveNodeNext();
    assert(iter.getCurrentNode()->_0.x == 3.0f);
    iter.watchStartEdge();
    assert(iter.isWatchedPrevEdge());
    iter.watchNextEdge();
    assert(!iter.isWatchEndEdge());
    assert(iter.getWatchNode()->_0.z == 12.0f);
    iter.watchNextEdge();
    assert(iter.isWatchEndEdge());

    std::puts("PASS original Petari RailGraph creation, bidirectional edges, distances and iterator traversal");
    return 0;
}

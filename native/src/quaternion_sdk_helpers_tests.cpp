#include <cassert>
#include <cmath>
#include <revolution/mtx.h>

int main() {
    Mtx identity={{1,0,0,0},{0,1,0,0},{0,0,1,0}};
    Quaternion qIdentity{};
    C_QUATMtx(&qIdentity,identity);
    assert(std::abs(qIdentity.x)<1e-6f&&std::abs(qIdentity.y)<1e-6f&&std::abs(qIdentity.z)<1e-6f&&std::abs(qIdentity.w-1)<1e-6f);

    Mtx halfTurnX={{1,0,0,0},{0,-1,0,0},{0,0,-1,0}};
    Quaternion qHalfTurn{};
    C_QUATMtx(&qHalfTurn,halfTurnX);
    assert(std::abs(qHalfTurn.x-1)<1e-6f&&std::abs(qHalfTurn.y)<1e-6f&&std::abs(qHalfTurn.z)<1e-6f&&std::abs(qHalfTurn.w)<1e-6f);

    Quaternion midpoint{};
    C_QUATSlerp(&qIdentity,&qHalfTurn,&midpoint,.5f);
    const float rootHalf=std::sqrt(.5f);
    assert(std::abs(midpoint.x-rootHalf)<1e-5f&&std::abs(midpoint.y)<1e-6f&&std::abs(midpoint.z)<1e-6f&&std::abs(midpoint.w-rootHalf)<1e-5f);

    Quaternion opposite={-qIdentity.x,-qIdentity.y,-qIdentity.z,-qIdentity.w};
    Quaternion shortest{};
    C_QUATSlerp(&qIdentity,&opposite,&shortest,.5f);
    assert(std::abs(shortest.x)<1e-6f&&std::abs(shortest.y)<1e-6f&&std::abs(shortest.z)<1e-6f&&std::abs(shortest.w-1)<1e-6f);
    return 0;
}

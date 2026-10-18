#include <Game/Util/Functor.hpp>
#include <Game/NameObj/NameObjAdaptor.hpp>
#include <cassert>
#include <cstdio>
#include <stdexcept>

// Base scene registration is outside this callback-consumer boundary check.
NameObj::NameObj(const char* name):mName(name),mFlag(0),mExecutorIdx(-1){}
NameObj::~NameObj()=default;
void NameObj::init(const JMapInfoIter&){} void NameObj::initAfterPlacement(){}
void NameObj::movement(){} void NameObj::draw()const{} void NameObj::calcAnim(){} void NameObj::calcViewAndEntry(){}
void* JKRHeap::alloc(u32,int,JKRHeap*){throw std::logic_error("Unexpected heap clone in host-adaptor test");}
void JKRHeap::free(void*,JKRHeap*){throw std::logic_error("Unexpected heap free in host-adaptor test");}

static int draws;
static void drawCallback(){++draws;}
struct Target {
    int movements=0,animations=0;
    void move(){++movements;}
    void animate(){++animations;}
};
int main(){
    Target target;
    {
        NameObjAdaptor adaptor("actual callback consumer");
        {
            auto move=MR::Functor(&target,&Target::move);
            auto animate=MR::Functor(&target,&Target::animate);
            auto draw=MR::Functor(&drawCallback);
            adaptor.connectToMovement(move);
            adaptor.connectToCalcAnim(animate);
            adaptor.connectToDraw(draw);
            assert(adaptor.mMovementFunc!=&move&&adaptor.mCalcAnimFunc!=&animate&&adaptor.mDrawAnimFunc!=&draw);
        }
        NameObj* sceneObject=&adaptor;
        sceneObject->movement();sceneObject->calcAnim();sceneObject->draw();
        sceneObject->movement();sceneObject->draw();
        assert(target.movements==2&&target.animations==1&&draws==2);
    }
    std::puts("PASS actual NameObjAdaptor clones current-ABI member/free callbacks and dispatches them after the source functors expire");
}

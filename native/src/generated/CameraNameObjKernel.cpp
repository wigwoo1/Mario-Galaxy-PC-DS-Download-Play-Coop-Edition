#include <Game/NameObj/NameObj.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
NameObj::NameObj(const char* pName) : mName(pName), mFlag(), mExecutorIdx(-1) {
    SingletonHolder< NameObjRegister >::get()->add(this);
}
NameObj::~NameObj() {
}
void NameObj::init(const JMapInfoIter& rIter) {
}
void NameObj::initAfterPlacement() {
}
void NameObj::movement() {
}
void NameObj::draw() const {
}
void NameObj::calcAnim() {
}
void NameObj::calcViewAndEntry() {
}
NameObjHolder::NameObjHolder(int num) {
    mObjArray1.init(num);
}
void NameObjHolder::add(NameObj* pObj) {
    mObjArray1.push_back(pObj);
}
void NameObjHolder::clearArray() {
    mObjArray1.clear();
    mObjArray2.clear();
}

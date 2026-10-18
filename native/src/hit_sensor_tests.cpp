#include <Game/LiveActor/HitSensor.hpp>
#include <Game/LiveActor/SensorHitChecker.hpp>
#include <Game/Util/CollisionPartsFilter.hpp>
#include <cassert>
#include <cstdio>
namespace {SensorGroup* selected=nullptr;}
// Test-only scene boundary; runtime uses original SensorHitChecker SceneObj lookup.
namespace MR {void initHitSensorGroup(HitSensor* sensor){sensor->mSensorGroup=selected;}}
// Proximity test boundary: no player actor is installed in this focused fixture.
namespace MR {bool isExistSceneObj(int){return false;}TVec3f* getPlayerPos(){return nullptr;}bool isNear(const HitSensor*,const HitSensor*,float);}
// Filter-only fixture: no collision geometry is needed; filters inspect sensor ownership.
CollisionParts::CollisionParts(){mHitSensor=nullptr;}
int main(){
 SensorGroup first(4,"first"),second(4,"second");selected=&first;
 HitSensor a(10,2,30,nullptr),b(11,2,40,nullptr),zero(12,0,0,nullptr);
 assert(first.mSensorCount==0&&!a.isValid()&&a.mValidByHost);
 assert(a.isType(10)&&!a.isType(11)&&a.mPosition.x==0&&a.mRadius==30);
 a.mPosition.set(10,-20,30);b.mPosition.set(13,-16,42);
 assert(!MR::isNear(&a,&b,13)&&MR::isNear(&a,&b,13.01f)&&!MR::isNear(&a,&b,12.99f));
 assert(MR::isNear(&b,&a,13.01f));b.mPosition.set(a.mPosition);
 assert(!MR::isNear(&a,&b,0)&&MR::isNear(&a,&b,0.01f));
 b.mPosition.set(1010,-20,30);assert(!MR::isNear(&a,&b,1000));
 std::puts("PASS original sensor proximity: strict 3D boundary, symmetry, coincident zero threshold and translation");
 int identities[2];a.mHost=reinterpret_cast<LiveActor*>(&identities[0]);b.mHost=reinterpret_cast<LiveActor*>(&identities[1]);
 CollisionParts ownerParts;ownerParts.mHitSensor=&a;
 CollisionPartsFilterSensor sensorFilter(&a),otherFilter(&b);CollisionPartsFilterActor actorFilter(a.mHost);
 assert(sensorFilter.isInvalidParts(&ownerParts)&&!otherFilter.isInvalidParts(&ownerParts)&&actorFilter.isInvalidParts(&ownerParts));
 ownerParts.mHitSensor=&b;assert(!sensorFilter.isInvalidParts(&ownerParts)&&otherFilter.isInvalidParts(&ownerParts)&&!actorFilter.isInvalidParts(&ownerParts));
 b.mHost=a.mHost;assert(actorFilter.isInvalidParts(&ownerParts));
 std::puts("PASS original collision filters: sensor identity, actor ownership and multiple sensors sharing a host");
 a.validateBySystem();a.validateBySystem();assert(first.mSensorCount==1&&a.isValid());
 b.validateBySystem();assert(first.mSensorCount==2);
 a.addHitSensor(&b);a.addHitSensor(&zero);a.addHitSensor(&a);
 assert(a.mSensorCount==2&&a.mSensors[0]==&b&&a.mSensors[1]==&zero);
 first.clear();assert(a.mSensorCount==0&&b.mSensorCount==0&&first.mSensorCount==2);
 a.invalidate();a.invalidate();assert(first.mSensorCount==1&&first.mSensors[0]==&b&&!a.isValid());
 a.validate();a.validate();assert(first.mSensorCount==2&&a.isValid());
 selected=&second;a.setType(99);assert(first.mSensorCount==1&&second.mSensorCount==1&&a.isType(99)&&a.isValid());
 a.addHitSensor(&b);a.invalidateBySystem();a.invalidateBySystem();assert(second.mSensorCount==0&&a.mSensorCount==0&&!a.isValid());
 a.invalidate();a.validateBySystem();assert(second.mSensorCount==0&&!a.isValid());
 a.validate();assert(second.mSensorCount==1&&a.isValid());
 zero.validateBySystem();assert(zero.isValid()&&first.mSensorCount==1&&second.mSensorCount==1);
 a.invalidateBySystem();b.invalidateBySystem();zero.invalidateBySystem();
 delete[] a.mSensors;delete[] b.mSensors;delete[] first.mSensors;delete[] second.mSensors;
 std::puts("PASS original HitSensor registration, validity, type migration, hit capacity and frame clear");
}

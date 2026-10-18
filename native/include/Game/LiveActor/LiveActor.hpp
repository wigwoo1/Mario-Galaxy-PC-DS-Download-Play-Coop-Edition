#pragma once
#include <revolution.h>
#include <JSystem/JGeometry/TVec.hpp>
class HitSensor;class Nerve;class Spine;class Binder;
// Native sensor host interface only; not original LiveActor layout or gameplay.
class LiveActor {bool nativeDead=false;public:const char* mName="NativeActor";virtual void appear(){nativeDead=false;}virtual void kill(){mVelocity={0,0,0};nativeDead=true;} TVec3f mPosition,mVelocity;TVec3f mGravity{0,-1,0};Binder* mBinder=nullptr;Spine* mSpine=nullptr;void initNerve(const Nerve*);void setNerve(const Nerve*);bool isNerve(const Nerve*)const;s32 getNerveStep()const;virtual ~LiveActor()=default;virtual bool receiveMessage(u32,HitSensor*,HitSensor*);
virtual bool receiveMsgPush(HitSensor*,HitSensor*){return false;}
virtual bool receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*){return false;}
virtual bool receiveMsgEnemyAttack(u32,HitSensor*,HitSensor*){return false;}
virtual bool receiveMsgTake(HitSensor*,HitSensor*){return false;}
virtual bool receiveMsgTaken(HitSensor*,HitSensor*){return false;}
virtual bool receiveMsgThrow(HitSensor*,HitSensor*){return false;}
// Apart interaction state is pending; native default rejects it.
virtual bool receiveMsgApart(HitSensor*,HitSensor*){return false;}
virtual bool receiveOtherMsg(u32,HitSensor*,HitSensor*){return false;}virtual bool isClipped()const{return false;}virtual bool isDead()const{return nativeDead;}virtual MtxPtr getBaseMtx()const{return nullptr;}virtual void updateHitSensor(HitSensor*){}virtual void attackSensor(HitSensor*,HitSensor*){}};







#pragma once
#include "stateful_placed_actor.hpp"
inline AffineMatrix advanceDiagnosticPlacement(std::shared_ptr<NativeJmapTable> table,size_t row){
 struct State:Nerve {void execute(Spine* spine)const override{auto actor=static_cast<NativeStatefulPlacedActor*>(static_cast<LiveActor*>(spine->mExecutor));actor->localRotationDegrees[1]+=15;actor->localScale[0]*=1.1f;actor->mPosition.x+=1;}} state;
 NativeScene scene;auto& actor=scene.place<NativeStatefulPlacedActor>(JMapInfoIter(&table->info,int(row)),table,identityMatrix(),state);auto initial=actor.world;for(int i=0;i<3;++i)scene.tick();assetCheck(actor.getNerveStep()==3&&actor.world!=initial,"Rendered actor state did not advance");return actor.world;
}

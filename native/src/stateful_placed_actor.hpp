#pragma once
#include "placed_actor.hpp"
#include "stateful_sensor_host.hpp"
// Joins checked authored placement with native sensor/state host; not an original gameplay actor.
class NativeStatefulPlacedActor:public NativePlacedActor,public NativeStatefulSensorHost {
    Mtx baseMatrix{};
public:
    std::array<float,3> localRotationDegrees{},localScale{1,1,1};
    NativeStatefulPlacedActor(std::shared_ptr<NativeJmapTable> table,const AffineMatrix& parent,const Nerve& state):NativePlacedActor(std::move(table),parent),NativeStatefulSensorHost(state){}
    void init(const JMapInfoIter& iterator)override{NativePlacedActor::init(iterator);LiveActor::mName=placement.name.c_str();localRotationDegrees=placement.rotationDegrees;localScale=placement.scale;for(size_t r=0;r<3;++r)for(size_t c=0;c<4;++c)baseMatrix[r][c]=world[r*4+c];mPosition={baseMatrix[0][3],baseMatrix[1][3],baseMatrix[2][3]};}
    MtxPtr getBaseMtx()const override{return const_cast<MtxPtr>(baseMatrix);}
    void syncTransform(){
        assetCheck(std::isfinite(mPosition.x)&&std::isfinite(mPosition.y)&&std::isfinite(mPosition.z),"Invalid stateful actor position");auto local=placement;local.rotationDegrees=localRotationDegrees;local.scale=localScale;auto next=concatenate(parent,placementMatrix(local));next[3]=mPosition.x;next[7]=mPosition.y;next[11]=mPosition.z;for(float value:next)assetCheck(std::isfinite(value),"Invalid stateful actor transform");world=next;for(size_t r=0;r<3;++r)for(size_t c=0;c<4;++c)baseMatrix[r][c]=world[r*4+c];
    }
    void movement()override{NativePlacedActor::movement();if(!isDead()&&!isClipped())updateState();syncTransform();}
};
inline size_t verifyStatefulScenarioActors(WiiAssets& assets,const ScenarioAssembly& assembly,bool moving=false,bool transform=false){
 struct State:Nerve {bool moving=false,transform=false;mutable size_t calls=0;void execute(Spine* spine)const override{auto actor=static_cast<LiveActor*>(spine->mExecutor);assetCheck(actor->getNerveStep()>=0,"Placed state ran at negative step");if(moving){actor->mPosition.x+=1;actor->mPosition.y-=2;actor->mPosition.z+=.5f;}if(transform){auto placed=static_cast<NativeStatefulPlacedActor*>(actor);placed->localRotationDegrees[1]+=15;for(auto& scale:placed->localScale)scale*=1.1f;}++calls;}} state;state.moving=moving;state.transform=transform;
 NativeScene scene;NativeSensorWorld sensors;std::map<std::string,std::shared_ptr<NativeJmapTable>> tables;std::map<std::string,std::unique_ptr<Rarc>> archives;std::vector<NativeStatefulPlacedActor*> actors;std::vector<HitSensor*> bodies;std::vector<std::array<float,3>> expected;
 for(const auto& record:assembly.placements){if(std::filesystem::path(record.placement.table).filename()!="objinfo")continue;auto key=record.zone+":"+record.placement.table;auto& table=tables[key];if(!table){auto& archive=archives[record.zone];if(!archive)archive=std::make_unique<Rarc>(assets.readFile("/StageData/"+record.zone+".arc"));auto bytes=archive->readFile(record.placement.table);table=std::make_shared<NativeJmapTable>(BinaryView(bytes));}
 auto& actor=scene.place<NativeStatefulPlacedActor>(JMapInfoIter(&table->info,int(record.placement.row)),table,record.parent,state);assetCheck(actor.world==record.world&&actor.placement.name==record.placement.name&&actor.afterPlacement,"Stateful authored actor initialization mismatch");auto& keeper=sensors.createKeeperForType(2,1);auto& body=keeper.add("diagnostic",2,1,1,actor,{1,2,3});keeper.validate();actors.push_back(&actor);bodies.push_back(&body);auto expectedWorld=record.world;if(transform){auto local=record.placement;for(int i=0;i<3;++i){local.rotationDegrees[1]+=15;for(auto& scale:local.scale)scale*=1.1f;}expectedWorld=concatenate(record.parent,placementMatrix(local));}auto point=transformPoint(expectedWorld,{1,2,3});if(moving){point[0]+=3;point[1]-=6;point[2]+=1.5f;}expected.push_back(point);}
 for(int frame=0;frame<3;++frame){scene.tick();sensors.tick();}for(size_t i=0;i<actors.size();++i){assetCheck(actors[i]->updates==3&&actors[i]->getNerveStep()==3,"Placed actor state scheduling mismatch");auto matrix=actors[i]->getBaseMtx();assetCheck(matrix[0][3]==actors[i]->mPosition.x&&matrix[1][3]==actors[i]->mPosition.y&&matrix[2][3]==actors[i]->mPosition.z&&actors[i]->world[3]==actors[i]->mPosition.x&&actors[i]->world[7]==actors[i]->mPosition.y&&actors[i]->world[11]==actors[i]->mPosition.z,"Actor matrix translation diverged from state position");float actual[]={bodies[i]->mPosition.x,bodies[i]->mPosition.y,bodies[i]->mPosition.z};for(size_t c=0;c<3;++c)assetCheck(std::abs(actual[c]-expected[i][c])<.01f,"Authored actor sensor world transform mismatch");}assetCheck(state.calls==actors.size()*3,"Placed actor state call count mismatch");if(!actors.empty()){auto actor=actors.front();auto previous=actor->world;float angle=actor->localRotationDegrees[0];actor->localRotationDegrees[0]=std::numeric_limits<float>::quiet_NaN();bool rejected=false;try{actor->syncTransform();}catch(const std::exception&){rejected=true;}assetCheck(rejected&&actor->world==previous,"Invalid rotation changed committed actor matrix");actor->localRotationDegrees[0]=angle;float scale=actor->localScale[0];actor->localScale[0]=std::numeric_limits<float>::infinity();rejected=false;try{actor->syncTransform();}catch(const std::exception&){rejected=true;}assetCheck(rejected&&actor->world==previous,"Invalid scale changed committed actor matrix");actor->localScale[0]=scale;actor->syncTransform();}return actors.size();
}






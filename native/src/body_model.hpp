#pragma once
#include "animation_clock.hpp"
#include "mesh_render.hpp"
#include "collision_geometry.hpp"
#include "diagnostic_motion.hpp"
#include "animation_track_blend.hpp"
#include "mario_animation_groups.hpp"
#include "mario_animation_defaults.hpp"
#include "mario_walk_targets.hpp"
// Visual diagnostic attached to the sphere, not an implementation of MarioActor.
class DiagnosticMarioModel {
    std::vector<uint8_t> bytes;ModelInfo model;ModelRig rig;ModelMesh mesh;std::unique_ptr<RetainedMeshAppearance> appearance;mutable std::optional<std::vector<RenderVertex>> localGeometry;mutable size_t geometryBuilds=0;
    using Motion=DiagnosticMotion;
    std::unique_ptr<Rarc> animations;std::shared_ptr<NativeMarioClipCache> clipCache;std::shared_ptr<const JointAnimation> idle,running,rising,falling;AnimationClock clock;Motion motion=Motion::Idle;float scale=1,foot=0;
    std::unique_ptr<NativeBckSettingsTable> settings;std::unique_ptr<NativeMarioGroupPlayer> group;
    NativeMarioSpecialInertiaState inertiaState;float walkStartBlend=0;NativeMarioWalkSpeedState walkSpeedState;unsigned walkIndex=0;float stickMagnitude=0;bool diagnosticCadence=false;std::unique_ptr<DiagnosticMotionController> controller;

    int motionTransitionFrames(Motion state)const{const auto& control=settings->select(state==Motion::Rise?"Jump":state==Motion::Fall?"Fall":"Run");return control.mInterpole>=0?control.mInterpole:6;}
    void configureMotionGroup(Motion state){if(state==Motion::Rise||state==Motion::Fall)group->configureTracks({settings->select(state==Motion::Rise?"Jump":"Fall")});else group->configureTracks({settings->select("WalkSoft"),settings->select("Walk"),settings->select("Run"),settings->select("Wait")});}
    const JointAnimation& clip()const{switch(motion){case Motion::Run:return *running;case Motion::Rise:return *rising;case Motion::Fall:return *falling;default:return *idle;}}
    ModelRig currentPose()const{assetCheck(bool(group),"Missing diagnostic Mario playback group");return group->pose();}
public:
    explicit DiagnosticMarioModel(WiiAssets& assets):animations(std::make_unique<Rarc>(assets.readFile("/ObjectData/MarioAnime.arc"))),clipCache(std::make_shared<NativeMarioClipCache>(*animations)),idle(clipCache->getShared("Wait")),running(clipCache->getShared("Run")),rising(clipCache->getShared("Jump")),falling(clipCache->getShared("Fall")),clock(*idle){
        Rarc archive(assets.readFile("/ObjectData/Mario.arc"));bytes=archive.readFile("/mario.bdl");BinaryView view(bytes);model=decodeModel(view);rig=decodeRig(view);mesh=decodeMesh(view,model,rig);appearance=std::make_unique<RetainedMeshAppearance>(view,model,mesh);
        auto controlBytes=animations->readFile("/marioanime.banmt");settings=std::make_unique<NativeBckSettingsTable>(BinaryView(controlBytes));if(!clipCache)clipCache=std::make_shared<NativeMarioClipCache>(*animations);group=std::make_unique<NativeMarioGroupPlayer>(*animations,rig,resolveMarioGroup(quadAnimeTable[0].mParent.mAnimationName),clipCache);configureMotionGroup(Motion::Idle);group->useOriginalSharedPhase();group->useFrameBeforeMovement();group->setWeights({0,0,0,1});
        auto vertices=defaultPoseTriangles(model,rig,mesh);float low=INFINITY,high=-INFINITY;for(auto vertex:vertices){low=std::min(low,vertex.position[1]);high=std::max(high,vertex.position[1]);}assetCheck(high>low,"Empty diagnostic Mario bounds");scale=1/(high-low);foot=low;
    }
    static JointAnimation loadAnimation(const Rarc& archive,const char* path){auto data=archive.readFile(path);return decodeBck(BinaryView(data));}
    const std::vector<TextureImage>& textures()const{return model.textures;}
    void enableDiagnosticCadence(bool enabled=true){diagnosticCadence=enabled;}
    unsigned currentWalkIndex()const{return walkIndex;}
    void verify(){
        auto pose=currentPose();auto cached=materialPoseTriangles(BinaryView(bytes),model,pose,mesh,&appearance->materials,&appearance->vertices),reference=materialPoseTriangles(BinaryView(bytes),model,pose,mesh);assetCheck(cached.size()==reference.size(),"Cached Mario material topology mismatch");for(size_t i=0;i<cached.size();++i)assetCheck(cached[i].position==reference[i].position&&cached[i].normal==reference[i].normal&&cached[i].uv==reference[i].uv&&cached[i].color==reference[i].color&&cached[i].texture==reference[i].texture,"Cached Mario material vertex mismatch");
        {auto previousMagnitude=stickMagnitude;auto previousMotion=motion;auto previousFrame=clock.frame;auto previousWeights=group->trackWeights();auto previousBuilds=geometryBuilds;for(float bad:{std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){bool rejected=false;try{tick(true,false,bad,.5f);}catch(const std::exception&){rejected=true;}assetCheck(rejected&&stickMagnitude==previousMagnitude&&motion==previousMotion&&clock.frame==previousFrame&&group->trackWeights()==previousWeights&&geometryBuilds==previousBuilds,"Rejected animation velocity partially changed state");}}
        auto initialDecodes=clipCache->decodes;assetCheck(clipCache->getShared("Wait")==idle&&clipCache->getShared("Run")==running&&clipCache->getShared("Jump")==rising&&clipCache->getShared("Fall")==falling&&clipCache->decodes==initialDecodes,"Diagnostic legacy clips are not shared with grouped cache");
        auto savedInertia=inertiaState;inertiaState={};auto savedStartBlend=walkStartBlend;walkStartBlend=0;auto savedSpeed=walkSpeedState;walkSpeedState={};auto savedIndex=walkIndex;auto savedMagnitude=stickMagnitude;localGeometry.reset();auto savedGroup=std::move(group);auto controlBytes=animations->readFile("/marioanime.banmt");settings=std::make_unique<NativeBckSettingsTable>(BinaryView(controlBytes));if(!clipCache)clipCache=std::make_shared<NativeMarioClipCache>(*animations);group=std::make_unique<NativeMarioGroupPlayer>(*animations,rig,resolveMarioGroup(quadAnimeTable[0].mParent.mAnimationName),clipCache);configureMotionGroup(Motion::Idle);group->useOriginalSharedPhase();group->useFrameBeforeMovement();auto savedClock=clock;auto savedMotion=motion;auto savedController=std::move(controller);motion=Motion::Run;clock=AnimationClock(*running);
        tick(true,true,0,.3f);assetCheck(walkIndex==2,"Diagnostic analog walk index integration failed");tick(true,true,0,.21f);assetCheck(walkIndex==2,"Diagnostic analog hysteresis integration failed");tick(true,true,0,.19f);assetCheck(walkIndex==1,"Diagnostic analog decreasing index failed");
        auto first=triangles({0,0,0},{0,1,0},{1,0,0},{0,0,1},10,0);tick(true);tick(true);auto next=triangles({0,0,0},{0,1,0},{1,0,0},{0,0,1},10,0);
        auto beforeBuilds=geometryBuilds;auto rotated=triangles({10,20,30},{-1,0,0},{0,1,0},{0,0,1},10,0);assetCheck(geometryBuilds==beforeBuilds,"Repeated diagnostic pose render rebuilt skinning geometry");size_t changed=0;
        assetCheck(first.size()==next.size()&&next.size()==rotated.size()&&!next.empty(),"Diagnostic Mario topology changed");
        for(size_t i=0;i<next.size();++i){if(first[i].position!=next[i].position)++changed;auto p=next[i].position,r=rotated[i].position;assetCheck(std::abs(r[0]-(10-p[1]))<.001&&std::abs(r[1]-(20+p[0]))<.001&&std::abs(r[2]-(30+p[2]))<.001,"Diagnostic Mario gravity basis transform mismatch");}
        assetCheck(changed>100,"Diagnostic Mario run pose did not animate");
        tick(false,false,20);assetCheck(motion==Motion::Rise&&clock.frame==1,"Diagnostic Mario rise transition failed");
        auto jumpPose=triangles({0,0,0},{0,1,0},{1,0,0},{0,0,1},10,0);tick(true,false,-20);assetCheck(motion==Motion::Fall&&clock.frame==1,"Diagnostic Mario fall transition failed");
        auto fallPose=triangles({0,0,0},{0,1,0},{1,0,0},{0,0,1},10,0);assetCheck(jumpPose.size()==fallPose.size()&&jumpPose.front().position!=fallPose.front().position,"Diagnostic airborne clips indistinguishable");
        tick(true,true,0);assetCheck(motion==Motion::Run&&clock.frame==1,"Diagnostic Mario landing to run failed");tick(false,true,0);assetCheck(motion==Motion::Idle&&clock.frame==1,"Diagnostic Mario landing to idle failed");tick(false,true,0);assetCheck(clock.frame==2,"Diagnostic unchanged clip restarted");for(int i=0;i<5;++i)tick(false,true,0);assetCheck(jointWorldMatrices(currentPose()).size()==rig.joints.size(),"Diagnostic grouped transition pose invalid");auto cachedDecodes=clipCache->decodes;tick(false,false,20);tick(false,false,-20);tick(false,true,0);assetCheck(clipCache->decodes==cachedDecodes,"Repeated diagnostic motion redecoded cached clips");inertiaState=savedInertia;walkStartBlend=savedStartBlend;walkSpeedState=savedSpeed;walkIndex=savedIndex;stickMagnitude=savedMagnitude;group=std::move(savedGroup);localGeometry.reset();clock=savedClock;motion=savedMotion;controller=std::move(savedController);
    }
    void tick(bool movement,bool grounded=true,float verticalSpeed=0,float magnitude=-1){assetCheck(magnitude==-1||(std::isfinite(magnitude)&&magnitude>=0&&magnitude<=1),"Invalid diagnostic stick magnitude");assetCheck(std::isfinite(verticalSpeed),"Invalid diagnostic animation velocity");stickMagnitude=magnitude<0?(movement?1:0):magnitude;auto next=grounded?(movement?Motion::Run:Motion::Idle):(verticalSpeed>0?Motion::Rise:Motion::Fall);
        if(!controller)controller=std::make_unique<DiagnosticMotionController>([this](Motion state,bool first){if(group){if(first&&state!=motion){bool airborne=state==Motion::Rise||state==Motion::Fall;bool wasAirborne=motion==Motion::Rise||motion==Motion::Fall;if(airborne||wasAirborne)group->replaceGroup(*animations,airborne?std::vector<NativeMarioGroupTrack>{{state==Motion::Rise?"Jump":"Fall",1}}:resolveMarioGroup(quadAnimeTable[0].mParent.mAnimationName),0);if(airborne||wasAirborne)configureMotionGroup(state);group->beginOriginalPoseTransition(motionTransitionFrames(state));}if(state==Motion::Idle||state==Motion::Run){walkIndex=nativeMarioWalkIndex(stickMagnitude,walkIndex);auto targets=nativeMarioWalkTargets(walkIndex);group->approachWeights(std::vector<float>(targets.begin(),targets.end()),NativeMarioAnimationDefaults::walk);if(diagnosticCadence){auto cadence=nativeMarioDefaultCadence();cadence.index=walkIndex;cadence.targetSpeed=nativeMarioTargetWalkSpeed(walkIndex);inertiaState.walkSpeed=walkSpeedState.speed;NativeMarioInertiaContext inertiaContext;inertiaContext.target=cadence.targetSpeed;inertiaContext.startBlend=walkStartBlend;float inertia=nativeMarioDecideInertia(inertiaState,inertiaContext);walkSpeedState.speed=inertiaState.walkSpeed;walkSpeedState.update(cadence.targetSpeed,inertia,NativeMarioAnimationDefaults::slowStart);cadence.walkSpeed=walkSpeedState.speed;walkStartBlend=group->applyWalkCadence(cadence).startBlend;}}group->tick();}motion=state;if(first)clock=AnimationClock(clip());clock.tick();});controller->tick(next);localGeometry.reset();}
    std::vector<RenderVertex> triangles(const CollisionPoint& center,const CollisionPoint& up,const CollisionPoint& right,const CollisionPoint& forward,float radius,int textureOffset)const{
        if(!localGeometry){auto pose=currentPose();localGeometry=materialPoseTriangles(BinaryView(bytes),model,pose,mesh,&appearance->materials,&appearance->vertices);++geometryBuilds;}auto vertices=*localGeometry;float size=2*radius*scale;
        AffineMatrix matrix{};for(size_t k=0;k<3;++k){matrix[k*4]=right[k]*size;matrix[k*4+1]=up[k]*size;matrix[k*4+2]=forward[k]*size;matrix[k*4+3]=center[k]-up[k]*(radius+foot*size);}
        for(auto& vertex:vertices){vertex.position=transformPoint(matrix,vertex.position);vertex.normal=skinNormal(matrix,vertex.normal,vertex.normal);if(vertex.texture>=0)vertex.texture+=textureOffset;for(float value:vertex.position)assetCheck(std::isfinite(value),"Non-finite diagnostic Mario position");}return vertices;
    }
};























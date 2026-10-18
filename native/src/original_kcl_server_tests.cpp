#include "native_kcl_storage.hpp"
#include "native_dynamic_kcl.hpp"
#include <Game/Map/CollisionParts.hpp>
#include <Game/Map/HitInfo.hpp>
#include <Game/LiveActor/Binder.hpp>
#include <Game/Util/MapUtil.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <fstream>
#include <iterator>
#include <cassert>
#include <cstdio>
extern "C" float PSVECDotProduct(const Vec* a,const Vec* b){return a->x*b->x+a->y*b->y+a->z*b->z;}
#ifdef NATIVE_COLLISION_SCENE
#include "native_collision_scene.hpp"
#include <Native/CollisionCamera.hpp>
#include <Game/Map/CollisionDirector.hpp>
#include <Game/Camera/CameraPolygonCodeUtil.hpp>
#include <Game/Map/CollisionCode.hpp>
#include <Game/Scene/SceneFunction.hpp>
#include <Game/Util/ObjUtil.hpp>
class MovementProbe:public NameObj{
public:MovementProbe(std::vector<int>& events,int value):NameObj("movement probe"),events(events),value(value){}
void movement()override{events.push_back(value);}
void initAfterPlacement()override{events.push_back(value+100);}
std::vector<int>& events;int value;
};
#endif
int main(int argc,char** argv){
assert(!MR::isFloorPolygonCos(TVec3f(0,0,0),TVec3f(0,-1,0),0));
assert(MR::isFloorPolygonCos(TVec3f(0,1,0),TVec3f(0,-1,0),1));
assert(!MR::isFloorPolygonCos(TVec3f(0,1,0),TVec3f(0,-1,0),1.001f));
assert(!MR::isFloorPolygonCos(TVec3f(0,-1,0),TVec3f(0,-1,0),0));
assert(MR::isFloorPolygonCos(TVec3f(0,.4f,.916515f),TVec3f(0,-1,0),.4f));
assert(!MR::isFloorPolygonCos(TVec3f(0,.4f,.916515f),TVec3f(0,-1,0),.401f));
assert(!MR::isFloorPolygonCos(TVec3f(0,.3f,.953939f),TVec3f(0,-1,0),.2f));
std::puts("PASS original floor cosine classification: zero, floor/roof, inclusive angle threshold and wall exclusion");
#ifdef NATIVE_COLLISION_SCENE
NativeCollisionScene scene(2);assert(scene.director()==MR::getCollisionDirector());assert(scene.globals.create(SceneObj_CollisionDirector)==scene.director());
std::vector<int> movementEvents;MovementProbe mapFirst(movementEvents,2),collisionLast(movementEvents,1),mapSecond(movementEvents,3),switchLast(movementEvents,0),sensorLast(movementEvents,-1);
MR::connectToScene(&mapFirst,MR::MovementType_MapObj,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);
MR::connectToScene(&collisionLast,MR::MovementType_CollisionDirector,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);
MR::connectToScene(&mapSecond,MR::MovementType_MapObj,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);
MR::connectToScene(&switchLast,MR::MovementType_SwitchWatcherHolder,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);
MR::connectToScene(&sensorLast,MR::MovementType_SensorHitChecker,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);scene.tick();assert((movementEvents==std::vector<int>{-1,0,1,2,3}));movementEvents.clear();
scene.finishPlacement();assert((movementEvents==std::vector<int>{102,101,103,100,99}));movementEvents.clear();
bool duplicateRejected=false;try{MR::connectToScene(&mapFirst,MR::MovementType_MapObj,MR::CalcAnimType_None,MR::DrawBufferType_None,MR::DrawType_None);}catch(const std::logic_error&){duplicateRejected=true;}assert(duplicateRejected);
mapFirst.requestSuspend();scene.movementFlagsPending=true;scene.tick();assert((movementEvents==std::vector<int>{-1,0,1,3}));movementEvents.clear();
mapFirst.requestResume();scene.movementFlagsPending=true;
scene.tick();assert((movementEvents==std::vector<int>{-1,0,1,2,3}));movementEvents.clear();
std::puts("PASS native movement category order, stable registration, duplicate rejection and suspend/resume");
#endif
assert(argc==3);std::ifstream in(argv[1],std::ios::binary);assert(in);std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});std::ifstream metadataInput(argv[2],std::ios::binary);assert(metadataInput);std::vector<std::uint8_t> metadataBytes((std::istreambuf_iterator<char>(metadataInput)),{});
#ifdef NATIVE_COLLISION_SCENE
TPos3f placement;placement.set(0,-2,0,100,2,0,0,-50,0,0,2,25);
CollisionParts& parts=scene.loadTerrain(bytes,metadataBytes,placement,"native terrain",1);KCollisionServer& server=*parts.mServer;JMapInfo& attributes=*server.mapInfo;assert(scene.terrain.size()==1 && parts.mZone->mZoneID==1);
const auto groupsBefore=scene.pendingCameraGroups.size();{JMapInfo absentMetadata;KCollisionServer withoutMetadata(bytes,&absentMetadata);Native::CollisionCameraCollection collection("metadata optional",1);assert(!withoutMetadata.calcFarthestVertexDistance());collection.commit();assert(std::isfinite(withoutMetadata.mMaxVertexDistance)&&std::abs(withoutMetadata.mMaxVertexDistance-server.mMaxVertexDistance)<.01f);assert(scene.pendingCameraGroups.size()==groupsBefore);}try{Native::CollisionCameraCollection failed("failed actor",1);MR::registerCameraCode(7);throw std::runtime_error("injected placement failure");}catch(const std::runtime_error&){}assert(scene.cameraCollectionZone==-1&&scene.cameraCollectionHost.empty()&&scene.pendingCameraGroups.size()==groupsBefore);{Native::CollisionCameraCollection retry("retry actor",1);MR::registerCameraCode(7);retry.commit();}assert(scene.cameraCollectionZone==-1&&scene.pendingCameraGroups.size()==groupsBefore+1);scene.pendingCameraGroups.resize(groupsBefore);
auto expectLoadFailure=[&](const std::vector<std::uint8_t>& k,const std::vector<std::uint8_t>& m,const TPos3f& transform){bool rejected=false;try{scene.loadTerrain(k,m,transform,"rejected terrain",1);}catch(const std::exception&){rejected=true;}assert(rejected && scene.terrain.size()==1 && scene.director()->getCategoryKeeper(0)->mPartsCount==1 && scene.pendingCameraGroups.size()==groupsBefore && scene.cameraCollectionZone==-1);};
expectLoadFailure(std::vector<std::uint8_t>(16),metadataBytes,placement);
expectLoadFailure(bytes,std::vector<std::uint8_t>(4),placement);
auto noRows=metadataBytes;for(int h=0;h<4;++h)noRows[h]=0;expectLoadFailure(bytes,noRows,placement);
TPos3f singular;singular.set(0,0,0,0,0,0,0,0,0,0,0,0);expectLoadFailure(bytes,metadataBytes,singular);
std::puts("PASS native terrain loader: asset ownership, original initialization, registration and failed-load rollback");
#else
JMapInfo attributes;assert(attributes.attachNative(metadataBytes));KCollisionServer server(bytes,&attributes);CollisionParts parts;delete parts.mServer->mapInfo;delete parts.mServer;parts.mServer=&server;parts.mBaseMatrix.set(0,-2,0,100,2,0,0,-50,0,0,2,25);assert(PSMTXInverse(parts.mBaseMatrix.mMtx,parts.mInvBaseMatrix.mMtx));
#endif
{
 KC_PrismData* target=nullptr;for(int i=0;i<server.getTriangleNum();++i){auto* prism=server.getPrismData(i);if(prism->mHeight>0){target=prism;break;}}assert(target);
 auto center=(server.getPos(target,0)+server.getPos(target,1)+server.getPos(target,2))/3.0f;auto normal=*server.getFaceNormal(target);
 Fxyz point;auto inside=center-normal*.1f;point.x=inside.x;point.y=inside.y;point.z=inside.z;float pointDepth=-1;assert(server.checkPoint(&point,1,&pointDepth)==target);assert(std::abs(pointDepth-.1f)<.02f);
 Fxyz boxMin,boxMax;boxMin.x=center.x-1;boxMin.y=center.y-1;boxMin.z=center.z-1;boxMax.x=center.x+1;boxMax.y=center.y+1;boxMax.z=center.z+1;
 std::vector<KC_PrismData*> areaHits(server.getTriangleNum());auto areaCount=server.checkArea3D(&boxMin,&boxMax,areaHits.data(),static_cast<u32>(areaHits.size()));assert(areaCount>0);assert(std::find(areaHits.begin(),areaHits.begin()+areaCount,target)!=areaHits.begin()+areaCount);
 Fxyz sphere;sphere.x=inside.x;sphere.y=inside.y;sphere.z=inside.z;std::vector<KC_PrismData*> thickHits(server.getTriangleNum());std::vector<float> thickDepths(thickHits.size());std::vector<u8> thickFlags(thickHits.size());
 auto thickCount=server.checkSphereWithThickness(&sphere,1.0f,1.0f,static_cast<u32>(thickHits.size()),thickHits.data(),thickDepths.data(),thickFlags.data(),40.0f);assert(thickCount>0);assert(std::find(thickHits.begin(),thickHits.begin()+thickCount,target)!=thickHits.begin()+thickCount);
 std::puts("PASS original KCollision point, area and sphere-with-thickness queries against retail geometry");
}
{
 KC_PrismData* source=nullptr;
 for(int i=0;i<server.getTriangleNum();++i){auto* candidate=server.getPrismData(i);if(candidate->mHeight>0){source=candidate;break;}}
 assert(source);
 TVec3f dynamicPosition[1]={server.getPos(source,0)};
 TVec3f dynamicNormals[4]={*server.getFaceNormal(source),*server.getEdgeNormal1(source),*server.getEdgeNormal2(source),*server.getEdgeNormal3(source)};
 KC_PrismData dynamicPrisms[2]{};dynamicPrisms[1]=*source;
 dynamicPrisms[1].mPositionIndex=0;dynamicPrisms[1].mNormalIndex=0;
 for(int j=0;j<3;++j)dynamicPrisms[1].mEdgeIndices[j]=j+1;
 KCLFile dynamicFile{};dynamicFile.mPos=dynamicPosition;dynamicFile.mNorms=dynamicNormals;dynamicFile.mPrisms=dynamicPrisms;
 dynamicFile.mThickness=40;dynamicFile.mMin=dynamicPosition[0]-TVec3f(10000,10000,10000);
 dynamicFile.mXMask=dynamicFile.mYMask=dynamicFile.mZMask=-2147483647-1;
 dynamicFile.mBlockWidthShift=31;dynamicFile.mBlockXShift=dynamicFile.mBlockXYShift=-1;
 JMapInfo dynamicAttributes;KCollisionServer moving(encodeDynamicKcl(dynamicFile,1,4,1),&dynamicAttributes);
 auto* held=moving.getPrismData(0);auto* heldNormal=moving.getFaceNormal(held);
 auto center=(moving.getPos(held,0)+moving.getPos(held,1)+moving.getPos(held,2))/3.0f;
 auto normal=*heldNormal;auto rayStart=center+normal*10.0f;auto rayDirection=normal*-20.0f;
 float fraction=0;u8 flag=0;assert(moving.KCHitArrow(held,rayStart,rayDirection,&fraction,&flag));assert(std::abs(fraction-.5f)<.002f);
 dynamicPosition[0]+=normal*100.0f;
 moving.updateNativeDynamicData(encodeDynamicKcl(dynamicFile,1,4,1));
 assert(moving.getPrismData(0)==held && moving.getFaceNormal(held)==heldNormal);
 assert(!moving.KCHitArrow(held,rayStart,rayDirection,&fraction,&flag));
 assert(moving.KCHitArrow(held,rayStart+normal*100.0f,rayDirection,&fraction,&flag));assert(std::abs(fraction-.5f)<.002f);
 Fxyz sphere;auto sphereCenter=center+normal*105.0f;sphere.x=sphereCenter.x;sphere.y=sphereCenter.y;sphere.z=sphereCenter.z;
 float penetration=0;assert(moving.KCHitSphere(held,&sphere,10,1,&penetration,&flag));assert(std::abs(penetration-5)<.02f);
 std::puts("PASS original dynamic collision queries: moved triangle old-ray miss, new-ray hit, sphere contact and retained references");
 float indexedFraction=1;u8 indexedFlag=0;
 assert(!moving.checkArrow(rayStart,rayDirection,&indexedFraction,&indexedFlag,nullptr,nullptr,0));
 indexedFraction=1;
 assert(moving.checkArrow(rayStart+normal*100.0f,rayDirection,&indexedFraction,&indexedFlag,nullptr,nullptr,0)==held);
 assert(std::abs(indexedFraction-.5f)<.002f);
 KC_PrismData* indexedPrisms[1]{};float depths[1]{};u8 flags[1]{};
 assert(moving.checkSphere(&sphere,10,1,1,indexedPrisms,depths,flags)==1);
 assert(indexedPrisms[0]==held && std::abs(depths[0]-5)<.02f);
 // Rebase the spatial header and verify indexed queries use the refreshed view.
 dynamicFile.mMin+=normal*100.0f;
 moving.updateNativeDynamicData(encodeDynamicKcl(dynamicFile,1,4,1));
 indexedFraction=1;
 assert(moving.checkArrow(rayStart+normal*100.0f,rayDirection,&indexedFraction,&indexedFlag,nullptr,nullptr,0)==held);
 assert(moving.checkSphere(&sphere,10,1,1,indexedPrisms,depths,flags)==1);
 std::puts("PASS dynamic indexed line/sphere queries and spatial-header refresh");
}
TVec3f bindPosition(0,0,0),gravity(0,-1,0);Binder binder(parts.mBaseMatrix.mMtx,&bindPosition,&gravity,20,0,0);std::size_t hits=0,skipped=0;
assert(binder.copyPlaneArrayAndSortingSensor(nullptr,0)==0);
binder._C8=1;binder._158=2;binder._1E8=3;
HitInfo* output[3]={nullptr,nullptr,nullptr};bool capacityRejected=false;
try{binder.copyPlaneArrayAndSortingSensor(output,2);}catch(const std::length_error&){capacityRejected=true;}assert(capacityRejected&&output[0]==nullptr&&output[1]==nullptr&&output[2]==nullptr);
assert(binder.copyPlaneArrayAndSortingSensor(output,3)==3);
assert(std::find(output,output+3,&binder.mGroundInfo)!=output+3&&std::find(output,output+3,&binder.mWallInfo)!=output+3&&std::find(output,output+3,&binder.mRoofInfo)!=output+3);
binder.clear();
Binder stored(parts.mBaseMatrix.mMtx,&bindPosition,&gravity,20,0,2);stored.mPlaneNum=2;
assert(stored.getPlane(0)==stored.mPlane&&stored.getPlane(1)==stored.mPlane+1);
assert(stored.copyPlaneArrayAndSortingSensor(output,2)==2);
delete[] stored.mPlane;
std::puts("PASS Binder plane access/copy: fallback contacts, stored planes and atomic capacity rejection");
// Original movement updates must keep inverse queries and previous transforms coherent.
TPos3f authoredMatrix(parts.mBaseMatrix);authoredMatrix.mMtx[0][3]+=7;
const float priorBaseX=parts.mBaseMatrix.mMtx[0][3];parts.setMtx(authoredMatrix);
assert(parts.mMatrix.mMtx[0][3]==priorBaseX+7&&parts.mBaseMatrix.mMtx[0][3]==priorBaseX);
TPos3f sourceMatrix(parts.mBaseMatrix);parts._0=&sourceMatrix;parts.setMtx();assert(parts.mMatrix.mMtx[0][3]==priorBaseX);
parts.updateBoundingSphereRange();assert(std::abs(parts._DC-2)<.001f&&std::abs(parts._D8-2*server.mMaxVertexDistance)<.001f);
std::puts("PASS original collision authored/source matrix setters and uniform-scale bounds");
#ifdef NATIVE_COLLISION_SCENE
sourceMatrix.mMtx[0][3]+=7;parts._CD=false;parts._CE=false;parts.resetAllMtx();assert(parts.mBaseMatrix.mMtx[0][3]==priorBaseX);
parts._CE=true;parts.resetAllMtx();assert(parts.mBaseMatrix.mMtx[0][3]==priorBaseX+7&&parts.mPrevBaseMatrix.mMtx[0][3]==priorBaseX+7&&parts.mMatrix.mMtx[0][3]==priorBaseX+7);
TVec3f resetPoint;parts.mInvBaseMatrix.mult(TVec3f(priorBaseX+7,-50,25),resetPoint);assert(resetPoint.length()<.001f);
sourceMatrix.mMtx[0][3]=priorBaseX+9;parts._CE=false;parts.forceResetAllMtxAndSetUpdateMtxOneTime();assert(parts._CE&&parts.mBaseMatrix.mMtx[0][3]==priorBaseX+9);
sourceMatrix.mMtx[0][3]=priorBaseX;parts._CD=true;parts._CE=false;parts.resetAllMtx();
std::puts("PASS original collision source reset: disabled gate, forced reset, matrix history and inverse");
#endif
TPos3f initialMatrix(parts.mBaseMatrix);parts.mMatrix.setInline(initialMatrix);parts._D4=0;parts.updateMtx();assert(parts._D4==1);
parts.mMatrix.mMtx[0][3]+=40;parts.updateMtx();assert(parts._D4==0 && parts.mBaseMatrix.mMtx[0][3]==140 && parts.mPrevBaseMatrix.mMtx[0][3]==100);
TVec3f movedPoint,roundTrip;parts.mBaseMatrix.mult(TVec3f(4,8,12),movedPoint);parts.mInvBaseMatrix.mult(movedPoint,roundTrip);assert((roundTrip-TVec3f(4,8,12)).length()<.001f);
parts.updateMtx();assert(parts._D4==1);parts.updateMtx();assert(parts._D4==2);
parts._CD=false;parts.mMatrix.mMtx[0][3]=200;parts.updateMtx();assert(parts.mBaseMatrix.mMtx[0][3]==140);
parts._CE=true;parts.updateMtx();assert(parts._D4==1 && !parts._CE && parts.mBaseMatrix.mMtx[0][3]==200);
parts._CD=true;parts.mMatrix.set(3,0,0,0,0,3,0,0,0,0,3,0);parts.updateMtx();assert(parts._DC==3 && std::abs(parts._D8-3*server.mMaxVertexDistance)<.001f);
parts.mMatrix.setInline(initialMatrix);parts.updateMtx();assert(parts._DC==2);parts._D4=0;
std::puts("PASS original collision matrix update: movement history, stationary frames, forced update, inverse and scale bounds");
// Exercise original zone registration with translated collision parts before prism queries.
parts._D8=30.0f;CollisionZone zone(1);zone.addParts(&parts);
assert(zone.mParts.size()==1 && zone.mParts[0]==&parts);
assert((zone.mCenter-TVec3f(100,-50,25)).length()<.001f);
assert((zone.mMin-TVec3f(70,-80,-5)).length()<.001f);
assert((zone.mMax-TVec3f(130,-20,55)).length()<.001f);
assert(std::abs(zone.mRadius-30.0f)<.001f);
CollisionParts other;other.mBaseMatrix.set(1,0,0,-100,0,1,0,50,0,0,1,-25);other._D8=10;
zone.addParts(&other);assert(zone.mParts.size()==2);
assert((zone.mMin-TVec3f(-110,-80,-35)).length()<.001f);
assert((zone.mMax-TVec3f(130,60,55)).length()<.001f);
for(auto* part:zone.mParts){assert((part->getTrans()-zone.mCenter).length()+part->_D8<=zone.mRadius+.001f);}
zone.eraseParts(&parts);assert(zone.mParts.size()==1 && zone.mParts[0]==&other);
zone.calcMinMaxAndRadius();assert((zone.mCenter-other.getTrans()).length()<.001f && std::abs(zone.mRadius-10)<.001f);
zone.eraseParts(&parts);assert(zone.mParts.size()==1);
zone.eraseParts(&other);zone.calcMinMaxAndRadius();assert(zone.mParts.size()==0 && zone.mRadius==0);
CollisionZone global(0);global.addParts(&parts);assert(global.mParts.size()==1 && global.mRadius==0);global.eraseParts(&parts);
delete other.mServer->mapInfo;delete other.mServer;other.mServer=nullptr;
std::puts("PASS original collision zone registration, removal, global-zone handling and transformed-part bounds");

#ifdef NATIVE_COLLISION_SCENE
float extent=0;for(int i=0;i<server.getTriangleNum();++i)for(int v=0;v<3;++v)extent=std::max(extent,server.getPos(server.getPrismData(i),v).length());
assert(std::abs(server.mMaxVertexDistance-extent)<.01f);
std::array<bool,255> expectedCodes{};for(int i=0;i<server.getTriangleNum();++i){auto code=scene.director()->mCode->getCameraID(server.getAttributes(i));if(code<255)expectedCodes[code]=true;}
std::size_t expectedCodeCount=0;for(bool present:expectedCodes)expectedCodeCount+=present;assert(scene.pendingCameraGroups.size()==expectedCodeCount);
for(const auto& group:scene.pendingCameraGroups){assert(group.host=="native terrain" && group.zone==1 && group.code<255 && expectedCodes[group.code]);expectedCodes[group.code]=false;}
MR::initCameraCodeCollection("collection check",0);MR::registerCameraCode(7);MR::registerCameraCode(7);MR::registerCameraCode(254);MR::registerCameraCode(255);MR::termCameraCodeCollection();assert(scene.pendingCameraGroups.size()==expectedCodeCount+2);assert(scene.pendingCameraGroups[expectedCodeCount].code==7 && scene.pendingCameraGroups.back().code==254);parts.updateBoundingSphereRangePrivate(2);parts._CC=true;
auto* keeper=scene.director()->getCategoryKeeper(0);scene.tick();assert(keeper->mPartsCount==1);parts.removeFromBelongZone();assert(keeper->mPartsCount==0);parts.addToBelongZone();assert(keeper->mPartsCount==1);scene.tick();
#endif
 for(int i=0;i<server.getTriangleNum();++i){auto prism=server.getPrismData(i);auto iter=server.getAttributes(i);assert(iter.isValid() && iter.mInfo==&attributes && iter.mIndex==prism->mAttribute);if(prism->mHeight<=0){++skipped;continue;}auto a=server.getPos(prism,0),b=server.getPos(prism,1),c=server.getPos(prism,2);auto center=(a+b+c)/3.0f;auto normal=*server.getFaceNormal(prism);auto origin=center+normal*10.0f;auto direction=normal*-20.0f;float t=0;u8 flag=0;assert(server.KCHitArrow(prism,origin,direction,&t,&flag));assert(std::abs(t-.5f)<.002f);
 float nearest=1;KC_PrismData* expected=nullptr;for(int j=0;j<server.getTriangleNum();++j){float fraction;u8 f;auto candidate=server.getPrismData(j);if(candidate->mHeight>0 && server.KCHitArrow(candidate,origin,direction,&fraction,&f) && fraction<nearest){nearest=fraction;expected=candidate;}}
 float indexed=1;u8 indexedFlag=0;auto found=server.checkArrow(origin,direction,&indexed,&indexedFlag,nullptr,nullptr,0);assert(bool(found)==bool(expected));assert(!found||std::abs(indexed-nearest)<.002f);
 TVec3f worldOrigin,worldEnd;parts.mBaseMatrix.mult(origin,worldOrigin);parts.mBaseMatrix.mult(origin+direction,worldEnd);HitInfo worldHits[64];auto worldCount=parts.checkStrikeLine(worldHits,64,worldOrigin,worldEnd-worldOrigin,nullptr);assert(worldCount>0);float worldNearest=1;for(u32 k=0;k<worldCount;++k){float fraction=worldHits[k]._60/(worldEnd-worldOrigin).length();if(fraction<worldNearest)worldNearest=fraction;assert(worldHits[k].mParentTriangle.mParts==&parts);}assert(std::abs(worldNearest-nearest)<.002f);assert(!server.KCHitArrow(prism,center-normal*10.0f,normal*20.0f,&t,&flag));
 Fxyz sphere;auto point=center+normal*5.0f;sphere.x=point.x;sphere.y=point.y;sphere.z=point.z;float penetration=0;u8 sphereFlag=0;
 assert(server.KCHitSphere(prism,&sphere,10.0f,1.0f,&penetration,&sphereFlag));assert(sphereFlag==1 && std::abs(penetration-5.0f)<.01f);
 TVec3f worldSphere;parts.mBaseMatrix.mult(point,worldSphere);HitInfo sphereHits[64];auto partSphereCount=parts.checkStrikeBall(sphereHits,64,worldSphere,20.0f,false,nullptr);bool matchedPrism=false;
 for(u32 k=0;k<partSphereCount;++k){assert(sphereHits[k].mParentTriangle.mParts==&parts);if(sphereHits[k].mParentTriangle.mIdx==u32(i)){matchedPrism=true;assert(std::abs(sphereHits[k]._60-10.0f)<.02f && sphereHits[k]._88==1);
 binder.mPlaneNum=1;TVec3f reaction;binder.obtainMomentFixReaction(&sphereHits[k],1,&reaction,0);auto expectedReaction=*sphereHits[k].mParentTriangle.getFaceNormal()*sphereHits[k]._60;assert((reaction-expectedReaction).length()<.001f);
 TVec3f resolved=worldSphere+reaction;TVec3f resolvedLocal;parts.mInvBaseMatrix.mult(resolved,resolvedLocal);Fxyz resolvedCenter;resolvedCenter.x=resolvedLocal.x;resolvedCenter.y=resolvedLocal.y;resolvedCenter.z=resolvedLocal.z;float residual=0;u8 residualFlag=0;bool remains=server.KCHitSphere(prism,&resolvedCenter,10,1,&residual,&residualFlag);assert(!remains||residual<.02f);
 auto face=*sphereHits[k].mParentTriangle.getFaceNormal();gravity=face*-1; binder.clear();binder.mPlaneNum=1;binder.storeContactPlane(&sphereHits[k],1);assert(binder.isBindedGround()&&!binder.isBindedWall()&&!binder.isBindedRoof());
 gravity=face;binder.clear();binder.mPlaneNum=1;binder.storeContactPlane(&sphereHits[k],1);assert(!binder.isBindedGround()&&!binder.isBindedWall()&&binder.isBindedRoof());
 TVec3f axis=std::abs(face.x)<.8f?TVec3f(1,0,0):TVec3f(0,1,0);gravity.cross(face,axis);gravity.normalize();binder.clear();binder.mPlaneNum=1;binder.storeContactPlane(&sphereHits[k],1);assert(!binder.isBindedGround()&&binder.isBindedWall()&&!binder.isBindedRoof());}}
 assert(matchedPrism);
#ifdef NATIVE_COLLISION_SCENE
HitInfo directHits[32];auto directCount=parts.checkStrikeBall(directHits,32,worldSphere,20,false,nullptr);auto sceneCount=keeper->checkStrikeBall(worldSphere,20,false,nullptr,nullptr);assert(sceneCount==directCount);
if(i<32){
 TVec3f worldNormal;PSMTXMultVecSR(parts.mBaseMatrix.mMtx,reinterpret_cast<const Vec*>(&normal),reinterpret_cast<Vec*>(&worldNormal));worldNormal.normalize();
 bindPosition=worldSphere+worldNormal*10;gravity=worldNormal*-1;binder._1EC._0=false;binder._1EC._1=false;
 auto adjusted=binder.bind(worldNormal*-10);assert(std::isfinite(adjusted.x)&&std::isfinite(adjusted.y)&&std::isfinite(adjusted.z));assert(binder.mPlaneNum>0 && binder.mFixReactionVector.length()>0);
 TVec3f boundLocal;parts.mInvBaseMatrix.mult(bindPosition+adjusted,boundLocal);Fxyz boundSphere;boundSphere.x=boundLocal.x;boundSphere.y=boundLocal.y;boundSphere.z=boundLocal.z;float boundDepth=0;u8 boundFlag=0;bool overlaps=server.KCHitSphere(prism,&boundSphere,10,1,&boundDepth,&boundFlag);assert(!overlaps||boundDepth<.02f);
 // Re-query because Binder uses the same category keeper hit buffer.
 sceneCount=keeper->checkStrikeBall(worldSphere,20,false,nullptr,nullptr);
}
for(int k=0;k<sceneCount;++k){auto* hit=keeper->getStrikeInfo(k);assert(hit->mParentTriangle.mParts==&parts && hit->mParentTriangle.mIdx==directHits[k].mParentTriangle.mIdx && std::abs(hit->_60-directHits[k]._60)<.001f);}
#endif
 std::vector<KC_PrismData*> foundSphere(server.getTriangleNum());std::vector<float> depths(foundSphere.size());std::vector<u8> flags(foundSphere.size());
 auto foundCount=server.checkSphere(&sphere,10.0f,1.0f,static_cast<u32>(foundSphere.size()),foundSphere.data(),depths.data(),flags.data());
 for(int j=0;j<server.getTriangleNum();++j){auto candidate=server.getPrismData(j);float depth=0;u8 contactFlag=0;bool contact=candidate->mHeight>0 && server.KCHitSphere(candidate,&sphere,10.0f,1.0f,&depth,&contactFlag);bool indexedContact=false;
  for(u32 k=0;k<foundCount;++k)if(foundSphere[k]==candidate){indexedContact=true;assert(std::abs(depths[k]-depth)<.01f && flags[k]==contactFlag);}
  assert(contact==indexedContact);
 }
 point=center+normal*20.0f;sphere.x=point.x;sphere.y=point.y;sphere.z=point.z;assert(!server.KCHitSphere(prism,&sphere,10.0f,1.0f,&penetration,&sphereFlag));++hits;}
 #ifdef NATIVE_COLLISION_SCENE
bindPosition=TVec3f(1e7f,1e7f,1e7f);auto freeMovement=binder.bind(TVec3f(7,-3,5));assert((freeMovement-TVec3f(7,-3,5)).length()<.001f && binder.mPlaneNum==0);
parts.removeFromBelongZone();assert(keeper->mPartsCount==0);assert(keeper->checkStrikeBall(TVec3f(0,0,0),20,false,nullptr,nullptr)==0);parts.addToBelongZone();assert(keeper->mPartsCount==1);scene.terrain.clear();assert(keeper->mPartsCount==0&&keeper->checkStrikeBall(TVec3f(0,0,0),20,false,nullptr,nullptr)==0);std::puts("PASS original collision scene director, registration, scheduling, sphere queries, Binder separation and terrain teardown");
#endif
assert(hits>0);std::printf("PASS original KCollisionServer: %zu prism-center line hits/backface misses and sphere contacts/separation, %zu skipped prisms\n",hits,skipped);
}







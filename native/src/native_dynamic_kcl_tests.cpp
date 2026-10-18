#include "native_dynamic_kcl.hpp"
#include "native_kcl_storage.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
int main() {
    TVec3f positions[2]={{1,2,3},{-4,5,6}};
    TVec3f normals[8]; for(int i=0;i<8;++i) normals[i].set(float(i),float(i+1),float(i+2));
    KC_PrismData prisms[3]{};
    for(int i=1;i<=2;++i) {auto& r=prisms[i];r.mHeight=float(i)*2.5f;r.mPositionIndex=i-1;r.mNormalIndex=(i-1)*4;r.mAttribute=i;for(int j=0;j<3;++j)r.mEdgeIndices[j]=(i-1)*4+j+1;}
    KCLFile file{};file.mPos=positions;file.mNorms=normals;file.mPrisms=prisms;
    file.mThickness=40;file.mMin.set(-4,2,3);file.mXMask=-16;file.mYMask=-8;file.mZMask=-8;
    file.mBlockWidthShift=4;file.mBlockXShift=-1;file.mBlockXYShift=-1;
    auto bytes=encodeDynamicKcl(file,2,8,2);NativeKclStorage decoded(bytes);
    assert(decoded.triangleCount()==2 && decoded.positions[1].x==-4);
    assert(decoded.normals[7].z==9 && decoded.prisms[2].mHeight==5);
    assert(decoded.prisms[2].mEdgeIndices[2]==7 && decoded.prisms[2].mAttribute==2);
    auto leaf=decoded.octree.lookup(0,0,0,4,-1,-1,2);
    assert(leaf.prisms==std::vector<std::uint16_t>({2,1}));
    positions[1].x=12;NativeKclStorage updated(encodeDynamicKcl(file,2,8,2));
    assert(updated.positions[1].x==12 && decoded.positions[1].x==-4);
    auto* heldPosition=decoded.positions.data();auto* heldNormal=decoded.normals.data();auto* heldPrism=decoded.prisms.data();
    auto heldLeaf=decoded.octree.nativeLookup(0,0,0,4,-1,-1,2).list;
    decoded.updateDynamic(encodeDynamicKcl(file,2,8,2));
    assert(decoded.positions.data()==heldPosition && decoded.normals.data()==heldNormal && decoded.prisms.data()==heldPrism);
    assert(heldPosition[1].x==12 && decoded.octree.nativeLookup(0,0,0,4,-1,-1,2).list==heldLeaf);
    auto malformed=encodeDynamicKcl(file,2,8,2);malformed.pop_back();malformed.pop_back();malformed.pop_back();
    bool failedUpdate=false;try{decoded.updateDynamic(malformed);}catch(const std::exception&){failedUpdate=true;}
    assert(failedUpdate && heldPosition[1].x==12 && decoded.prisms.data()==heldPrism);
    bool rejected=false;try{encodeDynamicKcl(file,2,7,2);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    prisms[1].mEdgeIndices[0]=8;rejected=false;try{encodeDynamicKcl(file,2,8,2);}catch(const std::out_of_range&){rejected=true;}assert(rejected);prisms[1].mEdgeIndices[0]=1;
    positions[0].x=std::numeric_limits<float>::infinity();rejected=false;try{encodeDynamicKcl(file,2,8,2);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    std::puts("PASS dynamic KCL byte encoding, original leaf order, mutable snapshots and invalid geometry rejection");
}

#pragma once
#include "mesh.hpp"
#include "animation_quaternion_math.hpp"
#include <functional>
#include <sstream>
#include <iomanip>
using AffineMatrix=std::array<float,12>;
inline AffineMatrix identityMatrix(){return {1,0,0,0,0,1,0,0,0,0,1,0};}
inline AffineMatrix concatenate(const AffineMatrix& a,const AffineMatrix& b){
    AffineMatrix c{};
    for(size_t r=0;r<3;++r){for(size_t col=0;col<3;++col)for(size_t k=0;k<3;++k)c[r*4+col]+=a[r*4+k]*b[k*4+col];
        c[r*4+3]=a[r*4+3];for(size_t k=0;k<3;++k)c[r*4+3]+=a[r*4+k]*b[k*4+3];}
    return c;
}
inline std::array<float,3> transformPoint(const AffineMatrix& m,const std::array<float,3>& p){
    std::array<float,3> out{};for(size_t r=0;r<3;++r){out[r]=m[r*4+3];for(size_t k=0;k<3;++k)out[r]+=m[r*4+k]*p[k];assetCheck(std::isfinite(out[r]),"Non-finite skinned vertex");}return out;
}
inline AffineMatrix jointRotationTranslation(const JointData& j){
    if(j.useQuaternion){auto q=j.quaternion;auto matrix=animationQuaternionMatrix({q[0],q[1],q[2],q[3]});for(size_t i=0;i<3;++i)matrix[i*4+3]=j.translation[i];return matrix;}
    constexpr double radians=6.2831853071795864769/65536.0;
    float sx=float(std::sin(j.rotation[0]*radians)),cx=float(std::cos(j.rotation[0]*radians));
    float sy=float(std::sin(j.rotation[1]*radians)),cy=float(std::cos(j.rotation[1]*radians));
    float sz=float(std::sin(j.rotation[2]*radians)),cz=float(std::cos(j.rotation[2]*radians));
    return {cz*cy,sx*cz*sy-cx*sz,cx*cz*sy+sx*sz,j.translation[0],
            sz*cy,sx*sz*sy+cx*cz,cx*sz*sy-sx*cz,j.translation[1],
            -sy,cy*sx,cy*cx,j.translation[2]};
}
inline void columnScale(AffineMatrix& m,const std::array<float,3>& scale){for(size_t r=0;r<3;++r)for(size_t c=0;c<3;++c)m[r*4+c]*=scale[c];}
inline std::vector<AffineMatrix> jointWorldMatrices(const ModelRig& rig){
    assetCheck(rig.matrixMode<=2,"Unsupported matrix calculation mode");
    std::vector<AffineMatrix> world(rig.joints.size()),current(rig.joints.size());
    std::vector<std::array<float,3>> accumulated(rig.joints.size());std::vector<uint8_t> state(rig.joints.size(),0);
    std::function<void(size_t)> evaluate=[&](size_t i){
        assetCheck(i<rig.joints.size(),"Invalid joint index");if(state[i]==2)return;assetCheck(state[i]!=1,"Joint cycle");state[i]=1;
        const auto& j=rig.joints[i];AffineMatrix parent=identityMatrix();std::array<float,3> previous{1,1,1};
        if(j.parent>=0){size_t p=size_t(j.parent);assetCheck(p<rig.joints.size(),"Invalid joint parent");evaluate(p);parent=current[p];previous=accumulated[p];}
        else assetCheck(j.parent==-1,"Invalid root joint parent");
        auto local=jointRotationTranslation(j);for(size_t k=0;k<3;++k)accumulated[i][k]=previous[k]*j.scale[k];
        if(rig.matrixMode==1){
            for(size_t k=0;k<3;++k)local[k*4+3]*=previous[k];current[i]=concatenate(parent,local);world[i]=current[i];columnScale(world[i],accumulated[i]);
        }else{
            bool applyScale=rig.matrixMode==2||accumulated[i]!=std::array<float,3>{1,1,1};if(applyScale)columnScale(local,j.scale);
            if(rig.matrixMode==2&&j.scaleCompensate){
                std::array<float,3> parentScale{1,1,1};if(j.parent>=0)parentScale=rig.joints[size_t(j.parent)].scale;
                for(size_t r=0;r<3;++r){assetCheck(parentScale[r]!=0,"Cannot compensate zero parent scale");for(size_t c=0;c<3;++c)local[r*4+c]/=parentScale[r];}
            }
            current[i]=concatenate(parent,local);world[i]=current[i];
        }
        for(float v:world[i])assetCheck(std::isfinite(v),"Non-finite world matrix");state[i]=2;
    };
    for(size_t i=0;i<rig.joints.size();++i)evaluate(i);return world;
}
inline std::vector<AffineMatrix> skinDrawMatrices(const ModelRig& rig){
    auto world=jointWorldMatrices(rig);std::vector<AffineMatrix> draw;
    for(const auto& reference:rig.drawMatrices){
        if(!reference.weighted){assetCheck(reference.index<world.size(),"Invalid rigid matrix");draw.push_back(world[reference.index]);continue;}
        assetCheck(reference.index<rig.envelopes.size(),"Invalid skin envelope");AffineMatrix blended{};double sum=0;
        for(auto weight:rig.envelopes[reference.index]){
            size_t j=weight.joint;assetCheck(j<world.size()&&j<rig.inverseBind.size()&&j<rig.inverseBindPresent.size()&&rig.inverseBindPresent[j],"Missing referenced inverse bind matrix");
            assetCheck(std::isfinite(weight.weight)&&weight.weight>=0,"Invalid runtime skin weight");auto matrix=concatenate(world[j],rig.inverseBind[j]);
            for(size_t k=0;k<12;++k)blended[k]+=weight.weight*matrix[k];sum+=weight.weight;
        }
        assetCheck(std::abs(sum-1)<0.001,"Unnormalized runtime skin envelope");draw.push_back(blended);
    }
    return draw;
}
inline std::vector<std::array<float,3>> skinPacket(const ModelInfo& model,const MeshPacket& packet,const std::vector<AffineMatrix>& matrices){
    std::vector<std::array<float,3>> positions;positions.reserve(packet.vertices.size());
    for(auto v:packet.vertices){assetCheck(v.position<model.positions.size()&&v.drawMatrix<matrices.size(),"Invalid skinned vertex reference");positions.push_back(transformPoint(matrices[v.drawMatrix],model.positions[v.position]));}
    return positions;
}
inline std::vector<uint8_t> exportBindObj(const ModelInfo& model,const ModelRig& rig,const ModelMesh& mesh){
    auto matrices=skinDrawMatrices(rig);std::ostringstream out;out<<std::setprecision(9)<<"# Native J3D default-pose geometry; materials/animation pending\n";size_t offset=1;
    for(size_t i=0;i<mesh.packets.size();++i){const auto& packet=mesh.packets[i];auto positions=skinPacket(model,packet,matrices);out<<"g shape_"<<packet.shape<<"_packet_"<<i<<'\n';
        for(auto p:positions)out<<"v "<<p[0]<<' '<<p[1]<<' '<<p[2]<<'\n';
        for(auto t:packet.triangles){for(auto index:t)assetCheck(index<positions.size(),"Invalid triangle vertex");out<<"f "<<offset+t[0]<<' '<<offset+t[1]<<' '<<offset+t[2]<<'\n';}offset+=positions.size();}
    std::string text=out.str();return {text.begin(),text.end()};
}
inline void skinningTests(){
    auto check=[](float a,float b){assetCheck(std::abs(a-b)<0.0001,"Skinning numeric test failed");};
    JointData root;root.scale={1,1,1};root.translation={1,2,3};root.rotation={0,0,16384};
    auto p=transformPoint(jointRotationTranslation(root),{1,0,0});check(p[0],1);check(p[1],3);check(p[2],3);
    ModelRig rig;rig.matrixMode=2;root.rotation={0,0,0};root.translation={0,0,0};root.scale={2,3,4};
    JointData child;child.parent=0;child.scale={1,1,1};child.translation={1,0,0};child.scaleCompensate=true;rig.joints={root,child};
    auto world=jointWorldMatrices(rig);p=transformPoint(world[1],{1,1,1});check(p[0],3);check(p[1],1);check(p[2],1);
    rig.joints[1].scaleCompensate=false;world=jointWorldMatrices(rig);p=transformPoint(world[1],{1,1,1});check(p[0],4);check(p[1],3);check(p[2],4);
    rig.inverseBind={identityMatrix(),identityMatrix()};rig.inverseBindPresent={true,true};rig.envelopes={{{0,0.25f},{1,0.75f}}};rig.drawMatrices={{true,0}};
    auto draw=skinDrawMatrices(rig);p=transformPoint(draw[0],{0,0,0});check(p[0],1.5f);
    bool rejected=false;rig.joints[0].parent=1;try{jointWorldMatrices(rig);}catch(const std::runtime_error&){rejected=true;}assetCheck(rejected,"Joint cycle accepted");
}


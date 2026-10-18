#pragma once
#include "collision_query.hpp"
using ContactVector=std::array<double,3>;
inline ContactVector contactSubtract(const ContactVector& a,const ContactVector& b){return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};}
inline double contactDot(const ContactVector& a,const ContactVector& b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
inline ContactVector closestTrianglePoint(const CollisionTriangle& triangle,const CollisionPoint& center){
    ContactVector p{center[0],center[1],center[2]},a{triangle.points[0][0],triangle.points[0][1],triangle.points[0][2]},b{triangle.points[1][0],triangle.points[1][1],triangle.points[1][2]},c{triangle.points[2][0],triangle.points[2][1],triangle.points[2][2]};
    auto ab=contactSubtract(b,a),ac=contactSubtract(c,a),ap=contactSubtract(p,a);
    double aa=contactDot(ab,ab),bb=contactDot(ab,ac),cc=contactDot(ac,ac),d=contactDot(ap,ab),e=contactDot(ap,ac),det=aa*cc-bb*bb;
    if(det>aa*cc*1e-14){double u=(d*cc-e*bb)/det,v=(e*aa-d*bb)/det;if(u>=0&&v>=0&&u+v<=1){ContactVector out{};for(size_t i=0;i<3;++i)out[i]=a[i]+u*ab[i]+v*ac[i];return out;}}
    ContactVector best=a;double distance=INFINITY;
    for(size_t edge=0;edge<3;++edge){const auto& pa=triangle.points[edge];const auto& pb=triangle.points[(edge+1)%3];ContactVector start{pa[0],pa[1],pa[2]},end{pb[0],pb[1],pb[2]},direction=contactSubtract(end,start);
        double length=contactDot(direction,direction),t=length>0?std::clamp(contactDot(contactSubtract(p,start),direction)/length,0.0,1.0):0;ContactVector point{};
        for(size_t i=0;i<3;++i)point[i]=start[i]+t*direction[i];auto delta=contactSubtract(p,point);double value=contactDot(delta,delta);if(value<distance){distance=value;best=point;}}
    return best;
}
struct SphereContact {CollisionPoint point,normal;double penetration;size_t instance,prism;uint16_t attribute;};
inline std::vector<SphereContact> sphereContacts(const CollisionGeometry& geometry,const CollisionPoint& center,float radius){
    assetCheck(std::isfinite(radius)&&radius>0,"Invalid collision sphere radius");for(float c:center)assetCheck(std::isfinite(c),"Invalid collision sphere center");
    std::vector<SphereContact> contacts;
    for(const auto& triangle:geometry.triangles){auto point=closestTrianglePoint(triangle,center);ContactVector delta{};for(size_t i=0;i<3;++i)delta[i]=double(center[i])-point[i];double distance2=contactDot(delta,delta);
        if(distance2>double(radius)*radius)continue;double distance=std::sqrt(distance2);SphereContact contact{{},{},double(radius)-distance,triangle.instance,triangle.prism,triangle.attribute};
        double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));assetCheck(length>0,"Zero sphere contact face normal");
        for(size_t i=0;i<3;++i){contact.point[i]=float(point[i]);contact.normal[i]=float(distance>1e-12?delta[i]/distance:triangle.normal[i]/length);}contacts.push_back(contact);
    }return contacts;
}
inline void sphereContactTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{0,0,0},CollisionPoint{2,0,0},CollisionPoint{0,2,0}},{0,0,1},7,4,3}};
    auto face=sphereContacts(geometry,{.5f,.5f,.5f},1);assetCheck(face.size()==1&&std::abs(face[0].penetration-.5)<1e-8&&face[0].normal[2]==1&&face[0].instance==3&&face[0].prism==4&&face[0].attribute==7,"Sphere face/metadata test");
    auto edge=sphereContacts(geometry,{1,-.5f,0},1);assetCheck(edge.size()==1&&edge[0].point[0]==1&&edge[0].point[1]==0&&edge[0].normal[1]==-1,"Sphere edge test");
    auto vertex=sphereContacts(geometry,{-1,-1,0},2);assetCheck(vertex.size()==1&&vertex[0].point==CollisionPoint{0,0,0}&&std::abs(vertex[0].penetration-(2-std::sqrt(2.0)))<1e-8,"Sphere vertex test");
    assetCheck(sphereContacts(geometry,{.5f,.5f,1},1).size()==1&&sphereContacts(geometry,{.5f,.5f,2},1).empty(),"Sphere tangency/miss test");
    auto back=sphereContacts(geometry,{.5f,.5f,-.5f},1);assetCheck(back.size()==1&&back[0].normal[2]==-1,"Sphere reverse side test");
    bool rejected=false;try{sphereContacts(geometry,{0,0,0},-1);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid sphere radius accepted");
}
inline size_t verifySphereContacts(const CollisionGeometry& geometry){
    size_t tested=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t index=0;index<geometry.triangles.size();index+=stride){const auto& triangle=geometry.triangles[index];CollisionPoint center{};double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));assetCheck(length>0,"Invalid sphere test normal");
        for(size_t i=0;i<3;++i)center[i]=float((double(triangle.points[0][i])+triangle.points[1][i]+triangle.points[2][i])/3+5*triangle.normal[i]/length);
        CollisionGeometry isolated;isolated.triangles={triangle};auto contacts=sphereContacts(isolated,center,10);assetCheck(contacts.size()==1&&std::abs(contacts[0].penetration-5)<.02,"Retail sphere centroid contact failed");
        auto scene=sphereContacts(geometry,center,10);bool found=false;for(const auto& contact:scene)if(contact.instance==triangle.instance&&contact.prism==triangle.prism){found=true;break;}assetCheck(found,"Placed sphere contact missing in complete mesh");++tested;
    }return tested;
}

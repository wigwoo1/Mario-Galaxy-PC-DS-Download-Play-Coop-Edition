#pragma once
#include "sphere_contact.hpp"
#include <numeric>
struct CollisionBox {std::array<double,3> low{INFINITY,INFINITY,INFINITY},high{-INFINITY,-INFINITY,-INFINITY};};
inline bool collisionBoxesOverlap(const CollisionBox& a,const CollisionBox& b){for(size_t i=0;i<3;++i)if(a.high[i]<b.low[i]||b.high[i]<a.low[i])return false;return true;}
class CollisionIndex {
    struct Node {CollisionBox box;size_t begin,count,left=0,right=0;};
    CollisionGeometry geometry;std::vector<size_t> order;std::vector<CollisionBox> boxes;std::vector<Node> nodes;
    size_t build(size_t begin,size_t count){
        CollisionBox box;for(size_t i=begin;i<begin+count;++i)for(size_t axis=0;axis<3;++axis){box.low[axis]=std::min(box.low[axis],boxes[order[i]].low[axis]);box.high[axis]=std::max(box.high[axis],boxes[order[i]].high[axis]);}
        size_t node=nodes.size();nodes.push_back({box,begin,count});if(count<=8)return node;
        size_t axis=0;for(size_t k=1;k<3;++k)if(box.high[k]-box.low[k]>box.high[axis]-box.low[axis])axis=k;
        size_t middle=begin+count/2;std::nth_element(order.begin()+begin,order.begin()+middle,order.begin()+begin+count,[&](size_t a,size_t b){return boxes[a].low[axis]+boxes[a].high[axis]<boxes[b].low[axis]+boxes[b].high[axis];});
        size_t left=build(begin,count/2),right=build(middle,count-count/2);nodes[node].left=left;nodes[node].right=right;return node;
    }
public:
    explicit CollisionIndex(const CollisionGeometry& source):geometry(source){
        order.resize(geometry.triangles.size());std::iota(order.begin(),order.end(),0);boxes.resize(order.size());
        for(size_t i=0;i<order.size();++i){auto& box=boxes[i];for(auto p:geometry.triangles[i].points)for(size_t k=0;k<3;++k){assetCheck(std::isfinite(p[k]),"Invalid indexed collision point");box.low[k]=std::min(box.low[k],double(p[k]));box.high[k]=std::max(box.high[k],double(p[k]));}
            double pad=1e-4;for(size_t k=0;k<3;++k)pad=std::max(pad,(box.high[k]-box.low[k])*1e-6);for(size_t k=0;k<3;++k){box.low[k]-=pad;box.high[k]+=pad;}}
        if(!order.empty())build(0,order.size());
    }
    CollisionGeometry candidates(const CollisionBox& query)const{
        for(size_t k=0;k<3;++k)assetCheck(std::isfinite(query.low[k])&&std::isfinite(query.high[k])&&query.low[k]<=query.high[k],"Invalid collision query bounds");
        std::vector<size_t> found,stack;if(!nodes.empty())stack.push_back(0);
        while(!stack.empty()){size_t i=stack.back();stack.pop_back();const auto& node=nodes[i];if(!collisionBoxesOverlap(query,node.box))continue;
            if(node.left==0){for(size_t j=node.begin;j<node.begin+node.count;++j)if(collisionBoxesOverlap(query,boxes[order[j]]))found.push_back(order[j]);}
            else{stack.push_back(node.left);stack.push_back(node.right);}}
        // Preserve source order for equal-distance hits and contact ordering.
        std::sort(found.begin(),found.end());CollisionGeometry result;for(size_t i:found)result.triangles.push_back(geometry.triangles[i]);return result;
    }
    std::optional<CollisionHit> trace(const CollisionPoint& start,const CollisionPoint& end,size_t* checked=nullptr)const{
        CollisionBox query;for(size_t k=0;k<3;++k){assetCheck(std::isfinite(start[k])&&std::isfinite(end[k]),"Invalid indexed segment");query.low[k]=std::min(start[k],end[k]);query.high[k]=std::max(start[k],end[k]);}
        auto subset=candidates(query);if(checked)*checked=subset.triangles.size();return traceCollision(subset,start,end);
    }
    std::vector<SphereContact> sphere(const CollisionPoint& center,float radius,size_t* checked=nullptr)const{
        assetCheck(std::isfinite(radius)&&radius>0,"Invalid indexed sphere radius");CollisionBox query;for(size_t k=0;k<3;++k){query.low[k]=double(center[k])-radius;query.high[k]=double(center[k])+radius;}
        auto subset=candidates(query);if(checked)*checked=subset.triangles.size();return sphereContacts(subset,center,radius);
    }
};
inline size_t verifyCollisionIndex(const CollisionGeometry& geometry){
    CollisionIndex index(geometry);size_t checked=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t i=0;i<geometry.triangles.size();i+=stride){const auto& triangle=geometry.triangles[i];CollisionPoint center{},start{},end{};double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));
        for(size_t k=0;k<3;++k){center[k]=float((double(triangle.points[0][k])+triangle.points[1][k]+triangle.points[2][k])/3);start[k]=float(center[k]+10*triangle.normal[k]/length);end[k]=float(center[k]-10*triangle.normal[k]/length);}
        size_t count=0;auto a=traceCollision(geometry,start,end),b=index.trace(start,end,&count);checked+=count;
        assetCheck(bool(a)==bool(b)&&(!a||(a->fraction==b->fraction&&a->instance==b->instance&&a->prism==b->prism&&a->attribute==b->attribute)),"Indexed segment differs from reference");
        auto contacts=sphereContacts(geometry,start,20),filtered=index.sphere(start,20,&count);checked+=count;assetCheck(contacts.size()==filtered.size(),"Indexed sphere contact count differs");
        for(size_t j=0;j<contacts.size();++j)assetCheck(contacts[j].instance==filtered[j].instance&&contacts[j].prism==filtered[j].prism&&contacts[j].point==filtered[j].point&&contacts[j].normal==filtered[j].normal&&contacts[j].penetration==filtered[j].penetration,"Indexed sphere contact differs");
    }return checked;
}

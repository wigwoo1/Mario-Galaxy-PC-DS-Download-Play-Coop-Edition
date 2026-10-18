#pragma once
#include "layout_pose.hpp"
struct NativeLayoutWorldPane{std::array<float,6> matrix{};std::array<std::array<float,2>,4> corners{};float alpha=255,descendantAlpha=1;bool visible=true,influenced=false;};
// Authored layout coordinates only; viewport/aspect adjustment and 3D panes are unsupported.
inline std::vector<NativeLayoutWorldPane> layoutWorldPanes(const NativeLayoutResource& layout){
    std::vector<NativeLayoutWorldPane> result;result.reserve(layout.panes.size());
    for(size_t i=0;i<layout.panes.size();++i){const auto& pane=layout.panes[i];assetCheck(pane.parent>=-1&&(pane.parent<0||size_t(pane.parent)<i),"Invalid layout parent order");assetCheck(pane.rotate[0]==0&&pane.rotate[1]==0,"Unsupported 3D layout rotation");float angle=pane.rotate[2]*0.017453292519943295f,c=std::cos(angle),s=std::sin(angle);NativeLayoutWorldPane world;
        world.matrix={c*pane.scale[0],-s*pane.scale[1],pane.translate[0],s*pane.scale[0],c*pane.scale[1],pane.translate[1]};world.alpha=pane.alpha;world.visible=(pane.flags&1)!=0;
        if(pane.parent>=0){const auto& parent=result[size_t(pane.parent)];const auto& a=parent.matrix;const auto b=world.matrix;world.matrix={a[0]*b[0]+a[1]*b[3],a[0]*b[1]+a[1]*b[4],a[0]*b[2]+a[1]*b[5]+a[2],a[3]*b[0]+a[4]*b[3],a[3]*b[1]+a[4]*b[4],a[3]*b[2]+a[4]*b[5]+a[5]};world.visible=world.visible&&parent.visible;world.descendantAlpha=parent.descendantAlpha;world.influenced=parent.influenced;if(parent.influenced)world.alpha=std::floor(pane.alpha*parent.descendantAlpha);}
        if((pane.flags&2)&&pane.alpha!=255){world.descendantAlpha*=pane.alpha/255.f;world.influenced=true;}
        float left=-float(pane.base%3)*pane.size[0]*0.5f,top=float(pane.base/3)*pane.size[1]*0.5f;std::array<std::array<float,2>,4> local{{{left,top},{left+pane.size[0],top},{left,top-pane.size[1]},{left+pane.size[0],top-pane.size[1]}}};
        for(unsigned k=0;k<4;++k){const auto& m=world.matrix;world.corners[k]={m[0]*local[k][0]+m[1]*local[k][1]+m[2],m[3]*local[k][0]+m[4]*local[k][1]+m[5]};for(float coordinate:world.corners[k])assetCheck(std::isfinite(coordinate),"Layout world coordinate overflow");}result.push_back(world);
    }return result;
}
inline void layoutGeometryTests(){
    NativeLayoutResource layout;NativeLayoutPane parent;parent.flags=3;parent.alpha=128;parent.scale={2,3};parent.rotate={0,0,90};parent.translate={10,20,0};layout.panes.push_back(parent);NativeLayoutPane child;child.parent=0;child.flags=1;child.alpha=64;child.scale={1,1};child.translate={2,4,0};child.size={10,8};child.base=4;layout.panes.push_back(child);auto world=layoutWorldPanes(layout);auto close=[](float a,float b){return std::abs(a-b)<0.0001f;};assetCheck(close(world[1].corners[0][0],-14)&&close(world[1].corners[0][1],14)&&close(world[1].corners[3][0],10)&&close(world[1].corners[3][1],34),"Hierarchical layout transform/anchor mismatch");assetCheck(close(world[1].alpha,32)&&world[1].visible,"Layout inherited alpha mismatch");NativeLayoutPane grandchild;grandchild.parent=1;grandchild.flags=1;grandchild.scale={1,1};layout.panes.push_back(grandchild);assetCheck(layoutWorldPanes(layout)[2].alpha==128,"Unflagged pane incorrectly contributed alpha to descendants");layout.panes[0].flags=0;assetCheck(!layoutWorldPanes(layout)[1].visible,"Hidden layout parent did not hide child");
}


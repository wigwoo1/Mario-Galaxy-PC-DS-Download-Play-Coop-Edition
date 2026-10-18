#pragma once
#include "layout_resource.hpp"
#include "layout_animation.hpp"
// Native pane-target application; material/texture animation is explicitly unsupported.
inline NativeLayoutResource sampleLayoutPose(const NativeLayoutResource& source,const NativeLayoutAnimation& animation,float frame){
    auto result=source;
    for(const auto& track:animation.tracks){assetCheck(track.contentType==0&&track.id==0,"Unsupported layout animation material/target ID");NativeLayoutPane* pane=nullptr;for(auto& candidate:result.panes)if(candidate.name==track.name){assetCheck(!pane,"Ambiguous animated layout pane");pane=&candidate;}assetCheck(pane,"Missing animated layout pane");auto value=sampleLayoutTrack(track,frame);
        if(track.kind=="RLPA"){
            if(track.target<3)pane->translate[track.target]=value;
            else if(track.target<6)pane->rotate[track.target-3]=value;
            else if(track.target<8)pane->scale[track.target-6]=value;
            else if(track.target<10){assetCheck(value>=0,"Negative animated layout dimensions");pane->size[track.target-8]=value;}
            else throw std::runtime_error("Unsupported layout transform target");
        }else if(track.kind=="RLVC"){
            auto channel=uint8_t(std::clamp(value,0.0f,255.0f));if(track.target==16)pane->alpha=channel;
            else{assetCheck(track.target<16&&(pane->kind=="pic1"||pane->kind=="wnd1"),"Unsupported layout vertex color target");unsigned corner=track.target/4,shift=(3-track.target%4)*8;pane->colors[corner]=(pane->colors[corner]&~(255u<<shift))|(uint32_t(channel)<<shift);}
        }else if(track.kind=="RLVI"){assetCheck(track.target==0&&track.curve==1,"Unsupported layout visibility target");pane->flags=uint8_t((pane->flags&~1u)|(value!=0?1u:0u));}
        else throw std::runtime_error("Unsupported layout animation kind");
    }return result;
}
inline void layoutPoseTests(){
    NativeLayoutResource source;NativeLayoutPane pane;pane.name="test";pane.kind="pic1";pane.alpha=255;source.panes.push_back(pane);
    NativeLayoutAnimation animation{};animation.tracks={{"test","RLPA",0,0,8,2,{{0,20,0},{10,10,0}}},{"test","RLVC",0,0,16,2,{{0,0,0},{10,255,0}}}};
    auto pose=sampleLayoutPose(source,animation,5);assetCheck(pose.panes[0].size[0]==15&&pose.panes[0].alpha==127&&source.panes[0].alpha==255,"Layout pose application/immutable source mismatch");
    animation.tracks[0].name="missing";bool rejected=false;try{sampleLayoutPose(source,animation,0);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Missing animated pane accepted");
}

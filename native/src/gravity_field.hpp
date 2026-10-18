#pragma once
#include "disk_gravity.hpp"
#include "box_gravity.hpp"
#include "segment_gravity.hpp"
#include <variant>
struct GravitySource {std::variant<NativePointGravity,NativeDiskGravity,NativeBoxGravity,NativeSegmentGravity> shape;int32_t priority=0;bool active=true;std::string name;};
struct GravitySelection {CollisionPoint direction{};int32_t priority=-1;size_t contributing=0;};
inline std::optional<GravitySelection> selectGravity(const std::vector<GravitySource>& sources,const CollisionPoint& position){
    std::optional<GravitySelection> result;std::array<double,3> total{};
    for(const auto& source:sources){if(!source.active)continue;
        auto direction=std::visit([&](const auto& shape){return shape.direction(position);},source.shape);if(!direction)continue;
        if(result&&source.priority<result->priority)continue;
        double distance=0,distant=0;
        if(auto point=std::get_if<NativePointGravity>(&source.shape)){double squared=0;for(size_t k=0;k<3;++k){double value=double(position[k])-point->center[k];squared+=value*value;}distance=std::sqrt(squared);distant=point->distant;}
        else if(std::holds_alternative<NativeDiskGravity>(source.shape)){const auto& disk=std::get<NativeDiskGravity>(source.shape);CollisionPoint relative{};for(size_t k=0;k<3;++k)relative[k]=position[k]-disk.parameters.center[k];double height=collisionDot(relative,disk.normal),radialSquared=0;
            for(size_t k=0;k<3;++k){double value=relative[k]-height*disk.normal[k];radialSquared+=value*value;}double edge=std::max(0.0,std::sqrt(radialSquared)-disk.radius);distance=std::sqrt(height*height+edge*edge);distant=disk.parameters.distant;}
        else if(auto box=std::get_if<NativeBoxGravity>(&source.shape)){distance=*box->distance(position);distant=box->parameters.distant;}
        else{const auto& segment=std::get<NativeSegmentGravity>(source.shape);distance=*segment.distance(position);distant=segment.parameters.distant;}
        double adjusted=std::max(1.0,distance-distant),weight=4000000/(adjusted*adjusted);
        if(!result||source.priority>result->priority){result=GravitySelection{{},source.priority,0};total={0,0,0};}
        ++result->contributing;for(size_t k=0;k<3;++k)total[k]+=(*direction)[k]*weight;
    }
    if(result){double length=std::sqrt(total[0]*total[0]+total[1]*total[1]+total[2]*total[2]);for(size_t k=0;k<3;++k)result->direction[k]=length>.001?float(total[k]/length):0;}return result;
}
struct ZoneGravityField {std::vector<GravitySource> sources;size_t unsupported=0;};
inline ZoneGravityField loadCommonGravity(WiiAssets& assets,const std::string& stagePath){
    Rarc archive(assets.readFile(stagePath));ZoneGravityField result;
    for(const auto& resource:archive.files)if(resource.path=="/jmp/placement/common/planetobjinfo"){auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();
        for(const auto& placement:decodePlacements(resource.path,table)){GravitySource source;
            if(placement.name=="GlobalPointGravity")source.shape=decodePointGravity(placement,table);else if(placement.name=="GlobalDiskGravity")source.shape=decodeDiskGravity(placement,table);else if(placement.name=="GlobalPlaneGravityInBox")source.shape=decodeBoxGravity(placement,table);else if(placement.name=="GlobalSegmentGravity")source.shape=decodeSegmentGravity(placement,table);else{++result.unsupported;continue;}
            source.name=placement.name;int field=table.find("Priority");if(field>=0){auto value=table.read(placement.row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Gravity priority type mismatch");int32_t priority=int32_t(std::get<uint32_t>(value));if(priority>=0)source.priority=priority;}result.sources.push_back(source);
        }
    }return result;
}
inline void gravitySelectionTests(){
    NativeSegmentGravity segment;segment.end={0,10,0};segment.length=10;std::vector<GravitySource> segmentSources{{segment,3,true,"segment"}};auto segmentSelected=selectGravity(segmentSources,{2,5,0});assetCheck(segmentSelected&&segmentSelected->priority==3&&segmentSelected->direction==CollisionPoint{-1,0,0},"Segment manager selection failed");
    NativePointGravity a,b;a.center={1,0,0};b.center={0,2,0};std::vector<GravitySource> sources{{a,0,true,"a"},{b,0,true,"b"}};
    auto blend=selectGravity(sources,{0,0,0});assetCheck(blend&&blend->contributing==2&&std::abs(blend->direction[0]-4/std::sqrt(17.0))<.0001&&std::abs(blend->direction[1]-1/std::sqrt(17.0))<.0001,"Gravity distance-weighted blend test");
    sources[1].priority=1;auto higher=selectGravity(sources,{0,0,0});assetCheck(higher&&higher->direction==CollisionPoint{0,1,0}&&higher->contributing==1,"Gravity priority override test");
    std::get<NativePointGravity>(sources[1].shape).range=1;auto fallback=selectGravity(sources,{0,0,0});assetCheck(fallback&&fallback->direction==CollisionPoint{1,0,0},"Out-of-range priority fallback test");
    sources[0].active=false;assetCheck(!selectGravity(sources,{0,0,0}),"Inactive gravity selection test");
}

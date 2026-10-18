#pragma once
#include "gravity_field.hpp"
#include "diagnostic_body.hpp"
inline CollisionPoint gravityBodyVector(const ZoneGravityField& field,const CollisionPoint& position){
    auto selected=selectGravity(field.sources,position);assetCheck(selected.has_value(),"Diagnostic body left supported gravity coverage");
    assetCheck(collisionDot(selected->direction,selected->direction)>.5f,"Diagnostic body encountered zero gravity direction");auto gravity=selected->direction;for(float& value:gravity)value*=100;return gravity;
}
inline CollisionPoint loadCommonPlayerStart(WiiAssets& assets,const std::string& stagePath){
    Rarc stage(assets.readFile(stagePath));auto bytes=stage.readFile("/jmp/start/common/startinfo");StageTable table{BinaryView(bytes)};
    auto placements=decodePlacements("/jmp/start/common/startinfo",table);for(const auto& placement:placements)if(placement.name=="Mario")return placement.position;
    throw std::runtime_error("No common Mario start in diagnostic zone");
}

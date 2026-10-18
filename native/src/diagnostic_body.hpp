#pragma once
#include "collision_slide.hpp"
#include "native_runtime_client.hpp"
// Diagnostic sphere physics, independent of the game's Mario actor and tuning.
struct DiagnosticBody {
    CollisionPoint position{},velocity{};float radius=10;bool grounded=false;size_t partialSteps=0;
    void tick(const CollisionIndex& index,const CollisionPoint& gravity,const CollisionPoint& input,bool jump=false,NativeRuntimeClient* runtime=nullptr){
        assetCheck(std::isfinite(radius)&&radius>0,"Invalid diagnostic body radius");for(float value:position)assetCheck(std::isfinite(value),"Invalid diagnostic body position");
        constexpr float dt=1.0f/60;float gravityLength=std::sqrt(collisionDot(gravity,gravity));assetCheck(std::isfinite(gravityLength)&&gravityLength>0,"Invalid body gravity");
        CollisionPoint up{},move=input;for(size_t k=0;k<3;++k){assetCheck(std::isfinite(input[k])&&std::isfinite(velocity[k]),"Invalid diagnostic body input/velocity");up[k]=-gravity[k]/gravityLength;}
        float vertical=collisionDot(move,up);for(size_t k=0;k<3;++k)move[k]-=vertical*up[k];float length=std::sqrt(collisionDot(move,move));assetCheck(std::isfinite(length),"Diagnostic movement magnitude overflow");if(length>1)for(float& v:move)v/=length;
        float speedUp=collisionDot(velocity,up);assetCheck(std::isfinite(speedUp),"Diagnostic vertical velocity overflow");for(size_t k=0;k<3;++k)velocity[k]=up[k]*speedUp+move[k]*40;
        if(jump&&grounded){for(size_t k=0;k<3;++k)velocity[k]+=up[k]*(60-speedUp);grounded=false;}
        CollisionPoint displacement{};for(size_t k=0;k<3;++k){velocity[k]+=gravity[k]*dt;displacement[k]=velocity[k]*dt;}
        if(runtime){
            runtime->tick();auto result=runtime->bind(position,gravity,displacement,radius);for(size_t k=0;k<3;++k)position[k]+=result.displacement[k];
            float verticalSpeed=collisionDot(velocity,up);grounded=(result.contacts&1u)&&verticalSpeed<=0;
            if((grounded&&verticalSpeed<0)||((result.contacts&4u)&&verticalSpeed>0))for(size_t k=0;k<3;++k)velocity[k]-=up[k]*verticalSpeed;
            return;
        }
        auto movement=slideSphere(index,position,displacement,radius);position=movement.position;if(!movement.complete){++partialSteps;velocity={0,0,0};}
        for(auto normal:movement.normals){float into=collisionDot(velocity,normal);if(into<0)for(size_t k=0;k<3;++k)velocity[k]-=into*normal[k];}
        CollisionPoint below{};for(size_t k=0;k<3;++k)below[k]=position[k]-up[k]*.05f;auto probe=sweepSphere(index,position,below,radius);
        grounded=collisionDot(velocity,up)<=0&&probe.complete&&probe.blocked&&collisionDot(probe.normal,up)>.6f;

    }
};
inline void diagnosticBodyTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{-100,-100,0},CollisionPoint{100,-100,0},CollisionPoint{0,100,0}},{0,0,1},0,1}};CollisionIndex index(geometry);
    DiagnosticBody body;body.position={0,0,3};body.radius=1;
    for(int i=0;i<120;++i)body.tick(index,{0,0,-10},{0,0,0});assetCheck(body.grounded&&body.position[2]>=1&&body.position[2]<1.03&&std::abs(body.velocity[2])<.2&&body.partialSteps==0,"Diagnostic body falling/grounding test");
    auto full=body,half=body;for(int i=0;i<10;++i){full.tick(index,{0,0,-10},{1,0,0});half.tick(index,{0,0,-10},{.5f,0,0});}assetCheck(std::abs((full.position[0]-body.position[0])-2*(half.position[0]-body.position[0]))<.001f,"Diagnostic analog movement speed mismatch");
    float before=body.position[0];for(int i=0;i<30;++i)body.tick(index,{0,0,-10},{1,0,0});assetCheck(body.position[0]>before+19&&body.grounded,"Diagnostic body ground movement test");
    body.tick(index,{0,0,-10},{0,0,0},true);assetCheck(!body.grounded&&body.velocity[2]>50,"Diagnostic body jump test");
    auto airborne=body;auto firstJumpSpeed=body.velocity[2];body.tick(index,{0,0,-10},{0,0,0},true);airborne.tick(index,{0,0,-10},{0,0,0},false);assetCheck(body.velocity==airborne.velocity&&body.position==airborne.position&&body.velocity[2]<firstJumpSpeed,"Airborne jump retriggered impulse");
    bool descended=false,landed=false;for(int i=0;i<800;++i){body.tick(index,{0,0,-10},{0,0,0});if(body.velocity[2]<0)descended=true;if(body.grounded){landed=true;break;}}assetCheck(descended&&landed&&body.partialSteps==0&&body.position[2]>=1&&body.position[2]<1.03,"Diagnostic jump did not complete landing");
    body.tick(index,{0,0,-10},{0,0,0},true);assetCheck(!body.grounded&&body.velocity[2]>50,"Landing did not re-enable jump");
    for(size_t axis=0;axis<3;++axis)for(float sign:{-1.f,1.f}){CollisionGeometry rotated;auto rotate=[&](CollisionPoint point){CollisionPoint result{};result[axis]=point[2]*sign;result[(axis+1)%3]=point[0];result[(axis+2)%3]=point[1];return result;};for(auto source:geometry.triangles){for(auto& point:source.points)point=rotate(point);source.normal=rotate(source.normal);rotated.triangles.push_back(source);}CollisionIndex rotatedIndex(rotated);DiagnosticBody traveller;traveller.radius=1;traveller.position=rotate({0,0,3});auto down=rotate({0,0,-10}),up=rotate({0,0,1});for(int i=0;i<120;++i)traveller.tick(rotatedIndex,down,{0,0,0});assetCheck(traveller.grounded,"Rotated gravity failed to establish support");traveller.tick(rotatedIndex,down,{0,0,0},true);assetCheck(!traveller.grounded&&collisionDot(traveller.velocity,up)>50,"Rotated gravity jump direction mismatch");bool landed=false;for(int i=0;i<800;++i){traveller.tick(rotatedIndex,down,{0,0,0});if(traveller.grounded){landed=true;break;}}assetCheck(landed&&traveller.partialSteps==0,"Rotated gravity jump failed to land");}
    {auto safe=body;auto previousPosition=safe.position,previousVelocity=safe.velocity;bool rejected=false;try{safe.tick(index,{0,0,-10},{std::numeric_limits<float>::max(),0,0});}catch(const std::exception&){rejected=true;}assetCheck(rejected&&safe.position==previousPosition&&safe.velocity==previousVelocity,"Overflowing movement modified body state");}
    DiagnosticBody rising;rising.radius=1;rising.position={0,0,1.005f};rising.velocity={0,0,.2f};rising.tick(index,{0,0,-10},{0,0,0});assetCheck(rising.velocity[2]>0&&!rising.grounded,"Near-floor rising body acquired ground support");
}
inline size_t verifyDiagnosticBody(const CollisionGeometry& geometry){
    CollisionIndex index(geometry);size_t count=0,stride=std::max<size_t>(1,geometry.triangles.size()/8);
    for(size_t i=0;i<geometry.triangles.size();i+=stride){const auto& triangle=geometry.triangles[i];double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));DiagnosticBody body;CollisionPoint gravity{};
        for(size_t k=0;k<3;++k){body.position[k]=float((double(triangle.points[0][k])+triangle.points[1][k]+triangle.points[2][k])/3+30*triangle.normal[k]/length);gravity[k]=float(-100*triangle.normal[k]/length);}
        for(int tick=0;tick<120;++tick)body.tick(index,gravity,{0,0,0});double remaining=0;for(auto contact:sphereContacts(geometry,body.position,body.radius))remaining=std::max(remaining,contact.penetration);
        assetCheck(body.partialSteps==0&&body.grounded&&remaining<=.001,"Retail diagnostic falling body check failed");++count;
    }return count;
}







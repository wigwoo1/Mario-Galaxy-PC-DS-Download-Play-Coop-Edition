#pragma once
#include <Native/GxCommandState.hpp>
namespace Native {
// CPU vertex lighting; inputs are view-space position and transformed normal.
// Singular attenuation denominators reject rather than fabricate a light value.
inline std::array<unsigned,4> evaluateDiffuseLighting(const GxCommandState& state,unsigned channel,
 const std::array<float,3>& position,const std::array<float,3>& normal,
 const std::array<unsigned,4>& vertexColor){
 for(float v:position)if(!std::isfinite(v))throw std::invalid_argument("Nonfinite lighting position");
 for(float v:normal)if(!std::isfinite(v))throw std::invalid_argument("Nonfinite lighting normal");
 for(unsigned v:vertexColor)if(v>255)throw std::invalid_argument("Invalid vertex color");
 std::array<unsigned,4> result{};
 for(unsigned component=0;component<4;++component){
  auto control=state.lightingChannel(channel,component==3);
  unsigned material=control.materialVertex?vertexColor[component]:state.lightingColor(channel)[component];
  int sum=255;
  if(control.enabled){
   sum=int(control.ambientVertex?vertexColor[component]:state.lightingColor(channel,true)[component]);

   for(unsigned slot=0;slot<8;++slot)if(control.lightMask&(1u<<slot)){
    auto light=state.light(slot);float diffuse=1,attenuation=1;
    std::array<float,3> direction{};float squared=0;
    for(unsigned axis=0;axis<3;++axis){direction[axis]=light.position[axis]-position[axis];squared+=direction[axis]*direction[axis];}
    if(!std::isfinite(squared))throw std::overflow_error("Lighting distance overflow");
    float length=std::sqrt(squared);
    for(unsigned axis=0;axis<3;++axis)direction[axis]=length?direction[axis]/length:normal[axis];
    float facing=0;for(unsigned axis=0;axis<3;++axis)facing+=direction[axis]*normal[axis];
    if(control.diffuse)diffuse=control.diffuse==2?std::max(0.f,facing):facing;
    if(control.attenuation){
     float angle=0;for(unsigned axis=0;axis<3;++axis)angle+=(control.spot?direction[axis]:normal[axis])*light.direction[axis];
     angle=std::max(0.f,angle);if(!control.spot&&facing<0)angle=0;
     auto polynomial=[](const std::array<float,3>& coefficients,float x,float xx){return coefficients[0]+coefficients[1]*x+coefficients[2]*xx;};
     auto distanceCoefficients=light.distance;
     if(!control.spot&&control.diffuse){
      float magnitude=0;for(float v:distanceCoefficients)magnitude+=v*v;magnitude=std::sqrt(magnitude);
      if(!std::isfinite(magnitude)||magnitude==0)throw std::invalid_argument("Invalid specular attenuation coefficients");
      for(float& v:distanceCoefficients)v/=magnitude;
     }
     float divisor=control.spot?polynomial(distanceCoefficients,length,squared):polynomial(distanceCoefficients,angle,angle*angle);
     if(!std::isfinite(divisor)||divisor==0)throw std::invalid_argument("Singular GX attenuation");
     attenuation=std::max(0.f,polynomial(light.angular,angle,angle*angle))/divisor;
    }
    float contribution=attenuation*diffuse*float(light.color[component]);
    if(!std::isfinite(contribution)||std::fabs(contribution)>1000000)throw std::overflow_error("Lighting contribution overflow");
    sum+=int(std::nearbyint(contribution));
   }
  }
  unsigned illumination=unsigned(std::clamp(sum,0,255));
  result[component]=(material*(illumination+(illumination>>7)))>>8;
 }
 return result;
}
}


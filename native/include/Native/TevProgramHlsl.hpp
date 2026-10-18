#pragma once
#include <Native/GxCommandState.hpp>
#include <Native/TevArithmeticHlsl.hpp>
#include <sstream>
namespace Native {
// Specializes only the combiner from an immutable draw-state snapshot.
// Caller supplies already sampled textures and lit raster colors per stage.
inline std::string tevProgramHlsl(const GxCommandState& state){
 std::ostringstream s;s<<tevArithmeticHlsl;
 auto rgba=[](const std::array<int,4>& v){return "int4("+std::to_string(v[0])+","+std::to_string(v[1])+","+std::to_string(v[2])+","+std::to_string(v[3])+")";};
 s<<"int4 evaluateTevProgram(int4 textures[16],int4 rasters[16]){int4 r[4];\n";
 for(unsigned i=0;i<4;++i)s<<"r["<<i<<"]="<<rgba(state.tevColor(i))<<";\n";
 for(unsigned i=0;i<state.tevStageCount();++i){
  auto stage=state.tevStage(i);
  s<<"{int4 p[4];for(uint j=0;j<4;++j)p[j]=r[j];int4 t=";
  if(state.tevOrder(i).enabled)s<<"textures["<<i<<"]";else s<<"int4(255,255,255,255)";
  s<<";int4 v=rasters["<<i<<"];int4 k="<<rgba(state.tevKonstValue(i))<<";\n";
  auto swap=[&](const char* name,unsigned table){
   auto order=state.tevSwapTable(table);s<<name<<"="<<name<<".";
   for(auto c:order)s<<"xyzw"[c];s<<";\n";
  };
  swap("t",stage.textureSwap);swap("v",stage.rasterSwap);
  auto color=[&](unsigned sel){
   if(sel<8)return std::string("p[")+std::to_string(sel/2)+"]"+(sel&1?".www":".xyz");
   if(sel==8)return std::string("t.xyz");if(sel==9)return std::string("t.www");
   if(sel==10)return std::string("v.xyz");if(sel==11)return std::string("v.www");
   if(sel==12)return std::string("int3(255,255,255)");if(sel==13)return std::string("int3(128,128,128)");
   if(sel==14)return std::string("k.xyz");return std::string("int3(0,0,0)");
  };
  auto alpha=[&](unsigned sel){
   if(sel<4)return std::string("p[")+std::to_string(sel)+"].w";
   if(sel==4)return std::string("t.w");if(sel==5)return std::string("v.w");
   if(sel==6)return std::string("k.w");return std::string("0");
  };
  for(unsigned j=0;j<4;++j){
   s<<"int3 c"<<j<<"="<<color(stage.color.inputs[j])<<(j<3?"&255":"")<<";";
   s<<"int a"<<j<<"="<<alpha(stage.alpha.inputs[j])<<(j<3?"&255":"")<<";\n";
  }
  s<<"int3 cr;";
  if(stage.color.bias==3)s<<"cr=tevColorCompare(c0,c1,c2,c3,"<<stage.color.scale*2+stage.color.subtract<<","<<(stage.color.clamp?"true":"false")<<");\n";
  else for(unsigned c=0;c<3;++c)s<<"cr["<<c<<"]=tevArithmetic(c0["<<c<<"],c1["<<c<<"],c2["<<c<<"],c3["<<c<<"],"<<stage.color.bias<<","<<(stage.color.subtract?"true":"false")<<","<<stage.color.scale<<","<<(stage.color.clamp?"true":"false")<<");\n";
  s<<"int ar=";
  if(stage.alpha.bias==3)s<<"tevAlphaCompare(c0,c1,a0,a1,a2,a3,"<<stage.alpha.scale*2+stage.alpha.subtract<<","<<(stage.alpha.clamp?"true":"false")<<");\n";
  else s<<"tevArithmetic(a0,a1,a2,a3,"<<stage.alpha.bias<<","<<(stage.alpha.subtract?"true":"false")<<","<<stage.alpha.scale<<","<<(stage.alpha.clamp?"true":"false")<<");\n";
  s<<"r["<<stage.color.destination<<"].xyz=cr;r["<<stage.alpha.destination<<"].w=ar;}\n";
 }
 auto last=state.tevStage(state.tevStageCount()-1);
 s<<"return int4(r["<<last.color.destination<<"].xyz,r["<<last.alpha.destination<<"].w)&255;}\n";
 return s.str();
}
}

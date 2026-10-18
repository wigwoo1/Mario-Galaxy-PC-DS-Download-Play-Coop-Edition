#pragma once
#include <Native/GxCommandState.hpp>
#include <Native/TevArithmetic.hpp>
namespace Native {
using TevRgba=std::array<int,4>;
using TevRegisters=std::array<TevRgba,4>;
// Evaluates combiner/register behavior. Sampling, lighting, indirect UVs and
// alpha/depth tests are supplied by the future complete rendering pipeline.
inline void evaluateTevStage(const GxCommandState& state,unsigned index,
                             TevRegisters& registers,TevRgba texture,TevRgba raster){
 auto stage=state.tevStage(index);auto constant=state.tevKonstValue(index);
 auto swap=[&](TevRgba value,unsigned table){auto order=state.tevSwapTable(table);TevRgba result{};for(unsigned c=0;c<4;++c)result[c]=value[order[c]];return result;};
 texture=swap(texture,stage.textureSwap);raster=swap(raster,stage.rasterSwap);
 const auto previous=registers;
 auto colorInput=[&](unsigned selector,unsigned channel){
  if(selector<8)return previous[selector/2][selector&1?3:channel];
  if(selector==8)return texture[channel];if(selector==9)return texture[3];
  if(selector==10)return raster[channel];if(selector==11)return raster[3];
  if(selector==12)return 255;if(selector==13)return 128;
  if(selector==14)return constant[channel];return 0;
 };
 auto alphaInput=[&](unsigned selector){
  if(selector<4)return previous[selector][3];if(selector==4)return texture[3];
  if(selector==5)return raster[3];if(selector==6)return constant[3];return 0;
 };
 std::array<TevRgb,4> rgb{};std::array<int,4> alpha{};
 for(unsigned input=0;input<4;++input){
  for(unsigned channel=0;channel<3;++channel){int value=colorInput(stage.color.inputs[input],channel);rgb[input][channel]=input<3?value&255:value;}
  int value=alphaInput(stage.alpha.inputs[input]);alpha[input]=input<3?value&255:value;
 }
 TevRgb colorResult{};
 if(stage.color.bias==3)colorResult=tevColorCompare(rgb[0],rgb[1],rgb[2],rgb[3],stage.color.scale*2+stage.color.subtract,stage.color.clamp);
 else for(unsigned channel=0;channel<3;++channel)colorResult[channel]=tevArithmetic(rgb[0][channel],rgb[1][channel],rgb[2][channel],rgb[3][channel],stage.color.bias,stage.color.subtract,stage.color.scale,stage.color.clamp);
 int alphaResult=stage.alpha.bias==3?tevAlphaCompare(rgb[0],rgb[1],alpha[0],alpha[1],alpha[2],alpha[3],stage.alpha.scale*2+stage.alpha.subtract,stage.alpha.clamp):tevArithmetic(alpha[0],alpha[1],alpha[2],alpha[3],stage.alpha.bias,stage.alpha.subtract,stage.alpha.scale,stage.alpha.clamp);
 // Both computations read the same pre-stage register snapshot.
 std::copy(colorResult.begin(),colorResult.end(),registers[stage.color.destination].begin());
 registers[stage.alpha.destination][3]=alphaResult;
}
struct TevFragmentInputs {
 std::array<TevRgba,16> textures{},raster{};
};
// Callers supply the stage samples after texture-coordinate generation,
// indirect lookup and lighting. This core evaluates the authored TEV sequence.
inline TevRgba evaluateTevProgram(const GxCommandState& state,const TevFragmentInputs& inputs){
 TevRegisters registers{};for(unsigned id=0;id<4;++id)registers[id]=state.tevColor(id);
 auto count=state.tevStageCount();
 for(unsigned stage=0;stage<count;++stage){
  auto texture=state.tevOrder(stage).enabled?inputs.textures[stage]:TevRgba{255,255,255,255};
  evaluateTevStage(state,stage,registers,texture,inputs.raster[stage]);
 }
 auto last=state.tevStage(count-1);TevRgba result{};
 for(unsigned channel=0;channel<3;++channel)result[channel]=registers[last.color.destination][channel]&255;
 result[3]=registers[last.alpha.destination][3]&255;
 return result;
}
}

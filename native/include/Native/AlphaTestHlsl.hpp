#pragma once
#include <Native/GxCommandState.hpp>
#include <string>
namespace Native {
// GX comparison functions: NEVER, LESS, EQUAL, LEQUAL, GREATER,
// NEQUAL, GEQUAL, ALWAYS; alpha combines the two booleans.
inline std::string alphaTestHlsl(const GxCommandState& state){
 auto test=state.alphaCompare();
 auto comparison=[](unsigned function,unsigned reference){
  static const char* operators[]={"","<","==","<=",">","!=",">=",""};
  if(function==0)return std::string("false");
  if(function==7)return std::string("true");
  return "(result.w"+std::string(operators[function])+std::to_string(reference)+")";
 };
 auto a=comparison(test.first,test.firstReference),b=comparison(test.second,test.secondReference);
 static const char* operations[]={"&&","||","!=","=="};
 return "if(!("+a+operations[test.operation]+b+"))discard;";
}
}

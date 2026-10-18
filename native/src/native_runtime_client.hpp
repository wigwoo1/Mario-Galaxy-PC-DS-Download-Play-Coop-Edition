#pragma once
#include "native_runtime_api.h"
#include <windows.h>
#include <filesystem>
#include <vector>
#include <array>
#include <stdexcept>
#include <string>
class NativeRuntimeClient {
 HMODULE module=nullptr;
 void* scene=nullptr;
 decltype(&galaxy_runtime_error) error=nullptr;
 decltype(&galaxy_runtime_destroy) destroy=nullptr;
 decltype(&galaxy_runtime_load_terrain) load=nullptr;
 decltype(&galaxy_runtime_tick) tickFunction=nullptr;
 decltype(&galaxy_runtime_bind) bindFunction=nullptr;
 template<class T>T entry(const char* name){auto p=GetProcAddress(module,name);if(!p)throw std::runtime_error(std::string("Missing native runtime export: ")+name);return reinterpret_cast<T>(p);}
 void require(int result){if(!result)throw std::runtime_error(std::string("Native runtime: ")+error());}
public:
 size_t bindCalls=0,contactCalls=0;
 explicit NativeRuntimeClient(int zones=1){
  wchar_t path[32768];auto length=GetModuleFileNameW(nullptr,path,32768);if(!length||length==32768)throw std::runtime_error("Cannot locate PC executable");
  auto library=std::filesystem::path(std::wstring(path,length)).parent_path()/L"galaxy-runtime.dll";
  module=LoadLibraryW(library.c_str());if(!module)throw std::runtime_error("Cannot load galaxy-runtime.dll beside PC executable");
  try{
   auto version=entry<decltype(&galaxy_runtime_version)>("galaxy_runtime_version");if(version()!=1)throw std::runtime_error("Unsupported native runtime version");
   error=entry<decltype(error)>("galaxy_runtime_error");destroy=entry<decltype(destroy)>("galaxy_runtime_destroy");load=entry<decltype(load)>("galaxy_runtime_load_terrain");tickFunction=entry<decltype(tickFunction)>("galaxy_runtime_tick");bindFunction=entry<decltype(bindFunction)>("galaxy_runtime_bind");
   scene=entry<decltype(&galaxy_runtime_create)>("galaxy_runtime_create")(zones);require(scene!=nullptr);
  }catch(...){FreeLibrary(module);module=nullptr;throw;}
 }
 ~NativeRuntimeClient(){if(scene)destroy(scene);if(module)FreeLibrary(module);}
 NativeRuntimeClient(const NativeRuntimeClient&)=delete;
 NativeRuntimeClient& operator=(const NativeRuntimeClient&)=delete;
 void loadTerrain(const std::vector<uint8_t>& kcl,const std::vector<uint8_t>& metadata,const std::array<float,12>& matrix,const std::string& name,int zone=0){require(load(scene,kcl.data(),kcl.size(),metadata.data(),metadata.size(),matrix.data(),name.c_str(),zone));}
 void tick(){require(tickFunction(scene));}
 struct Result {std::array<float,3> displacement;uint32_t contacts;};
 Result bind(const std::array<float,3>& position,const std::array<float,3>& gravity,const std::array<float,3>& movement,float radius){Result result{};require(bindFunction(scene,position.data(),gravity.data(),movement.data(),radius,result.displacement.data(),&result.contacts));++bindCalls;if(result.contacts)++contactCalls;return result;}
};

#include "../native_model_env_init.hpp"
namespace MR {
bool isEnvelope(J3DMaterial* material){return NativeModelInit::isEnvelope(material);}
bool isUseTexMtxEnvMap(J3DMaterial* material){return NativeModelInit::isUseTexMtxEnvMap(material);}
bool isUseTexMtxProjMap(J3DMaterial* material){return NativeModelInit::isUseTexMtxProjMap(material);}
}

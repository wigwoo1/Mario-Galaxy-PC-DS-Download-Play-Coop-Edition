#pragma once
#include <JSystem/J3DGraphLoader/J3DModelLoader.hpp>

// Native BDL display lists are rebuilt from MAT3 through the normal factory.
// Retain the material capabilities that the original patched BDL reader owns.
inline u32 nativeModelMaterialFlags(bool binaryDisplayLists,u32 flags){
    if(binaryDisplayLists)
        // Patched BDL blocks retain all TEV registers even with one authored
        // stage. Select equivalent normal-factory storage capacity; the factory
        // still sets the active stage count from MAT3's newTevStageNum().
        flags=(flags&~(0xf0000000u|0x001f0000u))|J3DMLF_Material_Color_LightOn|J3DMLF_Material_PE_Full|J3DMLF_21;
    return flags;
}

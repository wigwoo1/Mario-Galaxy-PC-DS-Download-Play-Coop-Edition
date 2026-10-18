#include "native_material_init.hpp"
// The decoded owner must outlive this factory and retain its vector storage.
J3DMaterialFactory::J3DMaterialFactory(NativeMaterialInitTables& t){
 if(!t.postTexCoords.empty())throw std::runtime_error("Post texture-coordinate record layout is not yet verified");
 auto pointer=[](auto& values){return values.empty()?nullptr:values.data();};
 mMaterialNum=t.header.materialCount;mpMaterialInitData=pointer(t.records);mpMaterialID=pointer(t.header.remap);mpIndInitData=pointer(t.indirect);
 mpMatColor=pointer(t.materialColors);mpColorChanNum=pointer(t.colorChannelCounts);mpColorChanInfo=pointer(t.colorChannels);mpAmbColor=pointer(t.ambientColors);mpLightInfo=pointer(t.lights);
 mpTexGenNum=pointer(t.texGenCounts);mpTexCoordInfo=pointer(t.texCoords);mpTexCoord2Info=nullptr;mpTexMtxInfo=pointer(t.textureMatrices);field_0x34=pointer(t.postTextureMatrices);mpTexNo=pointer(t.textureNumbers);mpCullMode=pointer(t.cullModes);
 mpTevOrderInfo=pointer(t.tevOrders);mpTevColor=pointer(t.tevColors);mpTevKColor=pointer(t.konstColors);mpTevStageNum=pointer(t.tevStageCounts);mpTevStageInfo=pointer(t.tevStages);mpTevSwapModeInfo=pointer(t.swapModes);mpTevSwapModeTableInfo=pointer(t.swapTables);
 mpFogInfo=pointer(t.fog);mpAlphaCompInfo=pointer(t.alphaTests);mpBlendInfo=pointer(t.blends);mpZModeInfo=pointer(t.depthModes);mpZCompLoc=pointer(t.depthCompareLocations);mpDither=pointer(t.ditherFlags);mpNBTScaleInfo=pointer(t.nbtScales);
 mpDisplayListInit=nullptr;mpPatchingInfo=nullptr;mpCurrentMtxInfo=nullptr;mpMaterialMode=nullptr;
}


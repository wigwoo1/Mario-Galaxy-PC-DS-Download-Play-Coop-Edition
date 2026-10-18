& "$PSScriptRoot\generate-material-struct.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$header=[IO.File]::ReadAllText("$p\Petari\include\Game\Animation\XanimeCore.hpp")
if(!$header.Contains('u32 _68;')){throw 'joint transform pointer field anchor'}
$header=$header.Replace('u32 _68;','std::uintptr_t _68;').Replace('#pragma once',"#pragma once`n#include <cstdint>")
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Animation" | Out-Null
$xanimeHeaderPath="$n\include-runtime\Game\Animation\XanimeCore.hpp"
if(-not (Test-Path -LiteralPath $xanimeHeaderPath)){[IO.File]::WriteAllText($xanimeHeaderPath,$header)}
foreach($unit in @('ObjUtil','JMapUtil','LiveActorUtil')){
$source=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\$unit.cpp")
switch($unit){
'ObjUtil'{if(!$source.Contains('#include <va_list.h>')){throw 'varargs anchor'};$source=$source.Replace('#include <va_list.h>','#include <cstdarg>')
$csvPattern='JMapInfo\* pInfo = new JMapInfo\(\);\s*pInfo->attach\(pHolder->mFileInfoTable->getRes\(buf\)\);\s*return pInfo;'
if([regex]::Matches($source,$csvPattern).Count-ne 1){throw 'Native resource CSV parser anchor'}
$csvReplacement=@'
const ResFileInfo* resource = pHolder->mFileInfoTable->findFileInfo(buf);
        if (resource == nullptr || resource->mResource == nullptr || static_cast<s32>(resource->_4) < 16) {
            throw std::runtime_error("Invalid native CSV resource extent");
        }
        const auto* bytes = static_cast<const std::uint8_t*>(resource->mResource);
        auto pInfo = std::make_unique<JMapInfo>();
        pInfo->attachNative(std::vector<std::uint8_t>(bytes, bytes + resource->_4));
        return pInfo.release();
'@
$source=[regex]::Replace($source,$csvPattern,$csvReplacement)
$source="#include <memory>`n#include <vector>`n#include <cstdint>`n#include <stdexcept>`n"+$source
# Native scene scheduling supplies this platform hook; keep all other original utility methods.
$signature='void MR::connectToScene(NameObj*'
$b=$source.IndexOf($signature);if($b -lt 0){throw 'native scene hook anchor'};$o=$source.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($source[$e] -eq '{'){$d++};if($source[$e] -eq '}'){$d--};$e++};$source=$source.Remove($b,$e-$b)}
'JMapUtil'{if(([regex]::Matches($source,'std::atan2\(')).Count -ne 4){throw 'placement atan count'};$source=$source.Replace('std::atan2(','::atan2f(')}
'LiveActorUtil'{
$start=$source.IndexOf('        switch (scaleType) {');if($start -lt 0){throw 'actor collision switch anchor'};$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
$source=$source.Remove($start,$end-$start).Insert($start,'        Native::initializeActorCollision(*parts,*pResHolder->mArchive,rMtx,pSensor,kclRes,paRes,unk,static_cast<int>(scaleType));')
$source=$source.Replace('        CollisionParts* parts = new CollisionParts();','        if(!pResHolder||!pResHolder->mArchive||!pResHolder->mFileInfoTable||!pName)throw std::invalid_argument("Invalid collision resources");'+[Environment]::NewLine+'        std::unique_ptr<CollisionParts,Native::ActorCollisionDeleter> owned(new CollisionParts());'+[Environment]::NewLine+'        CollisionParts* parts=owned.get();')
$start=$source.IndexOf('    CollisionParts* createCollisionParts(');$end=$source.IndexOf('    const char* createSubModelObjName',$start);$factory=$source.Substring($start,$end-$start);$source=$source.Remove($start,$end-$start).Insert($start,$factory.Replace('return parts;','return owned.release();'))
$source='#include <Native/ActorCollision.hpp>'+[Environment]::NewLine+$source
if(!$source.Contains('pTransform->_68 = (u32)pMtx;')){throw 'matrix pointer anchor'};$source=$source.Replace('pTransform->_68 = (u32)pMtx;','pTransform->_68 = reinterpret_cast<std::uintptr_t>(pMtx);').Replace('getResName(0UL)','getResName(static_cast<u32>(0))')}
}
[IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$source)
}$display=[IO.File]::ReadAllText("$p\Petari\src\Game\LiveActor\DisplayListMaker.cpp")
$anchor='std::mem_fun(&MaterialCtrl::update)'
if(!$display.Contains($anchor)){throw 'display material update anchor'}
$display=$display.Replace($anchor,'[](MaterialCtrl* material) { material->update(); }')
[IO.File]::WriteAllText("$n\src\generated\DisplayListMaker.cpp",$display)
$resource=[IO.File]::ReadAllText("$p\Petari\src\Game\System\ResourceHolder.cpp")
$btiAnchor='return mFileInfoTable->add(pName, pData, false);'
if(!$resource.Contains($btiAnchor)){throw 'Standalone texture resource anchor missing'}
$resource='#include "../native_owned_bti.hpp"'+"`n"+$resource.Replace($btiAnchor,'if (strstr(pName, ".bti") != nullptr) return mFileInfoTable->add(pName, loadNativeArchiveBti(*mArchive, pData, mHeap), false);'+"`n    "+$btiAnchor)
$bckAnchor='return mMotionResTable->add(pName, pData == nullptr ? nullptr : J3DAnmLoaderDataBase::load(pData), true);'
if(!$resource.Contains($bckAnchor)){throw 'Original BCK resource loading anchor changed'}
$resource=$resource.Replace($bckAnchor,'return mMotionResTable->add(pName, Native::loadArchiveBck(*mArchive, pData), true);')
$btpAnchor='return mBtpResTable->add(pName, J3DAnmLoaderDataBase::load(pData), true);'
if(!$resource.Contains($btpAnchor)){throw 'Original BTP resource loading anchor changed'}
$resource=$resource.Replace($btpAnchor,'return mBtpResTable->add(pName, loadNativeArchiveTexturePattern(*mArchive, pData), true);')
$resource='#include "../native_texture_pattern_animation.hpp"'+"`n"+$resource
$btkAnchor='return mBtkResTable->add(pName, J3DAnmLoaderDataBase::load(pData), true);'
if(!$resource.Contains($btkAnchor)){throw 'Original BTK resource loading anchor changed'}
$resource=$resource.Replace($btkAnchor,'return mBtkResTable->add(pName, loadNativeArchiveTextureSrt(*mArchive, pData), true);')
$resource='#include "../native_texture_srt_animation.hpp"'+"`n"+$resource
$bvaAnchor='return mBvaResTable->add(pName, J3DAnmLoaderDataBase::load(pData), true);'
if(!$resource.Contains($bvaAnchor)){throw 'Original BVA resource loading anchor changed'}
$resource=$resource.Replace($bvaAnchor,'return mBvaResTable->add(pName, Native::loadArchiveVisibility(*mArchive, pData), true);')
$resource='#include "../native_visibility_animation.hpp"'+"`n"+$resource
$resource='#include <Native/BckAnimation.hpp>'+"`n"+$resource
$resource='#include "Native/HeapPlacement.hpp"'+"`n"+$resource
$modelAnchor='J3DModelLoaderDataBase::loadBinaryDisplayList(pData, J3DMLF_Material_UseIndirect | J3DMLF_UseUniqueMaterials)'
if(!$resource.Contains($modelAnchor)){throw 'Original BDL resource loading anchor changed'}
$resource=$resource.Replace($modelAnchor,'loadNativeArchiveModel(*mArchive, pData, *mHeap, J3DMLF_Material_UseIndirect | J3DMLF_UseUniqueMaterials).release()')
$resource=$resource.Replace('        MR::initEnvelopeAndEnvMapOrProjMapModelData(modelData);','        // Native owned loader performs the original initialization while retaining rewrites.')
$bdlStart=$resource.IndexOf('    if (strstr(pName, ".bdl") != nullptr) {')
$bdlEnd=$resource.IndexOf('    if (strstr(pName, ".bmd") != nullptr) {',$bdlStart)
if($bdlStart -lt 0 -or $bdlEnd -lt 0){throw 'BDL branch scope anchor changed'}
$bdl=@"
    if (strstr(pName, ".bdl") != nullptr) {
        std::unique_ptr<J3DModelData> owned;
        {
            struct MutexScope {
                OSMutex* mutex;
                explicit MutexScope(OSMutex* value):mutex(value){OSLockMutex(mutex);}
                ~MutexScope(){OSUnlockMutex(mutex);}
            } lock(&MR::MutexHolder<0>::sMutex);
            owned=loadNativeArchiveModel(*mArchive,pData,*mHeap,J3DMLF_Material_UseIndirect|J3DMLF_UseUniqueMaterials);
        }
        J3DModelData* modelData=owned.get();
        if (MR::isUseFur(modelData) || MR::strcasecmp(pName, "MarioShadow.bdl") == 0) {
            static_cast<NativeOwnedModelData*>(modelData)->downFracVtx();
        }
        ResFileInfo* result=mModelResTable->add(pName,modelData,true);
        owned.release();
        return result;
    }

"@
$resource=$resource.Remove($bdlStart,$bdlEnd-$bdlStart).Insert($bdlStart,$bdl)
$resource='#include "../native_owned_model_data.hpp"'+"`n"+$resource
[IO.File]::WriteAllText("$n\src\generated\ResourceHolder.cpp",$resource)
$info=[IO.File]::ReadAllText("$p\Petari\src\Game\System\ResourceInfo.cpp")
if(!$info.Contains('#include <size_t.h>')){throw 'resource size header anchor'}
$info=$info.Replace('#include <size_t.h>','#include <cstddef>')
[IO.File]::WriteAllText("$n\src\generated\ResourceInfo.cpp",$info)
# This unreferenced helper forces Metrowerks template emission for binary matching.
$anm=[IO.File]::ReadAllText("$p\Petari\src\Game\Animation\AnmPlayer.cpp")
$signature='int (*AnmPlayer_FORCE_EMIT('
$b=$anm.IndexOf($signature);if($b -lt 0){throw 'animation force emission anchor'}
$o=$anm.IndexOf('{',$b);$d=1;$e=$o+1
while($d){if($anm[$e] -eq '{'){$d++};if($anm[$e] -eq '}'){$d--};$e++}
$anm=$anm.Remove($b,$e-$b)
[IO.File]::WriteAllText("$n\src\generated\AnmPlayer.cpp",$anm)
$modelLoader=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphLoader\J3DModelLoader.cpp")
$modelLoader='#include "Native/HeapPlacement.hpp"'+"`n"+$modelLoader
[IO.File]::WriteAllText("$n\src\generated\J3DModelLoader.cpp",$modelLoader)
$joint=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphAnimator\J3DJoint.cpp")
$joint='#include "JSystem/JMath/JMath.hpp"'+"`n"+$joint
[IO.File]::WriteAllText("$n\src\generated\J3DJoint.cpp",$joint)
$drawBuffer=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphBase\J3DDrawBuffer.cpp")
$drawBuffer='#include <Native/TexturePointers.hpp>'+"`n"+$drawBuffer.Replace('(reinterpret_cast< uintptr_t >(pTexture->getResTIMG(texNo)) + pTexture->getResTIMG(texNo)->mImageDataOffset)','reinterpret_cast<uintptr_t>(Native::textureImage(pTexture->getResTIMG(texNo)))')
$drawBuffer='#include "Native/HeapPlacement.hpp"'+"`n"+$drawBuffer
[IO.File]::WriteAllText("$n\src\generated\J3DDrawBuffer.cpp",$drawBuffer)
$textureTevs=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphBase\J3DTevs.cpp")
$textureTevs='#include <Native/TexturePointers.hpp>'+"`n"+$textureTevs.Replace('(u8*)resTIMG + resTIMG->mImageDataOffset','Native::textureImage(resTIMG)').Replace('(u8*)resTIMG + resTIMG->mPaletteDataOffset','Native::texturePalette(resTIMG)')
[IO.File]::WriteAllText("$n\src\generated\J3DTevs.cpp",$textureTevs)
$mtxBuffer=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphAnimator\J3DMtxBuffer.cpp")
$signature='void J3DMtxBuffer::calcWeightEnvelopeMtx()'
$b=$mtxBuffer.IndexOf($signature);if($b -lt 0){throw 'weighted envelope method anchor'}
$o=$mtxBuffer.IndexOf('{',$b);$d=1;$e=$o+1
while($d){if($mtxBuffer[$e] -eq '{'){$d++};if($mtxBuffer[$e] -eq '}'){$d--};$e++}
$body=@'
void J3DMtxBuffer::calcWeightEnvelopeMtx() {
    const u16* indices=mJointTree->getWEvlpMixMtxIndex();
    const f32* weights=mJointTree->getWEvlpMixWeight();
    for(unsigned i=0;i<mJointTree->getWEvlpMtxNum();++i){
        const unsigned count=mJointTree->getWEvlpMixMtxNum(i);
        mpEvlpScaleFlagArr[i]=Native::envelopeMatrix(mpAnmMtx,mJointTree->mInvJointMtx,indices,weights,mpScaleFlagArr,count,mpWeightEvlpMtx[i]);
        indices+=count;weights+=count;
    }
}
'@
$mtxBuffer=$mtxBuffer.Remove($b,$e-$b).Insert($b,$body)
$mtxBuffer='#include "Native/HeapPlacement.hpp"'+"`n"+'#include "Native/EnvelopeMatrix.hpp"'+"`n"+$mtxBuffer
$mtxBuffer=$mtxBuffer.Replace('new Mtx*[mtxNum]','new Mtx*[mtxNum]{}').Replace('new Mtx33*[mtxNum]','new Mtx33*[mtxNum]{}').Replace('new Mtx33**[(u16)materialCount]','new Mtx33**[(u16)materialCount]{}')
$mtxBuffer="#include <Native/HeapPlacement.hpp>"+"`r`n"+$mtxBuffer
$mtxBuffer=& (Join-Path $PSScriptRoot 'adapt-native-matrix-storage.ps1') -SourceText $mtxBuffer
[IO.File]::WriteAllText("$n\src\generated\J3DMtxBuffer.cpp",$mtxBuffer)
$transform=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphBase\J3DTransform.cpp")
$scalarStart=$transform.IndexOf('void J3DGetTranslateRotateMtx(')
$scalarEnd=$transform.IndexOf('asm void J3DScaleNrmMtx(')
if($scalarStart -lt 0 -or $scalarEnd -le $scalarStart){throw 'texture transform scalar anchors'}
$scalarTransforms=$transform.Substring($scalarStart,$scalarEnd-$scalarStart)
$b=$transform.IndexOf('asm void J3DPSCalcInverseTranspose(');if($b -lt 0){throw 'normal matrix method anchor'}
$transform=$transform.Substring(0,$b).Replace('JMath::fastSqrt(','std::sqrt(').Replace('PSVECNormalize(&vec, &vec);','const f32 inverseLength=1.0f/std::sqrt(vec.x*vec.x+vec.y*vec.y+vec.z*vec.z); vec.x*=inverseLength; vec.y*=inverseLength; vec.z*=inverseLength;')
$transform+=@'
void J3DPSCalcInverseTranspose(Mtx src,Mtx33 dst) {
    Mtx inverse;
    if(!PSMTXInverse(src,inverse))return;
    for(int r=0;r<3;++r)for(int c=0;c<3;++c)dst[r][c]=inverse[c][r];
}
void J3DPSMtxArrayConcat(Mtx left,Mtx input,Mtx output,u32 count) {
    Mtx stableLeft;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)stableLeft[r][c]=left[r][c];
    for(u32 i=0;i<count;++i)PSMTXConcat(stableLeft,input+3*i,output+3*i);
}
void J3DScaleNrmMtx(Mtx matrix,const Vec& scale) {
    const f32 x=scale.x,y=scale.y,z=scale.z;
    for(int r=0;r<3;++r){matrix[r][0]*=x;matrix[r][1]*=y;matrix[r][2]*=z;}
}
void J3DScaleNrmMtx33(Mtx33 matrix,const Vec& scale) {
    const f32 x=scale.x,y=scale.y,z=scale.z;
    for(int r=0;r<3;++r){matrix[r][0]*=x;matrix[r][1]*=y;matrix[r][2]*=z;}
}
void J3DMtxProjConcat(Mtx left,Mtx right,Mtx dst) {
    // Original assembly reads four rows from the right projection matrix,
    // despite the SDK's Mtx parameter spelling (a decayed pointer).
    Mtx result;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c){
        f32 value=left[r][0]*right[0][c];
        for(int k=1;k<4;++k)value+=left[r][k]*right[k][c];
        result[r][c]=value;
    }
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=result[r][c];
}
'@
[IO.File]::WriteAllText("$n\src\generated\J3DTransformPort.cpp",$transform+"`n"+$scalarTransforms)
foreach($unit in @('J3DShape','J3DShapeDraw','J3DShapeFactory','J3DPacket')){
    $category=if($unit -eq 'J3DShapeFactory'){'J3DGraphLoader'}else{'J3DGraphBase'}
    $shape=[IO.File]::ReadAllText("$p\Petari\src\JSystem\$category\$unit.cpp")
    if($unit -eq 'J3DShape'){
        $shape='#include <Native/GDCurrentScope.hpp>'+"`n"+$shape.Replace('void J3DShape::makeVcdVatCmd() {', 'void J3DShape::makeVcdVatCmd() {'+"`n    Native::GDCurrentScope restoreWriter;")
        $shape='#include <Native/GpuResources.hpp>'+"`n"+$shape
        $shape=$shape.Replace('((uintptr_t)data & 0x7FFFFFFF)','(data ? Native::gpuResources().byteAddress(data) : 0)')
        $anchor='GDSetArrayRaw((GXAttr)(i + GX_VA_POS), nullptr, stride[i]);'
        if(!$shape.Contains($anchor)){throw 'shape raw array anchor'}
        $shape=$shape.Replace($anchor,'GDSetArrayRaw((GXAttr)(i + GX_VA_POS), 0, stride[i]);')
    }else{
        $shape='#include "Native/HeapPlacement.hpp"'+"`n"+'#ifndef ALIGN_NEXT'+"`n"+'#define ALIGN_NEXT(X,N) (((X)+(N)-1)&~((N)-1))'+"`n"+'#endif'+"`n"+$shape
    }
    if($unit -eq 'J3DPacket'){
        & (Join-Path $n 'generate-j3d-packet.ps1')
        $shape=[IO.File]::ReadAllText((Join-Path $n 'src\generated\J3DPacket.cpp'))
    }
    if($unit -eq 'J3DShapeDraw'){
        $shape=$shape.Replace('*reinterpret_cast< u16* >(dl)','(u16(dl[0])<<8)|dl[1]').Replace('*reinterpret_cast< u16* >(oldDL)','(u16(oldDL[0])<<8)|oldDL[1]').Replace('*reinterpret_cast< u16* >(newDL) = vtxNum;','newDL[0]=u8(vtxNum>>8);newDL[1]=u8(vtxNum);')
    }
    [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$shape)
}
foreach($unit in @('J3DMaterialFactory','J3DMaterialFactory_v21')){
    $factory=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphLoader\$unit.cpp")
    if(!$factory.Contains('? TRUE : FALSE')){throw 'material BOOL constant anchor'}
    $factory=$factory.Replace('? TRUE : FALSE','? 1 : 0')
    [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$factory)
}
$model=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphAnimator\J3DModel.cpp")
if(!$model.Contains('mbHasScale == TRUE')){throw 'model scale BOOL anchor'}
$model=$model.Replace('mbHasScale == TRUE','mbHasScale == 1')
[IO.File]::WriteAllText("$n\src\generated\J3DModel.cpp",$model)
$math=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JMath\JMath.cpp")
$b=$math.IndexOf('void JMAQuatLerp(');if($b -lt 0){throw 'JMath interpolation anchor'}
$math=$math.Substring(0,$b)
$math+=@'
void JMAQuatLerp(const Quaternion* p,const Quaternion* q,f32 t,Quaternion* dst){
    const Quaternion a=*p,b=*q;
    const f32 dot=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
    const f32 sign=dot<0?-1.0f:1.0f;
    dst->x=a.x+t*(sign*b.x-a.x);dst->y=a.y+t*(sign*b.y-a.y);
    dst->z=a.z+t*(sign*b.z-a.z);dst->w=a.w+t*(sign*b.w-a.w);
}
void JMAVECScaleAdd(const Vec* a,const Vec* b,Vec* dst,f32 scale){
    const Vec result{a->x*scale+b->x,a->y*scale+b->y,a->z*scale+b->z};*dst=result;
}
void JMAVECLerp(const Vec* a,const Vec* b,Vec* dst,f32 t){
    const Vec result{a->x+(b->x-a->x)*t,a->y+(b->y-a->y)*t,a->z+(b->z-a->z)*t};*dst=result;
}
void JMAMTXApplyScale(const Mtx src,Mtx dst,f32 x,f32 y,f32 z){
    Mtx result;
    for(int r=0;r<3;++r){result[r][0]=src[r][0]*x;result[r][1]=src[r][1]*y;result[r][2]=src[r][2]*z;result[r][3]=src[r][3];}
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=result[r][c];
}
'@
[IO.File]::WriteAllText("$n\src\generated\JMath.cpp",$math)
$gd=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphBase\J3DGD.cpp")
foreach($resource in @('image_ptr','tlut_ptr')){
    $anchor="OSCachedToPhysical($resource)"
    if(!$gd.Contains($anchor)){throw 'GD texture address anchor'}
    $gd=$gd.Replace($anchor,"Native::gpuResourceAddress($resource)")
}
$gd=$gd.Replace('mag_filt == TRUE','mag_filt == 1')
$gd='#include "Native/GDCompat.hpp"'+"`n"+$gd
[IO.File]::WriteAllText("$n\src\generated\J3DGD.cpp",$gd)
& "$n\generate-gd-fifo.ps1"


$adaptor=[IO.File]::ReadAllText("$p\Petari\src\Game\NameObj\NameObjAdaptor.cpp")
$adaptor='#include "Game/Util/Functor.hpp"'+"`n"+$adaptor
[IO.File]::WriteAllText("$n\src\generated\NameObjAdaptor.cpp",$adaptor)

$watcher=[IO.File]::ReadAllText("$p\Petari\src\Game\Map\SwitchWatcherHolder.cpp")
$watcherAnchor='std::mem_func(&SwitchWatcher::movement)'
if(!$watcher.Contains($watcherAnchor)){throw 'Original switch watcher movement anchor'}
$watcher=$watcher.Replace($watcherAnchor,'[](SwitchWatcher* watcher) { watcher->movement(); }')
[IO.File]::WriteAllText("$n\src\generated\SwitchWatcherHolder.cpp",$watcher)
$drawHolder=[IO.File]::ReadAllText("$p\Petari\src\Game\System\DrawBufferHolder.cpp")
foreach($method in @('allocateActorListBuffer','entry')){
    $anchor="std::mem_func(&DrawBufferGroup::$method)"
    if(!$drawHolder.Contains($anchor)){throw "Draw buffer adapter anchor: $method"}
    $replacement=if($method -eq 'allocateActorListBuffer'){'[](DrawBufferGroup& group) { group.allocateActorListBuffer(); }'}else{'[](DrawBufferGroup* group) { group->entry(); }'}
    $drawHolder=$drawHolder.Replace($anchor,$replacement)
}
if(!$drawHolder.Contains('std::for_each_array(')){throw 'Draw buffer iteration anchor'}
$drawHolder=$drawHolder.Replace('std::for_each_array(','std::for_each(')
[IO.File]::WriteAllText("$n\src\generated\DrawBufferHolder.cpp",$drawHolder)
$sceneList=[IO.File]::ReadAllText("$p\Petari\src\Game\Scene\SceneNameObjListExecutor.cpp")
$sceneList=$sceneList.Replace('            -1,  // TODO','            static_cast<u32>(-1),  // TODO')
$sceneList=$sceneList.Replace('            MR::LightType_None,','            static_cast<u32>(MR::LightType_None),')
[IO.File]::WriteAllText("$n\src\generated\SceneNameObjListExecutor.cpp",$sceneList)
$movementController=[IO.File]::ReadAllText("$p\Petari\src\Game\Scene\SceneNameObjMovementController.cpp")
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Scene" | Out-Null
Copy-Item -LiteralPath "$p\Petari\include\Game\Scene\SceneNameObjMovementController.hpp" -Destination "$n\include-runtime\Game\Scene\SceneNameObjMovementController.hpp" -Force
$movementController='#include "Game/Scene/SceneFunction.hpp"'+"`n"+$movementController
$movementController=$movementController.Replace('    NEW_NERVE(StopSceneStateControlNone',"#undef ATTRIBUTE_WEAK`n#define ATTRIBUTE_WEAK`n    NEW_NERVE(StopSceneStateControlNone")
[IO.File]::WriteAllText("$n\src\generated\SceneNameObjMovementController.cpp",$movementController)
$intermission=[IO.File]::ReadAllText("$p\Petari\src\Game\Scene\IntermissionScene.cpp").Replace('#include <va_list.h>','#include <cstdarg>')
$intermission=$intermission.Replace('va_end();','va_end(list);')
[IO.File]::WriteAllText("$n\src\generated\IntermissionScene.cpp",$intermission)
$logo=[IO.File]::ReadAllText("$p\Petari\src\Game\Scene\LogoScene.cpp")
$logo=$logo.Replace('    NEW_NERVE(LogoSceneCensorshipFadein',"#undef ATTRIBUTE_WEAK`n#define ATTRIBUTE_WEAK`n    NEW_NERVE(LogoSceneCensorshipFadein")
[IO.File]::WriteAllText("$n\src\generated\LogoScene.cpp",$logo)
foreach($unit in @('DrawBufferGroup','DrawBufferExecuter')){
    $source=[IO.File]::ReadAllText("$p\Petari\src\Game\System\$unit.cpp")
    $class=if($unit -eq 'DrawBufferGroup'){'DrawBufferExecuter'}else{'LiveActor'}
    $methods=if($unit -eq 'DrawBufferGroup'){@('allocateActorListBuffer','calcViewAndEntry','drawOpa','drawXlu')}else{@('calcViewAndEntry')}
    foreach($method in $methods){
        $anchor="std::mem_func(&$class`::$method)"
        if(!$source.Contains($anchor)){throw "Original draw adapter anchor: $unit / $method"}
        $source=$source.Replace($anchor,"[]($class* object) { object->$method(); }")
    }
    [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$source)
}
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Screen" | Out-Null
Copy-Item -LiteralPath "$p\Petari\include\Game\Screen\LayoutActor.hpp" -Destination "$n\include-runtime\Game\Screen\LayoutActor.hpp" -Force
$layoutCore=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\LayoutCoreUtil.cpp")
if(!$layoutCore.Contains('TVec3f pos(rLocalPos);') -or !$layoutCore.Contains('convertLayoutPosToScreenPos(pScreenPos, layoutPos);')){throw 'Layout vector conversion anchors missing'}
$layoutCore=$layoutCore.Replace('TVec3f pos(rLocalPos);','TVec3f pos(rLocalPos.x, rLocalPos.y, 0.0f);').Replace('convertLayoutPosToScreenPos(pScreenPos, layoutPos);','convertLayoutPosToScreenPos(pScreenPos, TVec2f(layoutPos.x, layoutPos.y));')
[IO.File]::WriteAllText("$n\src\generated\LayoutCoreUtil.cpp",$layoutCore)
$layoutHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\Screen\LayoutActor.hpp")
$layoutHeader=[regex]::Replace($layoutHeader,'virtual ~LayoutActor\(\) \{\s*\}','virtual ~LayoutActor();')
[IO.File]::WriteAllText("$n\include-runtime\Game\Screen\LayoutActor.hpp",$layoutHeader)
$layoutActor=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\LayoutActor.cpp")+"`nLayoutActor::~LayoutActor() {}`n"
[IO.File]::WriteAllText("$n\src\generated\LayoutActor.cpp",$layoutActor)
$layoutUtil=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\LayoutUtil.cpp")
$iteratorAnchor='template nw4r::lyt::PaneList::Iterator nw4r::ut::LinkList< nw4r::lyt::Pane, 4 >::Iterator::operator++(int);'
if(!$layoutUtil.Contains($iteratorAnchor)){throw 'Layout iterator anchor missing'}
$layoutUtil=$layoutUtil.Replace($iteratorAnchor,'// Native iterator operator is inline; console explicit instantiation omitted.')
[IO.File]::WriteAllText("$n\src\generated\LayoutUtil.cpp",$layoutUtil)
$paneEffects=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\PaneEffectKeeper.cpp")
$paneEffects=$paneEffects.Replace('isExistPaneMtxRef(pName1) == nullptr','isExistPaneMtxRef(pName1) == false').Replace('std::mem_func(&MultiEmitter::playCalcAndDeleteForeverEmitter)','[](MultiEmitter* emitter) { emitter->playCalcAndDeleteForeverEmitter(); }')
[IO.File]::WriteAllText("$n\src\generated\PaneEffectKeeper.cpp",$paneEffects)
$pointerTarget=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\StarPointerTarget.cpp")
$pointerTarget=$pointerTarget.Replace('*pos.toTVec3()','TVec3f(pos.x, pos.y, pos.z)').Replace('rPointerPos.squared(screenPos)','TVec2f(rPointerPos.x-screenPos.x, rPointerPos.y-screenPos.y).squared()').Replace('pos.squared(rPos)','TVec2f(pos.x-rPos.x, pos.y-rPos.y).squared()').Replace('mtx.setInline(MR::getStarPointerProjMtx());','mtx.set(MR::getStarPointerProjMtx());')
[IO.File]::WriteAllText("$n\src\generated\StarPointerTarget.cpp",$pointerTarget)

$resourceManager=[IO.File]::ReadAllText("$p\Petari\src\Game\System\ResourceHolderManager.cpp")
$resourceManager=$resourceManager.Replace('get(), createResourceHolder,','get(), &ResourceHolderManager::createResourceHolder,').Replace('get(), createLayoutHolder,','get(), &ResourceHolderManager::createLayoutHolder,')
[IO.File]::WriteAllText("$n\src\generated\ResourceHolderManager.cpp",$resourceManager)
$asyncHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\System\FunctionAsyncExecutor.hpp")
if(!$asyncHeader.Contains('volatile bool mIsEnd;')){throw 'Async completion field anchor missing'}
$asyncHeader=$asyncHeader.Replace('#pragma once',"#pragma once`n#include <atomic>").Replace('volatile bool mIsEnd;','std::atomic<bool> mIsEnd;').Replace('bool mIsSuspended;','std::atomic<bool> mIsSuspended;').Replace('/* 0x40 */ const char* mName;','/* 0x40 */ std::atomic<const char*> mName;')
$asyncHeader=$asyncHeader.Replace('#include <atomic>',"#include <atomic>`n#include <JSystem/JKernel/JKRHeap.hpp>`n#include <new>").Replace('~FunctionAsyncExecInfo();',"~FunctionAsyncExecInfo();`n    static void operator delete(void* memory) noexcept { if(!memory)return; if(auto heap=JKRHeap::findFromRoot(memory))JKRHeap::free(memory,heap);else ::operator delete(memory); }")
[IO.File]::WriteAllText("$n\include-runtime\Game\System\FunctionAsyncExecutor.hpp",$asyncHeader)
function Convert-NativeAsyncCompletion([string]$source) {
    $pattern='OSSendMessage\(&info->mQueue, nullptr, OS_MESSAGE_NOBLOCK\);\s*info->mIsEnd = true;'
    if([regex]::Matches($source,$pattern).Count -ne 2){throw 'Async completion ordering anchors missing'}
    return [regex]::Replace($source,$pattern,"info->mIsEnd = true;`n        OSSendMessage(&info->mQueue, nullptr, OS_MESSAGE_NOBLOCK);")
}
$asyncSource=[IO.File]::ReadAllText("$p\Petari\src\Game\System\FunctionAsyncExecutor.cpp")
$asyncSource=Convert-NativeAsyncCompletion $asyncSource
$asyncSource="#include <Native/AsyncGxCommands.hpp>`n"+$asyncSource.Replace('    (*mFunc)();', "    Native::AsyncGxCommands graphicsCommands;`n    (*mFunc)();`n    graphicsCommands.commit();")
if(!$asyncSource.Contains('JKRUnitHeap::create(0x34, 0x34A8,')){throw 'Async executor pool sizing anchor missing'}
$asyncSource="#include <Native/HeapPlacement.hpp>`n"+$asyncSource.Replace('JKRUnitHeap::create(0x34, 0x34A8, 4,','JKRUnitHeap::create(sizeof(FunctionAsyncExecInfo), sizeof(JKRUnitHeap) + 256 * sizeof(FunctionAsyncExecInfo) + 32 + 2 * alignof(FunctionAsyncExecInfo), alignof(FunctionAsyncExecInfo),')
[IO.File]::WriteAllText("$n\src\generated\FunctionAsyncExecutor.cpp",$asyncSource)
$asyncOriginal=Convert-NativeAsyncCompletion ([IO.File]::ReadAllText("$p\Petari\src\Game\System\FunctionAsyncExecutor.cpp"))
$infoStart=$asyncOriginal.IndexOf('FunctionAsyncExecInfo::FunctionAsyncExecInfo(')
$infoEnd=$asyncOriginal.IndexOf('FunctionAsyncExecutorThread::FunctionAsyncExecutorThread(')
$mainStart=$asyncOriginal.IndexOf('FunctionAsyncExecutorOnMainThread::FunctionAsyncExecutorOnMainThread(')
$mainEnd=$asyncOriginal.IndexOf('FunctionAsyncExecutor::FunctionAsyncExecutor()')
if($infoStart -lt 0 -or $mainEnd -lt 0){throw 'Async job kernel anchors missing'}
$asyncKernel="#include <Game/Util/Functor.hpp>`n#include <Game/System/FunctionAsyncExecutor.hpp>`n"+$asyncOriginal.Substring($infoStart,$infoEnd-$infoStart)+$asyncOriginal.Substring($mainStart,$mainEnd-$mainStart)
[IO.File]::WriteAllText("$n\src\generated\AsyncJobKernel.cpp",$asyncKernel)




$heapOriginal=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRHeap.cpp")
$heapStart=$heapOriginal.IndexOf('void JKRHeap::destroy(')
$heapEnd=$heapOriginal.IndexOf('void JKRHeap::copyMemory(')
if($heapStart -lt 0 -or $heapEnd -le $heapStart){throw 'Original heap dispatch anchors missing'}
$heapKernel="#include <JSystem/JKernel/JKRHeap.hpp>`nJKRHeap* JKRHeap::sSystemHeap;`nJKRHeap* JKRHeap::sCurrentHeap;`nJKRHeap* JKRHeap::sRootHeap;`n"+$heapOriginal.Substring($heapStart,$heapEnd-$heapStart)
$heapHeader=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRHeap.hpp")
if(!$heapHeader.Contains('void dispose_subroutine(u32, u32);')){throw 'Heap disposal declaration anchor missing'}
$heapHeader=$heapHeader.Replace('#pragma once',"#pragma once`n#include <stdint.h>").Replace('void dispose_subroutine(u32, u32);','void dispose_subroutine(u32, u32); void dispose_subroutine(uintptr_t, uintptr_t);')
[IO.Directory]::CreateDirectory("$n\include-runtime\JSystem\JKernel")|Out-Null
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRHeap.hpp",$heapHeader)
$heapKernel=$heapKernel.Replace('dispose_subroutine(u32 start, u32 end)','dispose_subroutine(uintptr_t start, uintptr_t end)').Replace('u32 begin = (u32)ptr;','uintptr_t begin = (uintptr_t)ptr;').Replace('u32 end = (u32)ptr + size;','uintptr_t end = (uintptr_t)ptr + size;').Replace('dispose_subroutine((u32)begin, (u32)end);','dispose_subroutine((uintptr_t)begin, (uintptr_t)end);')
$heapKernel+="`nvoid JKRHeap::dispose_subroutine(u32 start,u32 end){dispose_subroutine(uintptr_t(start),uintptr_t(end));}`n"
$ctorStart=$heapOriginal.IndexOf('JKRHeap::JKRHeap(')
$ctorEnd=$heapOriginal.IndexOf('bool JKRHeap::initArena(')
if($ctorStart -lt 0 -or $ctorEnd -le $ctorStart){throw 'Heap construction anchors missing'}
$heapConstruction=$heapOriginal.Substring($ctorStart,$ctorEnd-$ctorStart)
$heapConstruction=$heapConstruction.Replace('mChildTree.getParent()->removeChild(&mChildTree);','if (mChildTree.getParent()) mChildTree.getParent()->removeChild(&mChildTree);')
$heapConstruction=$heapConstruction.Replace('JSUTree< JKRHeap >* nextRootHeap = sRootHeap->mChildTree.getFirstChild();','if (sRootHeap == this) { if(sCurrentHeap==this)sCurrentHeap=nullptr; if(sSystemHeap==this)sSystemHeap=nullptr; sRootHeap=nullptr; return; } JSUTree< JKRHeap >* nextRootHeap = sRootHeap ? sRootHeap->mChildTree.getFirstChild() : nullptr;')
$heapKernel+="`nJKRErrorHandler JKRHeap::mErrorHandler;`nstatic bool byte_806B70B8;`nstatic bool byte_806B26D8=true;`n"+$heapConstruction
$defaultStart=$heapOriginal.IndexOf('void JKRHeap::state_register(')
$heapKernel+="`nu32 ARALT_AramStartAdr=0x90000000;`n"+$heapOriginal.Substring($defaultStart)
[IO.File]::WriteAllText("$n\src\generated\JKRHeapDispatch.cpp",$heapKernel)


$handlerStart=$heapOriginal.IndexOf('JKRErrorHandler JKRHeap::setErrorHandler(')
$handlerEnd=$heapOriginal.IndexOf('void* operator new(', $handlerStart)
if($handlerStart -lt 0 -or $handlerEnd -le $handlerStart){throw 'Heap error handler anchors missing'}
$heapKernel+=$heapOriginal.Substring($handlerStart,$handlerEnd-$handlerStart)
[IO.File]::WriteAllText("$n\src\generated\JKRHeapDispatch.cpp",$heapKernel)

$unitHeap=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRUnitHeap.cpp")
$unitHeap="#include <Native/HeapPlacement.hpp>`n#include <stdint.h>`n#define ALIGN_NEXT(value, alignment) (((value) + uintptr_t((alignment)-1)) & ~uintptr_t((alignment)-1))`n"+$unitHeap.Replace('reinterpret_cast< u32 >(bat + ALIGN_NEXT(unitCount / 8, 4))','reinterpret_cast< uintptr_t >(bat + ALIGN_NEXT(unitCount / 8, 4))').Replace('reinterpret_cast< u32 >(bat) & 3','reinterpret_cast< uintptr_t >(bat) & 3')
$unitHeap="#include <Native/UnitHeapBitmap.hpp>`n"+$unitHeap.Replace('word = *words;','word = nativeUnitHeapBitmapWord(words);').Replace('words != end ? *words : 0xFFFFFFFF','words != end ? nativeUnitHeapBitmapWord(words) : 0xFFFFFFFF').Replace('checkCode += *word;','checkCode += nativeUnitHeapBitmapWord(word);')
$addressStart=$unitHeap.IndexOf('s32 JKRUnitHeap::addressToIndex(')
$addressEnd=$unitHeap.IndexOf('BOOL JKRUnitHeap::isUnitUsed(', $addressStart)
if($addressStart -lt 0 -or $addressEnd -le $addressStart){throw 'Unit heap address conversion anchors missing'}
$unitHeap=$unitHeap.Substring(0,$addressStart)+"s32 JKRUnitHeap::addressToIndex(void* address){return nativeUnitHeapAddressIndex(address,mUnits,mUnitSize,mUnitCount);}`n`n"+$unitHeap.Substring($addressEnd)
[IO.File]::WriteAllText("$n\src\generated\JKRUnitHeap.cpp",$unitHeap)



$unitHeap+=[IO.File]::ReadAllText("$n\src\unit_heap_allocation.inc")
[IO.File]::WriteAllText("$n\src\generated\JKRUnitHeap.cpp",$unitHeap)
$unitHeap=$unitHeap.Replace('JKRUnitHeap* JKRUnitHeap::create(',"#ifndef GALAXY_UNIT_HEAP_CONSTRUCTED_TEST`nJKRUnitHeap* JKRUnitHeap::create(").Replace('void JKRUnitHeap::do_destroy()',"#endif`nvoid JKRUnitHeap::do_destroy()")
[IO.File]::WriteAllText("$n\src\generated\JKRUnitHeap.cpp",$unitHeap)
$unitHeap=$unitHeap.Replace('return mTotalFreeSize;', 'lock(); s32 total=mTotalFreeSize; unlock(); return total;')
[IO.File]::WriteAllText("$n\src\generated\JKRUnitHeap.cpp",$unitHeap)
$disposer=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRDisposer.cpp")
$disposer=$disposer.Replace('mHeap->mDisposerList.append(&mLink);','mHeap->lock(); mHeap->mDisposerList.append(&mLink); mHeap->unlock();').Replace('mHeap->mDisposerList.remove(&mLink);','mHeap->lock(); mHeap->mDisposerList.remove(&mLink); mHeap->unlock();')
[IO.File]::WriteAllText("$n\src\generated\JKRDisposer.cpp",$disposer)

$freeStart=$unitHeap.IndexOf('s32 JKRUnitHeap::do_getFreeSize(')
$freeEnd=$unitHeap.IndexOf('s32 JKRUnitHeap::do_getTotalFreeSize(', $freeStart)
if($freeStart -lt 0 -or $freeEnd -le $freeStart){throw 'Unit heap free query anchors missing'}
$freeQueries="s32 JKRUnitHeap::do_getFreeSize(){lock();auto run=nativeUnitHeapLongestFreeRun(mBat,mUnitCount);unlock();return mUnitSize*run.count;}`nvoid* JKRUnitHeap::do_getMaxFreeBlock(){lock();auto run=nativeUnitHeapLongestFreeRun(mBat,mUnitCount);void* block=run.count?indexToAddress(run.start):nullptr;unlock();return block;}`n"
$unitHeap=$unitHeap.Substring(0,$freeStart)+$freeQueries+$unitHeap.Substring($freeEnd)
[IO.File]::WriteAllText("$n\src\generated\JKRUnitHeap.cpp",$unitHeap)
$solidHeap=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRSolidHeap.cpp")
$solidHeap="#include <Native/HeapPlacement.hpp>`n#define ALIGN_NEXT(value, alignment) (((value)+uintptr_t((alignment)-1)) & ~uintptr_t((alignment)-1))`n#define ALIGN_PREV(value, alignment) ((value) & ~uintptr_t((alignment)-1))`n"+$solidHeap
[IO.File]::WriteAllText("$n\src\generated\JKRSolidHeap.cpp",$solidHeap)
$heapKernel+="`nu32 JKRHeap::getMaxAllocatableSize(int alignment){ int64_t requested=alignment<0?-int64_t(alignment):int64_t(alignment);if(!requested)requested=4;if((requested & (requested-1)) || requested>UINT32_MAX)return 0;lock();s32 available=getFreeSize();uintptr_t start=reinterpret_cast<uintptr_t>(getMaxFreeBlock());uintptr_t padding=(-start)&uintptr_t(requested-1);u32 result=available>0 && start && padding<u32(available)?u32(available)-u32(padding):0;unlock();return result;}`n"
[IO.File]::WriteAllText("$n\src\generated\JKRHeapDispatch.cpp",$heapKernel)
$solidHeap+="`nvoid JKRSolidHeap::do_free(void* address){if(address)reportFreeBlock(address);}`n"
[IO.File]::WriteAllText("$n\src\generated\JKRSolidHeap.cpp",$solidHeap)
$expHeap=[IO.File]::ReadAllText("$p\Petari\src\JSystem\JKernel\JKRExpHeap.cpp")
$expHeap="#include <Native/HeapPlacement.hpp>`n#include <stdint.h>`n#define ALIGN_NEXT(value, alignment) (((value)+uintptr_t((alignment)-1)) & ~uintptr_t((alignment)-1))`n#define ALIGN_PREV(value, alignment) ((value) & ~uintptr_t((alignment)-1))`n"+$expHeap.Replace('reinterpret_cast< u32 >','reinterpret_cast< uintptr_t >').Replace('u32 content;','uintptr_t content;').Replace('u32 start;','uintptr_t start;').Replace('u32 end;','uintptr_t end;').Replace('u32 endAddr =','uintptr_t endAddr =').Replace('u32 nextAddr =','uintptr_t nextAddr =').Replace('arena + 0x90','arena + ALIGN_NEXT(sizeof(JKRExpHeap), 16)').Replace('arenaSize - 0x90','arenaSize - ALIGN_NEXT(sizeof(JKRExpHeap), 16)').Replace('alignedSize < 0xA0','alignedSize < heapSize + sizeof(CMemBlock)')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$expHeap=$expHeap.Replace('JKRExpHeap* JKRExpHeap::createRoot(',"#ifndef GALAXY_EXP_HEAP_CONSTRUCTED_TEST`nJKRExpHeap* JKRExpHeap::createRoot(").Replace('JKRExpHeap* JKRExpHeap::create(u32',"#endif`nJKRExpHeap* JKRExpHeap::create(u32")
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$expHeap=$expHeap.Replace('ALIGN_NEXT(size, 4)','ALIGN_NEXT(size, alignof(CMemBlock))')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$expHeap=$expHeap.Replace('static u32 DB','static thread_local u32 DB').Replace('static JKRExpHeap::CMemBlock* DB','static thread_local JKRExpHeap::CMemBlock* DB')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$expHeap=$expHeap.Replace('u8* data = (mem + heapSize);','u8* data = mem ? mem + heapSize : nullptr;')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$solidHeap=$solidHeap.Replace('void* dataPtr = mem + solidHeapSize;','void* dataPtr = mem ? mem + solidHeapSize : nullptr;')
[IO.File]::WriteAllText("$n\src\generated\JKRSolidHeap.cpp",$solidHeap)

$expHeap=$expHeap.Replace('JKRHeap::initArena(&arena, &arenaSize, heapNum);','if(!JKRHeap::initArena(&arena, &arenaSize, heapNum))return nullptr;').Replace('heap->mIsExternal = 1;', 'if(heap)heap->mIsExternal = 1;')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$memoryUtil=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\MemoryUtil.cpp")
$memoryUtil="#include <stdint.h>`n"+$memoryUtil.Replace('reinterpret_cast< u32 >(pSrc)','reinterpret_cast< uintptr_t >(pSrc)').Replace('reinterpret_cast< u32 >(pDst)','reinterpret_cast< uintptr_t >(pDst)')
[IO.File]::WriteAllText("$n\src\generated\MemoryUtil.cpp",$memoryUtil)
$memoryUtil=$memoryUtil.Replace('delete static_cast< u8* >(pPtr);','delete[] static_cast< u8* >(pPtr);').Replace('srcAlignment == dstAlignment && (size & 0xF) == 0','srcAlignment == 0 && dstAlignment == 0 && (size & 0xF) == 0')
[IO.File]::WriteAllText("$n\src\generated\MemoryUtil.cpp",$memoryUtil)
$copyStart=$memoryUtil.IndexOf('    void copyMemory(')
$copyEnd=$memoryUtil.IndexOf('    u32 calcCheckSum(', $copyStart)
if($copyStart -lt 0 -or $copyEnd -le $copyStart){throw 'Memory operation anchors missing'}
$memoryKernel="#include <Game/Util/MemoryUtil.hpp>`n#include <revolution/os/OSCache.h>`n#include <stdint.h>`n#include <cstring>`nnamespace MR {`n"+$memoryUtil.Substring($copyStart,$copyEnd-$copyStart)+"`n}`n"
[IO.File]::WriteAllText("$n\src\generated\MemoryOperationKernel.cpp",$memoryKernel)
$heapWatcher=[IO.File]::ReadAllText("$p\Petari\src\Game\System\HeapMemoryWatcher.cpp")
$heapWatcher="#include <Native/HeapPlacement.hpp>`n#include <stdint.h>`n"+$heapWatcher.Replace('u32 arenaHi, arenaLo;','uintptr_t arenaHi, arenaLo;').Replace('reinterpret_cast< u32 >','reinterpret_cast< uintptr_t >')
# The retail 20 MiB MEM2 stationed heap cannot hold the native port's retained layout resources.
$heapWatcher=$heapWatcher.Replace('0x1400000, HeapMemoryWatcher::sRootHeapGDDR3','0x2000000, HeapMemoryWatcher::sRootHeapGDDR3')
$heapWatcher=$heapWatcher.Replace('::createExpHeap(0x900000, JKRHeap::sRootHeap, false)', '::createExpHeap(0x900000 * (sizeof(void*) / sizeof(u32)), JKRHeap::sRootHeap, false)')
[IO.File]::WriteAllText("$n\src\generated\HeapMemoryWatcher.cpp",$heapWatcher)
$switchStart=$heapOriginal.IndexOf('JKRHeap* JKRHeap::becomeSystemHeap(')
$switchEnd=$heapOriginal.IndexOf('void JKRHeap::destroy(', $switchStart)
if($switchStart -lt 0 -or $switchEnd -le $switchStart){throw 'Original heap switch anchors missing'}
$heapKernel+=$heapOriginal.Substring($switchStart,$switchEnd-$switchStart)
[IO.File]::WriteAllText("$n\src\generated\JKRHeapDispatch.cpp",$heapKernel)
$memoryUtil=[regex]::Replace($memoryUtil,'_0 = JKRHeap::sCurrentHeap;\s*OSLockMutex\(&MR::MutexHolder< 1 >::sMutex\);',"OSLockMutex(&MR::MutexHolder< 1 >::sMutex);`n        _0 = JKRHeap::sCurrentHeap;")
[IO.File]::WriteAllText("$n\src\generated\MemoryUtil.cpp",$memoryUtil)
$restoreStart=$memoryUtil.IndexOf('    CurrentHeapRestorer::CurrentHeapRestorer(')
$restoreEnd=$memoryUtil.IndexOf('    void* NewDeleteAllocator::alloc(', $restoreStart)
$becomeStart=$memoryUtil.IndexOf('    void becomeCurrentHeap(')
$becomeEnd=$memoryUtil.IndexOf('    bool isEqualCurrentHeap(', $becomeStart)
if($restoreStart -lt 0 -or $restoreEnd -le $restoreStart -or $becomeEnd -le $becomeStart){throw 'Current heap restorer anchors missing'}
$restoreKernel="#include <Game/Util/MemoryUtil.hpp>`n#include <Game/Util/MutexHolder.hpp>`n#include <JSystem/JAudio2/JASMutex.hpp>`n#include <JSystem/JKernel/JKRHeap.hpp>`nnamespace MR {`n"+$memoryUtil.Substring($restoreStart,$restoreEnd-$restoreStart)+$memoryUtil.Substring($becomeStart,$becomeEnd-$becomeStart)+"`n}`n"
[IO.File]::WriteAllText("$n\src\generated\CurrentHeapKernel.cpp",$restoreKernel)
# Native ARAM backing addresses must retain host pointer width. The Wii DSP
# consumer is not a native audio backend and must not receive a truncated address.
$heapKernel=$heapKernel.Replace('u32 ARALT_AramStartAdr=0x90000000;', 'uintptr_t ARALT_AramStartAdr=0;').Replace('setAltAramStartAdr(u32 addr)', 'setAltAramStartAdr(uintptr_t addr)').Replace('u32 JKRHeap::getAltAramStartAdr()', 'uintptr_t JKRHeap::getAltAramStartAdr()')
[IO.File]::WriteAllText("$n\src\generated\JKRHeapDispatch.cpp",$heapKernel)
$nativeHeapHeader=[IO.File]::ReadAllText("$n\include-runtime\JSystem\JKernel\JKRHeap.hpp").Replace('setAltAramStartAdr(u32)', 'setAltAramStartAdr(uintptr_t)').Replace('static u32 getAltAramStartAdr()', 'static uintptr_t getAltAramStartAdr()')
[IO.File]::WriteAllText("$n\include-runtime\JSystem\JKernel\JKRHeap.hpp",$nativeHeapHeader)
$heapWatcher=$heapWatcher.Replace('    JKRExpHeap::createRoot(1, true);', '    if (JKRHeap::sRootHeap || sRootHeapGDDR3) OSPanic(__FILE__, __LINE__, "Root heaps already initialized");').Replace('    newHi = reinterpret_cast< void* >(arenaLo + 0xE00000);', @'
    if (!arenaLo || arenaHi <= arenaLo || arenaHi-arenaLo < 0xE00000 + sizeof(JKRExpHeap) + sizeof(JKRExpHeap::CMemBlock) + 32 || arenaHi-arenaLo > UINT32_MAX)
        OSPanic(__FILE__, __LINE__, "Invalid secondary heap arena");
    if (!JKRExpHeap::createRoot(1, true)) OSPanic(__FILE__, __LINE__, "Missing primary heap arena");
    newHi = reinterpret_cast< void* >(arenaLo + 0xE00000);
'@)
[IO.File]::WriteAllText("$n\src\generated\HeapMemoryWatcher.cpp",$heapWatcher)
$rootStart=$heapWatcher.IndexOf('void HeapMemoryWatcher::createRootHeap()')
$rootEnd=$heapWatcher.IndexOf('void HeapMemoryWatcher::createHeaps()', $rootStart)
$rootKernel="#include <stdint.h>`n#include <Game/System/HeapMemoryWatcher.hpp>`n#include <Game/Util/MemoryUtil.hpp>`n#include <JSystem/JKernel/JKRExpHeap.hpp>`n#include <revolution/os.h>`nJKRExpHeap* HeapMemoryWatcher::sRootHeapGDDR3;`nnamespace MR { bool isEqualCurrentHeap(JKRHeap* heap){return JKRHeap::sCurrentHeap==heap;} }`n"+$heapWatcher.Substring($rootStart,$rootEnd-$rootStart)
[IO.File]::WriteAllText("$n\src\generated\RootHeapKernel.cpp",$rootKernel)
# External native heap storage must be aligned, non-null and large enough for
# both the heap object and an allocator block before pointer arithmetic.
$externalAnchor='JKRExpHeap* JKRExpHeap::create(void* ptr, u32 size, JKRHeap* pParent, bool errorFlag) {'
$externalGuard="`n    const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);`n    if (!ptr || address % alignof(JKRExpHeap) || size < ALIGN_NEXT(sizeof(JKRExpHeap), 0x10) + sizeof(CMemBlock) || size > UINTPTR_MAX-address) return nullptr;`n    if (!pParent && !sRootHeap) return nullptr;"
$expHeap=$expHeap.Replace($externalAnchor,$externalAnchor+$externalGuard).Replace('    heap->_70 = ptr;', '    if (!heap) return nullptr;'+"`n"+'    heap->_70 = ptr;')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$heapWatcher="#include <Native/InputWorkspace.hpp>`n"+$heapWatcher.Replace('WPADGetWorkMemorySize()', 'nativeInputWorkspaceBytes()')
[IO.File]::WriteAllText("$n\src\generated\HeapMemoryWatcher.cpp",$heapWatcher)
$heapPrefixEnd=$heapWatcher.IndexOf('JKRHeap* HeapMemoryWatcher::getHeapNapa(')
$heapStartupStart=$heapWatcher.IndexOf('void HeapMemoryWatcher::createRootHeap()')
$heapStartupEnd=$heapWatcher.IndexOf('void HeapMemoryWatcher::checkRestMemory()', $heapStartupStart)
[IO.File]::WriteAllText("$n\src\generated\HeapStartupKernel.cpp",$heapWatcher.Substring(0,$heapPrefixEnd)+$heapWatcher.Substring($heapStartupStart,$heapStartupEnd-$heapStartupStart))
$sceneBegin=$heapWatcher.IndexOf('void HeapMemoryWatcher::createFileCacheHeapOnGameHeap(')
$sceneAdjust=$heapWatcher.IndexOf('void HeapMemoryWatcher::adjustStationedHeaps()', $sceneBegin)
$sceneResume=$heapWatcher.IndexOf('void HeapMemoryWatcher::setCurrentHeapToStationedHeap()', $sceneAdjust)
$sceneEnd=$heapWatcher.IndexOf('void HeapMemoryWatcher::createRootHeap()', $sceneResume)
[IO.File]::AppendAllText("$n\src\generated\HeapStartupKernel.cpp",$heapWatcher.Substring($sceneBegin,$sceneAdjust-$sceneBegin)+$heapWatcher.Substring($sceneResume,$sceneEnd-$sceneResume))
$stringOriginal=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\StringUtil.cpp")
$basenameStart=$stringOriginal.IndexOf('    const char* getBasename(')
$basenameEnd=$stringOriginal.IndexOf('    void extractString(', $basenameStart)
$equalStart=$stringOriginal.IndexOf('    bool isEqualString(')
$equalEnd=$stringOriginal.IndexOf('    bool isEqualStringCase(', $equalStart)
if($basenameStart -lt 0 -or $basenameEnd -le $basenameStart -or $equalStart -lt 0 -or $equalEnd -le $equalStart){throw 'String kernel anchors missing'}
[IO.File]::WriteAllText("$n\src\generated\StringKernel.cpp","#include <cstring>`n#include <Game/Util/StringUtil.hpp>`nnamespace MR {`n"+$stringOriginal.Substring($basenameStart,$basenameEnd-$basenameStart)+$stringOriginal.Substring($equalStart,$equalEnd-$equalStart)+"`n}`n")
$stringKernel=[IO.File]::ReadAllText("$n\src\generated\StringKernel.cpp").Replace('#include <Game/Util/StringUtil.hpp>', '// C ABI-independent original string implementations; diagnostic inline header is intentionally excluded.')
[IO.File]::WriteAllText("$n\src\generated\StringKernel.cpp",$stringKernel)
# Validate ownership range before inspecting allocator headers.
$expHeap=$expHeap.Replace('    if (size == 0xFFFFFFFF) {', '    if (!pParent) return nullptr;'+"`n"+'    if (size == 0xFFFFFFFF) {')
$rangeGuard="    const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);`n    if (!ptr || address < reinterpret_cast<uintptr_t>(mStart) + sizeof(CMemBlock) || address >= reinterpret_cast<uintptr_t>(mEnd) || address % alignof(CMemBlock)) { unlock(); return -1; }`n"
$expHeap=$expHeap.Replace('    CMemBlock* block = CMemBlock::getHeapBlock(ptr);'+"`r`n"+'    if (block == nullptr || ptr < mStart || mEnd < ptr) {',$rangeGuard+'    CMemBlock* block = CMemBlock::getHeapBlock(ptr);'+"`n"+'    if (block == nullptr || ptr < mStart || mEnd < ptr) {')
$expHeap=$expHeap.Replace('    CMemBlock* block = CMemBlock::getHeapBlock(ptr);'+"`r`n"+'    if (!block || ptr < mStart || mEnd < ptr) {',$rangeGuard+'    CMemBlock* block = CMemBlock::getHeapBlock(ptr);'+"`n"+'    if (!block || ptr < mStart || mEnd < ptr) {')
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$expHeap+="`n"+[IO.File]::ReadAllText("$n\src\exp_heap_adjust.inc")
[IO.File]::WriteAllText("$n\src\generated\JKRExpHeap.cpp",$expHeap)
$heapUtilStart=$memoryUtil.IndexOf('    bool isEqualCurrentHeap(')
$heapUtilEnd=$memoryUtil.IndexOf('    void copyMemory(', $heapUtilStart)
[IO.File]::AppendAllText("$n\src\generated\CurrentHeapKernel.cpp","`n#include <JSystem/JKernel/JKRExpHeap.hpp>`nnamespace MR {`n"+$memoryUtil.Substring($heapUtilStart,$heapUtilEnd-$heapUtilStart)+"`n}`n")
$fontHolder=[IO.File]::ReadAllText("$p\Petari\src\Game\System\GameSystemFontHolder.cpp")
$fontHolder="#include <Native/HeapPlacement.hpp>`n"+$fontHolder.Replace('mMenuFont(), mNumberFont()', 'mMenuFont(), mNumberFont(), mCinemaFont()')
[IO.File]::WriteAllText("$n\src\generated\GameSystemFontHolder.cpp",$fontHolder)
$fontFileStart=$fontHolder.IndexOf('void GameSystemFontHolder::createFontFromFile()')
if($fontFileStart -lt 0){throw 'Game font startup anchor missing'}
$nativeFontLoad="void GameSystemFontHolder::createFontFromFile() { nativeLoadGameFonts(*this); }`n"
$fontHolder="#include <Native/GameFonts.hpp>`n"+$fontHolder.Substring(0,$fontFileStart)+$nativeFontLoad
[IO.File]::WriteAllText("$n\src\generated\GameSystemFontHolder.cpp",$fontHolder)
$fontCtorStart=$fontHolder.IndexOf('GameSystemFontHolder::GameSystemFontHolder()')
$fontCtorEnd=$fontHolder.IndexOf('void GameSystemFontHolder::createFontFromEmbeddedData()', $fontCtorStart)
[IO.File]::WriteAllText("$n\src\generated\GameFontHolderKernel.cpp","#include <Native/GameFonts.hpp>`n#include <Game/System/GameSystemFontHolder.hpp>`n#include <nw4r/ut/ResFont.h>`n"+$fontHolder.Substring($fontCtorStart,$fontCtorEnd-$fontCtorStart)+$nativeFontLoad)
$embeddedStart=$fontHolder.IndexOf('void GameSystemFontHolder::createFontFromEmbeddedData()')
$embeddedEnd=$fontHolder.IndexOf('namespace {', $embeddedStart)
if($embeddedStart -lt 0 -or $embeddedEnd -le $embeddedStart){throw 'Embedded font method anchors missing'}
$embeddedLoad="void GameSystemFontHolder::createFontFromEmbeddedData() { nativeLoadEmbeddedGameFonts(*this); }`n"
$fontHolder=$fontHolder.Remove($embeddedStart,$embeddedEnd-$embeddedStart).Insert($embeddedStart,$embeddedLoad)
[IO.File]::WriteAllText("$n\src\generated\GameSystemFontHolder.cpp",$fontHolder)
[IO.File]::AppendAllText("$n\src\generated\GameFontHolderKernel.cpp",$embeddedLoad)
$messageOriginal=[IO.File]::ReadAllText("$p\Petari\src\Game\System\MessageHolder.cpp")
$messageGetStart=$messageOriginal.IndexOf('bool MessageData::getMessage(TalkMessageInfo*')
$messageGetEnd=$messageOriginal.IndexOf('TalkNode* MessageData::findNode(', $messageGetStart)
$messageInfoStart=$messageOriginal.IndexOf('u8* MessageData::getMessageInfoTool(')
$messageInfoEnd=$messageOriginal.IndexOf('MessageHolder::MessageHolder()', $messageInfoStart)
[IO.File]::WriteAllText("$n\src\generated\MessageRecordKernel.cpp","#include <Game/System/MessageHolder.hpp>`n#include <Game/NPC/TalkMessageInfo.hpp>`n"+$messageOriginal.Substring($messageGetStart,$messageGetEnd-$messageGetStart)+$messageOriginal.Substring($messageInfoStart,$messageInfoEnd-$messageInfoStart))
$messageHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\System\MessageHolder.hpp").Replace('    MessageData(const char*);', "    MessageData(const char*);`n    MessageData():mIDTable(nullptr),mInfoBlock(nullptr),mDataBlock(nullptr),_C(0),mFlowBlock(nullptr),_14(nullptr),_18(nullptr),mFLI1Block(nullptr){}")
[IO.File]::WriteAllText("$n\include-runtime\Game\System\MessageHolder.hpp",$messageHeader)
$nodeStart=$messageOriginal.IndexOf('TalkNode* MessageData::getNode(')
$nodeEnd=$messageOriginal.IndexOf('u8* MessageData::getMessageInfoTool(', $nodeStart)
[IO.File]::AppendAllText("$n\src\generated\MessageRecordKernel.cpp","`n#include <Game/NPC/TalkNodeCtrl.hpp>`n"+$messageOriginal.Substring($nodeStart,$nodeEnd-$nodeStart))
$directStart=$messageOriginal.IndexOf('bool MessageData::getMessageDirect(')
$directEnd=$messageOriginal.IndexOf('bool MessageData::getMessage(TalkMessageInfo*', $directStart)
$lookupStart=$messageOriginal.IndexOf('inline JMapInfoIter end(')
$lookupEnd=$messageOriginal.IndexOf('void MessageHolder::initSystemData()', $lookupStart)
[IO.File]::AppendAllText("$n\src\generated\MessageRecordKernel.cpp","`n#include <Game/Util/JMapInfo.hpp>`n"+$messageOriginal.Substring($directStart,$directEnd-$directStart)+$messageOriginal.Substring($lookupStart,$lookupEnd-$lookupStart))
$messageInit=$messageOriginal.IndexOf('void MessageHolder::initSystemData()')
$messageNative="#include <Native/GameMessages.hpp>`n"+$messageOriginal.Substring(0,$messageInit)+"void MessageHolder::initSystemData(){nativeInitGameMessages(*this,true);}`nvoid MessageHolder::initGameData(){nativeInitGameMessages(*this,false);}`n"
[IO.File]::WriteAllText("$n\src\generated\MessageHolder.cpp",$messageNative)
$holderStart=$messageOriginal.IndexOf('MessageHolder::MessageHolder()')
$holderEnd=$messageOriginal.IndexOf('bool MessageSystem::getSystemMessageDirect(', $holderStart)
[IO.File]::WriteAllText("$n\src\generated\MessageHolderLifecycleKernel.cpp","#include <Game/System/MessageHolder.hpp>`n#include <Native/GameMessages.hpp>`n"+$messageOriginal.Substring($holderStart,$holderEnd-$holderStart)+"void MessageHolder::initSystemData(){nativeInitGameMessages(*this,true);}`nvoid MessageHolder::initGameData(){nativeInitGameMessages(*this,false);}`n")
# Native scene service retains the original holder's scene alias, rather than Wii GameSystem pointers.
foreach($messageFile in @('MessageHolder.cpp','MessageHolderLifecycleKernel.cpp')){
 $path="$n\src\generated\$messageFile"
 $code=[IO.File]::ReadAllText($path).Replace('mSceneMessageData = mGameMessageData;', 'mSceneMessageData = mGameMessageData; nativeBindSceneMessages(*this);').Replace('mSceneMessageData = nullptr;', 'nativeUnbindSceneMessages(*this); mSceneMessageData = nullptr;')
 $code=$code.Replace('return MR::getGameSystemObjHolder()->mMessageHolder->mSceneMessageData;', 'return nativeSceneMessages();')
 if($messageFile -eq 'MessageHolderLifecycleKernel.cpp'){$code+="`nMessageData* MessageSystem::getSceneMessageData(){return nativeSceneMessages();}`n"}
 [IO.File]::WriteAllText($path,$code)
}
$talkOriginal=[IO.File]::ReadAllText("$p\Petari\src\Game\NPC\TalkNodeCtrl.cpp")
$ctorStart=$talkOriginal.IndexOf('TalkNodeCtrl::TalkNodeCtrl()')
$ctorEnd=$talkOriginal.IndexOf('// Inexplicable', $ctorStart)
$navStart=$talkOriginal.IndexOf('bool TalkNodeCtrl::isExistNextNode()')
$navEnd=$talkOriginal.IndexOf('void TalkNodeCtrl::updateMessage()', $navStart)
[IO.File]::WriteAllText("$n\src\generated\TalkNodeNavigationKernel.cpp", "#include <Game/NPC/TalkNodeCtrl.hpp>`n#include <Game/System/MessageHolder.hpp>`n"+$talkOriginal.Substring($ctorStart,$ctorEnd-$ctorStart)+$talkOriginal.Substring($navStart,$navEnd-$navStart))
$findStart=$messageOriginal.IndexOf('TalkNode* MessageData::findNode(')
$findEnd=$messageOriginal.IndexOf('TalkNode* MessageData::getNode(', $findStart)
[IO.File]::AppendAllText("$n\src\generated\MessageRecordKernel.cpp",$messageOriginal.Substring($findStart,$findEnd-$findStart))
# Original layout sources with host C-library includes and an explicit raw-TPL boundary.
foreach($unit in @('lyt_arcResourceAccessor','lyt_material','lyt_texMap','lyt_textBox')){
 $code=[IO.File]::ReadAllText("$p\Petari\src\nw4r\lyt\$unit.cpp")
 if($unit -eq 'lyt_arcResourceAccessor'){$code=$code.Replace('#include <extras.h>','#include <cstring>').Replace('stricmp(', '_stricmp(').Replace('#include <cstring>', '#include <cstring>' + [Environment]::NewLine + '#include '+[char]34+'../native_arc_port.hpp'+[char]34).Replace('ArcResourceAccessor::~ArcResourceAccessor() {', 'ArcResourceAccessor::~ArcResourceAccessor() { nativeDetachArc(&mArcHandle);')}
 if($unit -eq 'lyt_textBox'){$code=$code.Replace('#include <wstring.h>','#include <wchar.h>')}
 if($unit -eq 'lyt_material'){$code="#ifndef ROUND_UP`n#define ROUND_UP(value, alignment) (((value) + (alignment) - 1) & ~((alignment) - 1))`n#endif`n#ifndef TRUE`n#define TRUE 1`n#endif`n"+$code}
 if($unit -eq 'lyt_texMap'){
  $code="#include <stdexcept>`n#include <Native/TplResources.hpp>`n"+$code
  $start=$code.IndexOf('        void TexMap::ReplaceImage(TPLPalette* p, u32 id)')
  $end=$code.IndexOf('    };  // namespace lyt', $start)
  if($start -lt 0 -or $end -lt 0){throw 'Missing TPL boundary'}
  $tplImplementation=@'
        void TexMap::ReplaceImage(TPLPalette* p, u32 id) {
            if (p == nullptr) throw std::invalid_argument("null TPL resource");
            const u8* const bytes = reinterpret_cast<const u8*>(p);
            const std::size_t size = Native::tplResourceSize(p);
            if (size < 12) throw std::invalid_argument("unregistered or truncated TPL resource");

            const auto read16 = [bytes, size](std::size_t offset) -> u16 {
                if (offset > size || size - offset < 2) throw std::invalid_argument("truncated TPL field");
                return detail::ReadBigEndianU16(bytes + offset);
            };
            const auto read32 = [bytes, size](std::size_t offset) -> u32 {
                if (offset > size || size - offset < 4) throw std::invalid_argument("truncated TPL field");
                return detail::ReadBigEndianU32(bytes + offset);
            };
            const auto extent = [size](std::size_t offset, std::size_t length) {
                if (offset > size || length > size - offset) throw std::invalid_argument("TPL resource extent is out of bounds");
            };

            if (read32(0) != 0x0020AF30) throw std::invalid_argument("invalid TPL resource signature");
            const u32 descriptorCount = read32(4);
            const u32 descriptorTable = read32(8);
            if (descriptorCount == 0 || descriptorCount > 4096) throw std::invalid_argument("invalid TPL descriptor count");
            extent(descriptorTable, std::size_t(descriptorCount) * 8);
            id %= descriptorCount;

            const std::size_t descriptor = descriptorTable + std::size_t(id) * 8;
            const u32 headerOffset = read32(descriptor);
            const u32 paletteOffset = read32(descriptor + 4);
            extent(headerOffset, 36);

            const u16 height = read16(headerOffset);
            const u16 width = read16(headerOffset + 2);
            const u32 format = read32(headerOffset + 4);
            const u32 imageOffset = read32(headerOffset + 8);
            if (width == 0 || height == 0 || width > 8192 || height > 8192 ||
                std::size_t(width) * height > 16777216) {
                throw std::invalid_argument("invalid TPL texture dimensions");
            }

            std::size_t blockWidth = 4, blockHeight = 4, bytesPerBlock = 32;
            switch (format) {
            case 0: case 8: blockWidth = blockHeight = 8; break;
            case 1: case 2: case 9: blockWidth = 8; break;
            case 3: case 4: case 5: case 10: break;
            case 6: bytesPerBlock = 64; break;
            case 14: blockWidth = blockHeight = 8; break;
            default: throw std::invalid_argument("unsupported TPL texture format");
            }
            const std::size_t imageBytes = ((std::size_t(width) + blockWidth - 1) / blockWidth) *
                                           ((std::size_t(height) + blockHeight - 1) / blockHeight) * bytesPerBlock;
            extent(imageOffset, imageBytes);

            SetImage(const_cast<u8*>(bytes + imageOffset));
            SetSize(width, height);
            SetTexelFormat(GXTexFmt(format));

            if (paletteOffset != 0) {
                extent(paletteOffset, 12);
                const u16 entryCount = read16(paletteOffset);
                const u32 paletteFormat = read32(paletteOffset + 4);
                const u32 paletteDataOffset = read32(paletteOffset + 8);
                if (entryCount == 0 || entryCount > 16384 || paletteFormat > 2) {
                    throw std::invalid_argument("invalid TPL palette metadata");
                }
                extent(paletteDataOffset, std::size_t(entryCount) * 2);
                SetPalette(const_cast<u8*>(bytes + paletteDataOffset));
                SetPaletteFormat(GXTlutFmt(paletteFormat));
                SetPaletteEntryNum(entryCount);
            } else {
                SetPalette(nullptr);
                SetPaletteFormat(GXTlutFmt(0));
                SetPaletteEntryNum(0);
            }
        }
'@
  $code=$code.Substring(0,$start)+$tplImplementation+$code.Substring($end)
 }
 [IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$code)
}
$allocatorOriginal=[IO.File]::ReadAllText("$p\Petari\src\RVL_SDK\mem\mem_allocator.c")
$start=$allocatorOriginal.IndexOf('void* MEMAllocFromAllocator(')
$end=$allocatorOriginal.IndexOf('// These SDK entry points', $start)
[IO.File]::WriteAllText("$n\src\generated\AllocatorDispatchKernel.cpp", "#include <revolution/mem/allocator.h>`n"+$allocatorOriginal.Substring($start,$end-$start))
# Native layout allocator storage must satisfy host object alignment.
$memoryHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\Util\MemoryUtil.hpp")
$memoryHeader=$memoryHeader.Replace('#pragma once', "#pragma once`n#include <cstddef>").Replace('sHeap->alloc(size, 0)', 'sHeap->alloc(size, alignof(std::max_align_t))')
[IO.File]::WriteAllText("$n\include-runtime\Game\Util\MemoryUtil.hpp",$memoryHeader)
# Desktop global new is not the Wii game-heap allocator. Route original layout defaults explicitly.
$memoryPath="$n\src\generated\MemoryUtil.cpp"
$nativeMemory=[IO.File]::ReadAllText($memoryPath).Replace('return new u8[size];','return JKRHeap::alloc(size, alignof(std::max_align_t), nullptr);').Replace('delete static_cast< u8* >(pPtr);','JKRHeap::free(pPtr, nullptr);').Replace('delete[] static_cast< u8* >(pPtr);','JKRHeap::free(pPtr, nullptr);')
[IO.File]::WriteAllText($memoryPath,$nativeMemory)
$allocatorStart=$nativeMemory.IndexOf('    void* NewDeleteAllocator::alloc(')
$allocatorEnd=$nativeMemory.IndexOf('    MEMAllocator* getHomeButtonLayoutAllocator()', $allocatorStart)
[IO.File]::WriteAllText("$n\src\generated\LayoutDefaultAllocatorKernel.cpp","#include <Game/Util/MemoryUtil.hpp>`nnamespace MR {`n"+$nativeMemory.Substring($allocatorStart,$allocatorEnd-$allocatorStart)+"`n}`n")
$trig=[IO.File]::ReadAllText("$p\Petari\src\nw4r\math\math_triangular.cpp")
$trig=$trig.Replace('f32 abs_fidx = FAbs(fidx);', 'if(!std::isfinite(fidx)) throw std::domain_error("Non-finite NW4R angle"); f32 abs_fidx = std::fmod(FAbs(fidx),65536.0f);')
[IO.File]::WriteAllText("$n\src\generated\math_triangular.cpp", "#include <cmath>`n#include <stdexcept>`n"+$trig)
$quadOriginal=[IO.File]::ReadAllText("$p\Petari\src\nw4r\lyt\lyt_common.cpp")
$quadStart=$quadOriginal.IndexOf('            void DrawQuad(const math::VEC2& basePt, const Size& size, u8 num, const math::VEC2 (*texCoords)[4], const ut::Color* vtxColors)')
$quadEnd=$quadOriginal.IndexOf('            void DrawQuad(', $quadStart+20)
if($quadStart -lt 0 -or $quadEnd -lt 0){throw 'Missing original quad source'}
[IO.File]::WriteAllText("$n\src\generated\LayoutQuadKernel.cpp", "#include <nw4r/lyt/common.h>`n#include <nw4r/math/types.h>`n#include <nw4r/ut/Color.h>`nnamespace nw4r::lyt::detail {`n"+$quadOriginal.Substring($quadStart,$quadEnd-$quadStart)+"`n}`n")
# Native argument ownership replaces pointer overlays in original dialogue tags.
$tagHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\Screen\CustomTagProcessor.hpp")
$tagHeader=$tagHeader.Replace('#pragma once',"#pragma once`n#include <map>`n#include <string>")
$tagHeader=$tagHeader.Replace('    /* 0x04 */ bool mIsShadow;', "    std::map<const wchar_t*,s32> mNativeNumbers;`n    std::map<const wchar_t*,std::wstring> mNativeStrings;`n    void clearNativeArguments(){mNativeNumbers.clear();mNativeStrings.clear();}`n    /* 0x04 */ bool mIsShadow;")
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Screen" | Out-Null
[IO.File]::WriteAllText("$n\include-runtime\Game\Screen\CustomTagProcessor.hpp",$tagHeader)
$tagSource=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\CustomTagProcessor.cpp")
$tagSource=$tagSource.Replace('*reinterpret_cast< s32* >(tag.getParamPtr(0)) = number;', 'mNativeNumbers.insert_or_assign(tag.mMessage,number);')
$tagSource=$tagSource.Replace('*reinterpret_cast< const wchar_t** >(tag.getParamPtr(0)) = string;', 'if(string)mNativeStrings.insert_or_assign(tag.mMessage,std::wstring(string));else mNativeStrings.erase(tag.mMessage);')
$tagSource=$tagSource.Replace('s32 number = *reinterpret_cast< s32* >(tag.getParamPtr(0));', 'auto argument=mNativeNumbers.find(tag.mMessage); s32 number=argument==mNativeNumbers.end()?static_cast<s32>(tag.getParam32(0)):argument->second;')
$tagSource=$tagSource.Replace('if (!*reinterpret_cast< const u8* >(tag.getParamPtr(0))) {', 'auto argument=mNativeStrings.find(tag.mMessage); if(argument==mNativeStrings.end()) {')
$tagSource=$tagSource.Replace('writeString(rect, *reinterpret_cast< const wchar_t** >(tag.getParamPtr(0)), context);', 'writeString(rect, argument->second.c_str(), context);')
[IO.File]::WriteAllText("$n\src\generated\CustomTagProcessor.cpp",$tagSource)
$tagSource=$tagSource.Replace('swprintf(string, 256,','swprintf(string, sizeof(string)/sizeof(string[0]),'); [IO.File]::WriteAllText("$n\src\generated\CustomTagProcessor.cpp",$tagSource)
# Reuse original wide print layout/control flow for the missing narrow specialization.
$originalTag=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\CustomTagProcessor.cpp")
$printStart=$originalTag.IndexOf('        template <>')
$printEnd=$originalTag.IndexOf('    }  // namespace ut',$printStart)
if($printStart -lt 0 -or $printEnd -lt 0){throw 'Original wide PrintImpl not found'}
$narrowPrint=$originalTag.Substring($printStart,$printEnd-$printStart).Replace('TextWriterBase< wchar_t >::PrintImpl','TextWriterBase< char >::PrintImpl')
$narrowPrint=[regex]::Replace($narrowPrint,'(?s)            CustomTagProcessor\* processor = nullptr;.*?            for \(CharCode','            for (CharCode')
$narrowPrint=[regex]::Replace($narrowPrint,'(?s)\s*if \(processor\) \{[^{}]*\}', '')
if($narrowPrint.Contains('processor') -or $narrowPrint.Contains('CustomTagProcessor')){throw 'Unexpected custom processor dependency in narrow PrintImpl'}
[IO.File]::WriteAllText("$n\src\generated\NarrowTextWriter.cpp","#include <nw4r/ut/TextWriterBase.h>`nnamespace nw4r::ut {`n"+$narrowPrint+"`n}`n")
$glyphSource=[IO.File]::ReadAllText("$p\Petari\src\nw4r\ut\ut_CharWriter.cpp")
$glyphStart=$glyphSource.IndexOf('        void CharWriter::PrintGlyph(')
$glyphEnd=$glyphSource.IndexOf('        void CharWriter::UpdateVertexColor()', $glyphStart)
[IO.File]::WriteAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","#include <nw4r/ut/CharWriter.h>`n#include <revolution/gx/GXVert.h>`nnamespace nw4r::ut {`n"+$glyphSource.Substring($glyphStart,$glyphEnd-$glyphStart)+"`n}`n")
$loadGlyphStart=$glyphSource.IndexOf('        void CharWriter::LoadTexture(')
$loadGlyphEnd=$glyphSource.IndexOf('        CharWriter::~CharWriter()', $loadGlyphStart)
[IO.File]::AppendAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","`nnamespace nw4r::ut {`n"+$glyphSource.Substring($loadGlyphStart,$loadGlyphEnd-$loadGlyphStart)+"`n}`n")
$fontCacheHook='void CharWriter::LoadTexture(const Glyph& glyph, GXTexMapID slot) {'
$fontCacheReplacement=$fontCacheHook+"`n            static std::uint64_t generation=0; auto current=Native::gpuResources().generation(); if(current!=generation){mLoadingTexture.Reset();generation=current;}"
$adaptedCharWriter=$glyphSource.Replace($fontCacheHook,$fontCacheReplacement)
[IO.File]::WriteAllText("$n\src\generated\ut_CharWriter.cpp","#include <Native/GpuResources.hpp>`n"+$adaptedCharWriter)
$glyphKernel=[IO.File]::ReadAllText("$n\src\generated\FontGlyphEmissionKernel.cpp")
[IO.File]::WriteAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","#include <Native/GpuResources.hpp>`n"+$glyphKernel.Replace($fontCacheHook,$fontCacheReplacement))
$charRanges=@(
 @('        CharWriter::CharWriter()', '        void CharWriter::LoadTexture('),
 @('        CharWriter::~CharWriter()', '        void CharWriter::SetupVertexFormat()'),
 @('        void CharWriter::EnableLinearFilter(', '        void CharWriter::PrintGlyph('),
 @('        void CharWriter::UpdateVertexColor()', '        void CharWriter::SetupGXWithColorMapping(')
)
$charMethods=''
foreach($range in $charRanges){$start=$glyphSource.IndexOf($range[0]);$end=$glyphSource.IndexOf($range[1],$start);if($start -lt 0 -or $end -lt 0){throw 'Missing CharWriter lifecycle method'};$charMethods+=$glyphSource.Substring($start,$end-$start)}
[IO.File]::AppendAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","`n#include <nw4r/ut/Font.h>`nnamespace nw4r::ut {`n"+$charMethods+"`n}`n")
$colorHeader=[IO.File]::ReadAllText("$p\Petari\libs\nw4r\include\nw4r\ut\Color.h")
$colorHeader=$colorHeader.Replace('ToU32ref() = color;', 'r=u8(color>>24);g=u8(color>>16);b=u8(color>>8);a=u8(color);').Replace('return operator=(*reinterpret_cast< const u32* >(&color));','r=color.r;g=color.g;b=color.b;a=color.a;return *this;')
$colorHeader=$colorHeader.Replace('u32& ToU32ref()', 'u32 ToU32ref()').Replace('const u32& ToU32ref() const','u32 ToU32ref() const').Replace('return *reinterpret_cast< u32* >(this);','return (u32(r)<<24)|(u32(g)<<16)|(u32(b)<<8)|a;').Replace('return *reinterpret_cast< const u32* >(this);','return (u32(r)<<24)|(u32(g)<<16)|(u32(b)<<8)|a;')
New-Item -ItemType Directory -Force "$n\include-runtime\nw4r\ut" | Out-Null
[IO.File]::WriteAllText("$n\include-runtime\nw4r\ut\Color.h",$colorHeader)
[IO.File]::WriteAllText("$n\include-runtime\nw4r\ut\CharWriter.h",[IO.File]::ReadAllText("$p\Petari\libs\nw4r\include\nw4r\ut\CharWriter.h"))
$metricsStart=$glyphSource.IndexOf('        void CharWriter::SetFontSize(')
$metricsEnd=$glyphSource.IndexOf('        void CharWriter::EnableLinearFilter(', $metricsStart)
[IO.File]::AppendAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","`nnamespace nw4r::ut {`n"+$glyphSource.Substring($metricsStart,$metricsEnd-$metricsStart)+"`n}`n")
foreach($pair in @(@('ut_TextWriterBase','NarrowTextWriterBaseTest','TextWriterBase'),@('ut_TagProcessorBase','NarrowTagProcessorTest','TagProcessorBase'))){
$source=[IO.File]::ReadAllText("$p\Petari\src\nw4r\ut\$($pair[0]).cpp").Replace("        template class $($pair[2])< wchar_t >;",'')
[IO.File]::WriteAllText("$n\src\generated\$($pair[1]).cpp",$source)
}
$widePrint=$originalTag.Substring($printStart,$printEnd-$printStart)
$alphaStart=$originalTag.IndexOf('u8 CustomTagAlphaCtrl::alpha() const')
$alphaEnd=$originalTag.IndexOf('void CustomTagAlphaCtrl::update()', $alphaStart)
$resetStart=$originalTag.IndexOf('void CustomTagProcessor::reset(')
$resetEnd=$originalTag.IndexOf('MessageEditorMessageTag CustomTagProcessor::getReplaceTag(', $resetStart)
$widePrefix="#include <Game/Screen/CustomTagProcessor.hpp>`n#include <nw4r/ut/TextWriterBase.h>`n#include <nw4r/lyt/textBox.h>`n#include <algorithm>`n"
[IO.File]::WriteAllText("$n\src\generated\WideTextWriterKernel.cpp",$widePrefix+"namespace nw4r::ut {`n"+$widePrint+"`n}`n"+$originalTag.Substring($alphaStart,$alphaEnd-$alphaStart)+$originalTag.Substring($resetStart,$resetEnd-$resetStart))
# Numeric message-tag access resolves only validated owned host payloads.
$tagHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\Screen\MessageTagSkipTagProcessor.hpp")
$tagHeader=$tagHeader.Replace('#pragma once',"#pragma once`n#include `"../../../src/native_message_tag_access.hpp`"")
$tagHeader=$tagHeader.Replace('return reinterpret_cast< const u8* >(mMessage)[1];','return nativeMessageTag(mMessage).group();').Replace('return mMessage[1];','return nativeMessageTag(mMessage).id();').Replace('return reinterpret_cast< const u8* >(mMessage)[1] == group && mMessage[1] == tag;','return getGroup() == group && getTag() == tag;')
[IO.File]::WriteAllText("$n\include-runtime\Game\Screen\MessageTagSkipTagProcessor.hpp",$tagHeader)
$tagSource=[IO.File]::ReadAllText("$p\Petari\src\Game\Screen\MessageTagSkipTagProcessor.cpp")
$tagSource=$tagSource.Replace('return reinterpret_cast< const u8* >(mMessage)[0] - 2;','return nativeMessageTag(mMessage).tag.size();').Replace('return reinterpret_cast< const u8* >(mMessage)[0] - 6;','return nativeMessageTag(mMessage).parameterBytes();')
$tagSource=$tagSource.Replace('return pParam[4];','return nativeMessageTag(mMessage).param8(static_cast<size_t>(index));').Replace('return pParam[2];','return nativeMessageTag(mMessage).param16(static_cast<size_t>(index));').Replace('return pParam[1];','return nativeMessageTag(mMessage).param32(static_cast<size_t>(index));')
$tagSource=$tagSource.Replace('return const_cast< wchar_t* >(reinterpret_cast< const wchar_t* >(pParam));','nativeMessageTag(mMessage); throw std::runtime_error("Typed tag string/pointer conversion is not implemented");')
[IO.File]::WriteAllText("$n\src\generated\MessageTagSkipTagProcessor.cpp",$tagSource)
$tagEnd=$tagSource.IndexOf('MessageTagSkipTagProcessor::MessageTagSkipTagProcessor()')
[IO.File]::WriteAllText("$n\src\generated\MessageTagAccessKernel.cpp",$tagSource.Substring(0,$tagEnd))
# Convert explicitly identified inline UTF16 fields; never reinterpret argument slots as strings.
$customSource=[IO.File]::ReadAllText("$n\src\generated\CustomTagProcessor.cpp")
$customSource=$customSource.Replace('tag.getParamPtr(0), static_cast< s32 >(tag.getParamLength()) / 2 + 1','nativeMessageTagText(tag.mMessage,0,tag.getParamLength()/2), static_cast< s32 >(tag.getParamLength()) / 2 + 1')
$customSource=$customSource.Replace('const wchar_t* rubyText = tag.getParamPtr(1);','const wchar_t* rubyText = nativeMessageTagText(tag.mMessage,1,(tag.getParamLength()-2)/2);')
$customSource=$customSource.Replace('const wchar_t* string = tag.getParamPtr(0);','const wchar_t* string = nativeMessageTagText(tag.mMessage,0,tag.getParamLength()/2);')
[IO.File]::WriteAllText("$n\src\generated\CustomTagProcessor.cpp",$customSource)
$customSource=[IO.File]::ReadAllText("$n\src\generated\CustomTagProcessor.cpp")
$customSource=$customSource.Replace('s32 baseLength = tag.getParam8(0);','assetCheck(tag.getParamLength() >= 2 && (tag.getParamLength()-2)/2 < 32, "Ruby text exceeds original local buffer");'+"`n    "+'s32 baseLength = tag.getParam8(0);'+"`n    "+'assetCheck(baseLength < 32, "Ruby base exceeds original local buffer");')
[IO.File]::WriteAllText("$n\src\generated\CustomTagProcessor.cpp",$customSource)
$customSource=[IO.File]::ReadAllText("$n\src\generated\CustomTagProcessor.cpp")
$customSource=$customSource.Replace('MR::copyMemory(base, context->str + tag.getTagLength() / 2, baseLength * sizeof(wchar_t));','MR::copyMemory(base, nativeMessageTagBase(tag.mMessage,baseLength), baseLength * sizeof(wchar_t));')
[IO.File]::WriteAllText("$n\src\generated\CustomTagProcessor.cpp",$customSource)
foreach($unit in @('MessageRecordKernel','MessageHolder')){
$source=[IO.File]::ReadAllText("$n\src\generated\$unit.cpp")
$source="#include `"../native_message_text_binding.hpp`"`n"+$source
$source=$source.Replace('pMessageInfo->_0 = reinterpret_cast< u8* >(mDataBlock + 1) + *reinterpret_cast< u32* >(pInfoTool);','pMessageInfo->_0 = nativeMessageText(*this,infoToolIndex);')
$source=$source.Replace('u8* pInfoTool = getMessageInfoTool(infoToolIndex);','if (!pMessageInfo || !mInfoBlock || infoToolIndex >= mInfoBlock->mItemCount) return false;'+"`n    "+'u8* pInfoTool = getMessageInfoTool(infoToolIndex);')
[IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$source)
}
# Native virtual query replaces the original unchecked custom-processor cast.
$baseHeader=[IO.File]::ReadAllText("$p\Petari\libs\nw4r\include\nw4r\ut\TagProcessorBase.h")
$baseHeader=$baseHeader.Replace('virtual ~TagProcessorBase();','virtual ~TagProcessorBase();'+"`n            "+'virtual void* NativeCustomProcessor() { return nullptr; }')
[IO.File]::WriteAllText("$n\include-runtime\nw4r\ut\TagProcessorBase.h",$baseHeader)
$customHeader=[IO.File]::ReadAllText("$n\include-runtime\Game\Screen\CustomTagProcessor.hpp")
$customHeader=$customHeader.Replace('class CustomTagProcessor : public MessageTagSkipTagProcessor {'+"`n"+'public:', 'class CustomTagProcessor : public MessageTagSkipTagProcessor {'+"`n"+'public:'+"`n    "+'void* NativeCustomProcessor() override { return this; }')
$customHeader=$customHeader.Replace('class CustomTagProcessor : public MessageTagSkipTagProcessor {'+"`r`n"+'public:', 'class CustomTagProcessor : public MessageTagSkipTagProcessor {'+"`r`n"+'public:'+"`r`n    "+'void* NativeCustomProcessor() override { return this; }')
[IO.File]::WriteAllText("$n\include-runtime\Game\Screen\CustomTagProcessor.hpp",$customHeader)
foreach($unit in @('CustomTagProcessor','WideTextWriterKernel')){
$source=[IO.File]::ReadAllText("$n\src\generated\$unit.cpp")
$source=$source.Replace('processor = static_cast< CustomTagProcessor* >(mTagProcessor);'+"`n                "+'processor->reset(str);','processor = static_cast< CustomTagProcessor* >(mTagProcessor->NativeCustomProcessor());'+"`n                "+'if(processor) processor->reset(str);')
$source=$source.Replace('processor = static_cast< CustomTagProcessor* >(mTagProcessor);'+"`r`n                "+'processor->reset(str);','processor = static_cast< CustomTagProcessor* >(mTagProcessor->NativeCustomProcessor());'+"`r`n                "+'if(processor) processor->reset(str);')
[IO.File]::WriteAllText("$n\src\generated\$unit.cpp",$source)
}
& "$n\generate-vertex-format.ps1"
$originalChar=[IO.File]::ReadAllText("$p\Petari\src\nw4r\ut\ut_CharWriter.cpp")
$vertexStart=$originalChar.IndexOf('        void CharWriter::SetupVertexFormat()');$vertexEnd=$originalChar.IndexOf('        void CharWriter::SetupGX()', $vertexStart)
[IO.File]::AppendAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","`nnamespace nw4r::ut {`n"+$originalChar.Substring($vertexStart,$vertexEnd-$vertexStart)+"`n}`n")
$commonStart=$originalChar.IndexOf('namespace {');$commonEnd=$originalChar.IndexOf('namespace nw4r {',$commonStart)
$setupStart=$originalChar.IndexOf('        void CharWriter::SetupGX()');$setupEnd=$originalChar.IndexOf('        void CharWriter::SetFontSize(', $setupStart)
$mappingStart=$originalChar.IndexOf('        void CharWriter::SetupGXWithColorMapping(');$mappingEnd=$originalChar.IndexOf('    };  // namespace ut',$mappingStart)
[IO.File]::AppendAllText("$n\src\generated\FontGlyphEmissionKernel.cpp","`n"+$originalChar.Substring($commonStart,$commonEnd-$commonStart)+"`nnamespace nw4r::ut {`n"+$originalChar.Substring($setupStart,$setupEnd-$setupStart)+$originalChar.Substring($mappingStart,$mappingEnd-$mappingStart)+"`n}`n")
$colorSource=[IO.File]::ReadAllText("$n\src\generated\J3DGD.cpp")
$colorStart=$colorSource.IndexOf('void J3DGDSetTevKColor(');$colorEnd=$colorSource.IndexOf('void J3DGDSetFog(', $colorStart)
[IO.File]::WriteAllText("$n\src\generated\TevColorKernel.cpp","#include <JSystem/J3DGraphBase/J3DGD.hpp>`n#include <revolution/gd/GDTev.h>`n"+$colorSource.Substring($colorStart,$colorEnd-$colorStart))


$layout=[IO.File]::ReadAllText("$p\Petari\src\Game\System\LayoutHolder.cpp")
$layout=$layout.Replace('*pResourceInfo = static_cast< u32* >(pResource)[1];','const auto* bytes = static_cast<const unsigned char*>(pResource); *pResourceInfo = (u32(bytes[4]) << 24) | (u32(bytes[5]) << 16) | (u32(bytes[6]) << 8) | u32(bytes[7]);')
$layout=$layout.Replace('sprintf(path, "%s%s%s", pRoot, "/", pFinder->mName);','int length = snprintf(path, sizeof(path), "%s/%s", pRoot ? pRoot : "", pFinder->mName); if (length < 0 || size_t(length) >= sizeof(path)) { delete pFinder; throw std::runtime_error("Layout resource path exceeds limit"); }')
$layout=$layout.Replace('snprintf(path, 128, "%s/%s", pRoot, pFinder->mName);','int length = snprintf(path, sizeof(path), "%s/%s", pRoot ? pRoot : "", pFinder->mName); if (length < 0 || size_t(length) >= sizeof(path)) { delete pFinder; throw std::runtime_error("Layout resource path exceeds limit"); }')
$layout=$layout.Replace('#include <cstring>','#include <cstring>' + [Environment]::NewLine + '#include <stdexcept>' + [Environment]::NewLine + '#include "Native/TplResources.hpp"')
$layout=$layout.Replace('LayoutHolder::~LayoutHolder() {','LayoutHolder::~LayoutHolder() {' + [Environment]::NewLine + '    for (u32 i = 0; i < mResOther.mCount; ++i) {' + [Environment]::NewLine + '        const ResFileInfo& info = mResOther.mFileInfoTable[i];' + [Environment]::NewLine + '        if (info.mName != nullptr && strstr(info.mName, ".tpl") != nullptr) Native::unregisterTplResource(info._8);' + [Environment]::NewLine + '    }')
$layout=$layout.Replace('            pInfo->_C = pFinder->mFileID;','            pInfo->_C = pFinder->mFileID;' + [Environment]::NewLine + '            if (strstr(pFinder->mName, ".tpl") != nullptr) Native::registerTplResource(pInfo->_8, pInfo->_4);')
[IO.File]::WriteAllText("$n\src\generated\LayoutHolder.cpp",$layout)

$stringExtra='#include <cwchar>'+[Environment]::NewLine+'namespace MR {'+[Environment]::NewLine
foreach($signature in @('int strcasecmp(', 'bool isEqualStringCase(', 'void copyString(char*', 'void copyString(wchar_t*')){
$b=$stringOriginal.IndexOf('    '+$signature);if($b -lt 0){throw "String function anchor $signature"};$o=$stringOriginal.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($stringOriginal[$e] -eq '{'){$d++};if($stringOriginal[$e] -eq '}'){$d--};$e++}
$stringExtra+=$stringOriginal.Substring($b,$e-$b).Replace('::strcasecmp(', '::_stricmp(').Replace('u32 num','unsigned int num')+[Environment]::NewLine
}
$stringExtra+='}'+[Environment]::NewLine
[IO.File]::AppendAllText("$n\src\generated\StringKernel.cpp",$stringExtra)
















& (Join-Path $n 'generate-npc-item-table.ps1')
& (Join-Path $n 'generate-bas-resource.ps1')

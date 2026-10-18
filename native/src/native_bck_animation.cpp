#include <Native/BckAnimation.hpp>
#include <algorithm>
#include <array>
#include "native_bck_tables.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>
namespace {
NativeBckTables decode(const void* data,std::size_t size){
    const auto* bytes=static_cast<const uint8_t*>(data);
    std::vector<uint8_t> copy(bytes,bytes+size);
    return NativeBckTables(BinaryView(copy));
}
class OwnedBckAnimation final : public J3DAnmTransformKey {
    NativeBckTables tables;
public:
    OwnedBckAnimation(const void* data,std::size_t size):tables(decode(data,size)){
        tables.bind(*this);
    }
};

#ifndef NATIVE_BCK_TEST_NO_BCA
struct NativeBcaTables {
    u8 loop;
    s16 duration;
    u16 jointCount;
    std::vector<J3DAnmTransformFullTable> axes;
    std::vector<f32> scales, translations;
    std::vector<s16> rotations;

    explicit NativeBcaTables(BinaryView view) {
        assetCheck(view.size() >= 32 && view.tag(0) == "J3D1" && view.tag(4) == "bca1",
                   "Unsupported or truncated BCA header");
        const u32 fileSize = view.u32(8);
        assetCheck(fileSize >= 32 && fileSize <= view.size(), "Invalid BCA file size");
        view = view.sub(0, fileSize);
        const u32 blockCount = view.u32(12);
        assetCheck(blockCount > 0 && blockCount <= 64, "Invalid BCA block count");

        BinaryView transformBlock = view.sub(32, view.u32(36));
        assetCheck(transformBlock.tag(0) == "ANF1", "BCA is missing its ANF1 transform block");
        loop = transformBlock.u8(8);
        duration = s16(transformBlock.u16(10));
        jointCount = transformBlock.u16(12);
        assetCheck(loop <= 4 && duration > 0 && jointCount > 0 && jointCount <= 4096,
                   "Invalid BCA transform metadata");

        const size_t tableOffset = transformBlock.u32(20);
        const size_t scaleOffset = transformBlock.u32(24);
        const size_t rotationOffset = transformBlock.u32(28);
        const size_t translationOffset = transformBlock.u32(32);
        const size_t axisCount = size_t(jointCount) * 3;
        extent(tableOffset, axisCount * sizeof(J3DAnmTransformFullTable), transformBlock.size());
        axes.resize(axisCount);

        size_t scaleCount = 0, rotationCount = 0, translationCount = 0;
        for (size_t axis = 0; axis < axisCount; ++axis) {
            const size_t entry = tableOffset + axis * sizeof(J3DAnmTransformFullTable);
            auto& table = axes[axis];
            table.mScaleMaxFrame = transformBlock.u16(entry + 0);
            table.mScaleOffset = transformBlock.u16(entry + 2);
            table.mRotationMaxFrame = transformBlock.u16(entry + 4);
            table.mRotationOffset = transformBlock.u16(entry + 6);
            table.mTranslateMaxFrame = transformBlock.u16(entry + 8);
            table.mTranslateOffset = transformBlock.u16(entry + 10);
            assetCheck(table.mScaleMaxFrame && table.mRotationMaxFrame && table.mTranslateMaxFrame,
                       "BCA transform channel has no samples");
            scaleCount = std::max(scaleCount, size_t(table.mScaleOffset) + table.mScaleMaxFrame);
            rotationCount = std::max(rotationCount, size_t(table.mRotationOffset) + table.mRotationMaxFrame);
            translationCount = std::max(translationCount, size_t(table.mTranslateOffset) + table.mTranslateMaxFrame);
        }

        extent(scaleOffset, scaleCount * sizeof(u32), transformBlock.size());
        extent(rotationOffset, rotationCount * sizeof(u16), transformBlock.size());
        extent(translationOffset, translationCount * sizeof(u32), transformBlock.size());
        scales.reserve(scaleCount);
        rotations.reserve(rotationCount);
        translations.reserve(translationCount);
        for (size_t i = 0; i < scaleCount; ++i) scales.push_back(transformBlock.f32(scaleOffset + i * 4));
        for (size_t i = 0; i < rotationCount; ++i) rotations.push_back(s16(transformBlock.u16(rotationOffset + i * 2)));
        for (size_t i = 0; i < translationCount; ++i) translations.push_back(transformBlock.f32(translationOffset + i * 4));
    }

    void bind(J3DAnmTransformFull& animation) {
        animation.mAttribute = loop;
        animation.mFrameMax = duration;
        animation.mFrame = 0.0f;
        animation.field_0x1e = jointCount;
        animation.mAnmTable = axes.data();
        animation.mScaleData = scales.data();
        animation.mRotData = rotations.data();
        animation.mTransData = translations.data();
    }
};

class OwnedBcaAnimation final : public J3DAnmTransformFull {
    NativeBcaTables tables;
public:
    OwnedBcaAnimation(const void* data, size_t size) : tables([&] {
        const auto* bytes = static_cast<const uint8_t*>(data);
        std::vector<uint8_t> copy(bytes, bytes + size);
        return NativeBcaTables(BinaryView(copy));
    }()) {
        tables.bind(*this);
    }
};
#endif

#ifndef NATIVE_BCK_TEST_NO_BPK
struct NativeBpkTables {
    u8 attribute;
    s16 duration;
    std::array<u16,4> metadata{};
    std::vector<J3DAnmColorKeyTable> materials;
    std::array<std::vector<s16>,4> channels;
    std::vector<u16> updateMaterialIds;
    std::vector<u8> nameTable;

    explicit NativeBpkTables(BinaryView view) {
        assetCheck(view.size()>=32 && view.tag(0)=="J3D1" && view.tag(4)=="bpk1",
                   "Unsupported or truncated BPK header");
        const u32 fileSize=view.u32(8);
        assetCheck(fileSize>=32 && fileSize<=view.size() && view.u32(12)==1,"Invalid BPK file header");
        BinaryView block=view.sub(32,view.u32(36));
        assetCheck(block.size()>=0x34 && block.tag(0)=="PAK1","BPK is missing its PAK1 color block");
        attribute=block.u8(8);
        duration=s16(block.u16(12));
        const u16 materialCount=block.u16(14);
        assetCheck(duration>=0 && materialCount<=4096,"Invalid BPK metadata");
        for(unsigned channel=0;channel<4;++channel)metadata[channel]=block.u16(0x10+channel*2);
        const size_t tableOffset=block.u32(0x18);
        const size_t idsOffset=block.u32(0x1c);
        const size_t namesOffset=block.u32(0x20);
        const size_t valueOffsets[4]={block.u32(0x24),block.u32(0x28),block.u32(0x2c),block.u32(0x30)};
        extent(tableOffset,size_t(materialCount)*sizeof(J3DAnmColorKeyTable),block.size());
        extent(idsOffset,size_t(materialCount)*sizeof(u16),block.size());
        materials.resize(materialCount);
        updateMaterialIds.resize(materialCount);
        for(size_t i=0;i<materialCount;++i)updateMaterialIds[i]=block.u16(idsOffset+i*2);

        if(materialCount){
            assetCheck(namesOffset<block.size(),"BPK name table offset is outside its block");
            const size_t namesSize=block.size()-namesOffset;
            extent(namesOffset,namesSize,block.size());
            assetCheck(namesSize>=4,"Truncated BPK name table");
            const u16 nameCount=block.u16(namesOffset);
            assetCheck(nameCount==materialCount,"BPK name and material counts differ");
            extent(namesOffset+4,size_t(nameCount)*4,namesOffset+namesSize);
            nameTable.resize(namesSize);
            for(size_t i=0;i<namesSize;++i)nameTable[i]=block.u8(namesOffset+i);
            nameTable[0]=u8(nameCount);nameTable[1]=u8(nameCount>>8);
            for(size_t i=0;i<nameCount;++i){
                const size_t entry=4+i*4;
                const u16 key=block.u16(namesOffset+entry),textOffset=block.u16(namesOffset+entry+2);
                nameTable[entry]=u8(key);nameTable[entry+1]=u8(key>>8);
                nameTable[entry+2]=u8(textOffset);nameTable[entry+3]=u8(textOffset>>8);
                assetCheck(textOffset<namesSize,"BPK name offset is outside its table");
                bool terminated=false;for(size_t at=textOffset;at<namesSize;++at)if(nameTable[at]==0){terminated=true;break;}
                assetCheck(terminated,"Unterminated BPK material name");
            }
        }

        J3DAnmKeyTableBase* fields[4];
        for(size_t material=0;material<materialCount;++material){
            auto& table=materials[material];
            fields[0]=&table.mRInfo;fields[1]=&table.mGInfo;fields[2]=&table.mBInfo;fields[3]=&table.mAInfo;
            for(unsigned channel=0;channel<4;++channel){
                const size_t entry=tableOffset+material*sizeof(J3DAnmColorKeyTable)+channel*sizeof(J3DAnmKeyTableBase);
                auto& info=*fields[channel];info.mMaxFrame=block.u16(entry);info.mOffset=block.u16(entry+2);info.mType=block.u16(entry+4);
                const size_t count=metadata[channel];
                extent(valueOffsets[channel],count*sizeof(s16),block.size());
                const size_t stride=info.mType?4:3;
                const size_t samples=info.mMaxFrame<=1?info.mMaxFrame:size_t(info.mMaxFrame)*stride;
                extent(info.mOffset,samples,count);
            }
        }
        for(unsigned channel=0;channel<4;++channel){
            const size_t count=metadata[channel];extent(valueOffsets[channel],count*sizeof(s16),block.size());
            channels[channel].reserve(count);
            for(size_t i=0;i<count;++i)channels[channel].push_back(s16(block.u16(valueOffsets[channel]+i*2)));
        }
    }

    void bind(J3DAnmColorKey& animation){
        animation.mAttribute=attribute;animation.mFrameMax=duration;animation.mFrame=0.0f;
        animation.mUpdateMaterialNum=u16(updateMaterialIds.size());
        animation.field_0xc=metadata[0];animation.field_0xe=metadata[1];animation.field_0x10=metadata[2];animation.field_0x12=metadata[3];
        animation.mAnmTable=materials.data();animation.mUpdateMaterialID=updateMaterialIds.data();
        animation.mColorR=channels[0].data();animation.mColorG=channels[1].data();animation.mColorB=channels[2].data();animation.mColorA=channels[3].data();
        animation.mUpdateMaterialName.setResource(nameTable.empty()?nullptr:reinterpret_cast<const ResNTAB*>(nameTable.data()));
    }
};
class OwnedBpkAnimation final : public J3DAnmColorKey {
    NativeBpkTables tables;
public:
    OwnedBpkAnimation(const void* data,size_t size):tables([&]{
        const auto* bytes=static_cast<const uint8_t*>(data);std::vector<uint8_t> copy(bytes,bytes+size);return NativeBpkTables(BinaryView(copy));
    }()){tables.bind(*this);}
};
#endif
}
J3DAnmBase* Native::createBckAnimation(const void* data,std::size_t size){
    assetCheck(data && size>=32,"Missing or truncated BCK resource");
    return new OwnedBckAnimation(data,size);
}
J3DAnmBase* Native::loadArchiveBck(const JKRArchive& archive,const void* data){
    if(!data)return nullptr;
    s32 size=archive.getResSize(data);
    assetCheck(size>=32,"BCK resource not owned by archive or truncated");
    return createBckAnimation(data,std::size_t(size));
}
#ifndef NATIVE_BCK_TEST_NO_BCA
J3DAnmBase* Native::createBcaAnimation(const void* data,std::size_t size){
    assetCheck(data && size>=32,"Missing or truncated BCA resource");
    return new OwnedBcaAnimation(data,size);
}
J3DAnmBase* Native::loadArchiveBca(const JKRArchive& archive,const void* data){
    if(!data)return nullptr;
    s32 size=archive.getResSize(data);
    assetCheck(size>=32,"BCA resource not owned by archive or truncated");
    return createBcaAnimation(data,std::size_t(size));
}
#endif
#ifndef NATIVE_BCK_TEST_NO_BPK
J3DAnmBase* Native::createBpkAnimation(const void* data,std::size_t size){
    assetCheck(data && size>=32,"Missing or truncated BPK resource");
    return new OwnedBpkAnimation(data,size);
}
J3DAnmBase* Native::loadArchiveBpk(const JKRArchive& archive,const void* data){
    if(!data)return nullptr;
    s32 size=archive.getResSize(data);
    assetCheck(size>=32,"BPK resource not owned by archive or truncated");
    return createBpkAnimation(data,std::size_t(size));
}
#endif

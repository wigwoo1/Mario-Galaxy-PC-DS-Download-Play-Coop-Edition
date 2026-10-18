#pragma once
#include "assets.hpp"
// Non-owning map: the input allocation must outlive every returned section.
struct NativeModelBlocks {
    struct Block {size_t offset,length;};
    std::vector<Block> blocks;
    size_t fileSize;
    NativeModelBlocks(const uint8_t* data,size_t available){
        assetCheck(data && available>=32,"Truncated J3D model header");
        assetCheck(!std::memcmp(data,"J3D2",4),"Expected J3D2 model");
        fileSize=be32(data+8);
        assetCheck(fileSize>=32 && fileSize<=available,"J3D file size outside allocation");
        auto count=be32(data+12);
        assetCheck(count<=(fileSize-32)/8,"J3D section count outside file");
        size_t offset=32;
        for(uint32_t index=0;index<count;++index){
            extent(offset,8,fileSize);
            size_t length=be32(data+offset+4);
            assetCheck(length>=8,"J3D section cannot advance");
            extent(offset,length,fileSize);
            blocks.push_back({offset,length});offset+=length;
        }
        // File padding may follow the final section; never parse it as a block.
    }
};

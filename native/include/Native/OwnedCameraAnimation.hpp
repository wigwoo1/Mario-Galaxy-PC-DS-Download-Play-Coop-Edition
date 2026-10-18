#pragma once
#include <Native/CameraAnimationData.hpp>
#include <Game/Camera/CameraAnim.hpp>
namespace Native {
// Stable typed storage for original accessors. Do not copy or move after binding.
class OwnedCameraAnimation {
    CameraAnimationData data;
    CanmFrameInfo frameInfo{};
    CanmKeyFrameInfo keyInfo{};
public:
    OwnedCameraAnimation()=default;
    OwnedCameraAnimation(const OwnedCameraAnimation&)=delete;
    OwnedCameraAnimation& operator=(const OwnedCameraAnimation&)=delete;
    bool load(const uint8_t* bytes,size_t size) {
        CameraAnimationData next;
        if(!next.decode(bytes,size))return false;
        return loadDecoded(std::move(next));
    }
    bool loadDecoded(CameraAnimationData&& next) {
        if(!next.frames || next.values.empty())return false;
        CanmFrameInfo fi{};CanmKeyFrameInfo ki{};
        auto* f=reinterpret_cast<uint8_t*>(&fi);
        auto* k=reinterpret_cast<uint8_t*>(&ki);
        static_assert(sizeof(CanmFrameInfo)==64 && sizeof(CanmKeyFrameInfo)==96);
        for(size_t i=0;i<8;++i){
            const auto& c=next.components[i];
            std::memcpy(f+i*8,&c.count,4);std::memcpy(f+i*8+4,&c.offset,4);
            std::memcpy(k+i*12,&c.count,4);std::memcpy(k+i*12+4,&c.offset,4);std::memcpy(k+i*12+8,&c.type,4);
        }
        data=std::move(next);frameInfo=fi;keyInfo=ki;
        return true;
    }
    // Rebind after each successful load; failed loads preserve prior bindings.
    BaseCamAnmDataAccessor* bind(CamAnmDataAccessor& frame,KeyCamAnmDataAccessor& key) {
        if(data.values.empty())return nullptr;
        if(data.keyed){key.set(&keyInfo,data.values.data());return &key;}
        frame.set(&frameInfo,data.values.data());return &frame;
    }
    bool keyed()const{return data.keyed;}
    uint32_t valueCount()const{return static_cast<uint32_t>(data.values.size());}
    uint32_t frames()const{return data.frames;}
    const std::array<uint32_t,4>& metadata()const{return data.metadata;}
};
}


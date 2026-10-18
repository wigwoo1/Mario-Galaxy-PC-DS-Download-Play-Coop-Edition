#pragma once
#include "assets.hpp"
#include <map>
// Native resource storage for the future JKR adapter. Never overlays Wii structs.
class OwnedArchiveResources {
    Rarc archive;
    std::map<std::string,std::vector<uint8_t>> resident;
public:
    explicit OwnedArchiveResources(std::vector<uint8_t> bytes):archive(std::move(bytes)){}
    const std::vector<AssetFile>& files() const{return archive.files;}
    const uint8_t* get(const std::string& path){
        auto it=resident.find(path);
        if(it==resident.end()){
            bool found=false;for(const auto& f:archive.files)if(f.path==path){found=true;break;}
            if(!found)return nullptr;
            auto bytes=archive.readFile(path);
            if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4))bytes=decodeYaz0(bytes);
            else if(bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4))bytes=decodeYay0(bytes);
            // Keep zero-size resources distinguishable from missing resources.
            if(bytes.empty())bytes.reserve(1);
            it=resident.emplace(path,std::move(bytes)).first;
        }
        return it->second.data();
    }
    size_t sizeOf(const void* pointer) const{
        for(const auto& item:resident)if(item.second.data()==pointer)return item.second.size();
        throw std::runtime_error("Pointer is not a resident archive resource");
    }
    size_t read(void* destination,size_t capacity,const std::string& path){
        const uint8_t* resource=get(path);
        if(!resource)return 0;
        size_t count=std::min(capacity,sizeOf(resource));
        assetCheck(destination || !count,"Null archive read destination");
        if(count)std::memcpy(destination,resource,count);
        return count;
    }
    OwnedArchiveResources(const OwnedArchiveResources&)=delete;
    OwnedArchiveResources& operator=(const OwnedArchiveResources&)=delete;
};


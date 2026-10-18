#pragma once
#include "owned_archive_resources.hpp"
#ifdef GetCharWidth
#undef GetCharWidth
#endif
#include <nw4r/ut/ResFont.h>
#include <Native/FontResource.hpp>
#include <memory>
class NativeArchiveFonts {
public:
    std::array<std::unique_ptr<nw4r::ut::ResFont>,5> fonts;
    explicit NativeArchiveFonts(OwnedArchiveResources& archive){
        const char* names[]={"messagefont26.brfnt","picturefont.brfnt","menufont64.brfnt","numberfont.brfnt","cinemafont26.brfnt"};
        for(size_t i=0;i<fonts.size();++i){
            const uint8_t* resource=nullptr;
            for(const auto& file:archive.files()){auto name=file.path.substr(file.path.find_last_of('/')+1);for(char& c:name)if(c>='A' && c<='Z')c=char(c-'A'+'a');if(name==names[i]){resource=archive.get(file.path);break;}}
            assetCheck(resource!=nullptr,"Missing campaign font resource");
            assetCheck(nativeRegisterFontResource(resource,archive.sizeOf(resource)),"Font registration failed");
            struct Registration { const void* pointer; ~Registration(){nativeUnregisterFontResource(pointer);} } registration{resource};
            auto font=std::make_unique<nw4r::ut::ResFont>();
            const bool loaded=font->SetResource(const_cast<uint8_t*>(resource));
            assetCheck(loaded,"Invalid campaign font resource");font->SetAlternateChar('?');fonts[i]=std::move(font);
        }
    }
};




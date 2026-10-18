#pragma once
#include "native_disc_archive_loader.hpp"
#include "native_archive_fonts.hpp"
#include <Native/GameFonts.hpp>
#include <Game/System/GameSystemFontHolder.hpp>
#include <Game/System/ErrorArchive.hpp>
// Holders must outlive this backend, or be explicitly released before destruction.
// All load/release/bind operations require quiescent font consumers.
class NativeDiscFontBackend final:public NativeGameFontBackend {
    NativeDiscArchiveLoader& disc;
    std::string language;
    std::unique_ptr<nw4r::ut::ResFont> miiOwner;
    std::map<GameSystemFontHolder*,std::unique_ptr<nw4r::ut::ResFont>> embeddedOwners;
    std::map<GameSystemFontHolder*,std::unique_ptr<NativeArchiveFonts>> owners;
public:
    explicit NativeDiscFontBackend(NativeDiscArchiveLoader& source,std::string prefix="UsEnglish"):disc(source),language(std::move(prefix)){for(char& c:language)if(c>='A' && c<='Z')c=char(c-'A'+'a');}
    NativeDiscFontBackend(const NativeDiscFontBackend&)=delete;
    nw4r::ut::ResFont* miiNameFont() override {
        if(miiOwner)return miiOwner.get();
        std::string archiveName="/LayoutData/Font.arc",resourceName="/messagefont26.brfnt";
        // Petari expects the separate Mii font, absent from the USA disc.
        // Prefer its language-specific archive where supplied by the disc.
        for(const auto& file:disc.files()){
            std::string lower=file.path;for(char& c:lower)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
            if(lower=="/"+language+"/layoutdata/miifont.arc"){archiveName=file.path;break;}
            if(lower=="/layoutdata/miifont.arc")archiveName=file.path;
        }
        if(archiveName!="/LayoutData/Font.arc")resourceName="/miifont26.brfnt";
        auto archive=disc.mount(archiveName);const auto* resource=archive->get(resourceName);
        assetCheck(resource!=nullptr,"Missing Mii-name font resource");
        assetCheck(nativeRegisterFontResource(resource,archive->sizeOf(resource)),"Mii-name font registration failed");
        struct Registration{const void* data;~Registration(){nativeUnregisterFontResource(data);}} registration{resource};
        auto font=std::make_unique<nw4r::ut::ResFont>();
        assetCheck(font->SetResource(const_cast<uint8_t*>(resource)),"Invalid Mii-name font resource");
        font->SetAlternateChar('?');miiOwner=std::move(font);return miiOwner.get();
    }
    NativeDiscFontBackend& operator=(const NativeDiscFontBackend&)=delete;
    ~NativeDiscFontBackend()override{nativeUnbindGameFontBackend(this);while(!owners.empty())release(*owners.begin()->first);while(!embeddedOwners.empty())release(*embeddedOwners.begin()->first);}
    void load(GameSystemFontHolder& holder)override{
        assetCheck(!owners.count(&holder) && (!holder.mEmbeddedMessageFont || (embeddedOwners.count(&holder) && embeddedOwners.at(&holder).get()==holder.mEmbeddedMessageFont)) && !holder._0 && !holder._4 && !holder.mMessageFont && !holder.mPictureFont && !holder.mMenuFont && !holder.mNumberFont && !holder.mCinemaFont,"Game font holder already populated");
        auto archive=disc.mount("/LayoutData/Font.arc");auto candidate=std::make_unique<NativeArchiveFonts>(*archive);
        auto& fonts=owners.emplace(&holder,std::move(candidate)).first->second->fonts;
        holder.mMessageFont=fonts[0].get();holder.mPictureFont=fonts[1].get();holder.mMenuFont=fonts[2].get();holder.mNumberFont=fonts[3].get();holder.mCinemaFont=fonts[4].get();
        holder.mEmbeddedMessageFont=nullptr;embeddedOwners.erase(&holder);
    }
    void loadEmbedded(GameSystemFontHolder& holder)override{
        assetCheck(!owners.count(&holder) && !embeddedOwners.count(&holder) && !holder.mEmbeddedMessageFont && !holder.mMessageFont && !holder._0 && !holder._4,"Embedded font holder already populated");
        auto declared=be32(cErrorArchive+4);assetCheck(declared>=32 && declared<=sizeof(cErrorArchive),"Invalid embedded error archive range");
        OwnedArchiveResources archive(std::vector<uint8_t>(cErrorArchive,cErrorArchive+declared));
        auto bytes=archive.get("/"+language+"/layoutdata/embeddedfont.arc");assetCheck(bytes!=nullptr,"Embedded font language unavailable");
        OwnedArchiveResources nested(std::vector<uint8_t>(bytes,bytes+archive.sizeOf(bytes)));
        auto resource=nested.get("/messagefont26.brfnt");assetCheck(resource!=nullptr,"Missing embedded message font");
        assetCheck(nativeRegisterFontResource(resource,nested.sizeOf(resource)),"Embedded font registration failed");
        struct Registration{const void* value;~Registration(){nativeUnregisterFontResource(value);}} registration{resource};
        auto font=std::make_unique<nw4r::ut::ResFont>();assetCheck(font->SetResource(const_cast<uint8_t*>(resource)),"Invalid embedded message font");
        auto published=font.get();embeddedOwners.emplace(&holder,std::move(font));holder.mEmbeddedMessageFont=published;
    }
    bool release(GameSystemFontHolder& holder){
        bool removed=false;auto embedded=embeddedOwners.find(&holder);
        if(embedded!=embeddedOwners.end()){if(holder.mEmbeddedMessageFont==embedded->second.get())holder.mEmbeddedMessageFont=nullptr;embeddedOwners.erase(embedded);removed=true;}
        auto found=owners.find(&holder);if(found==owners.end())return removed;
        auto& fonts=found->second->fonts;
        nw4r::ut::ResFont** fields[]={&holder.mMessageFont,&holder.mPictureFont,&holder.mMenuFont,&holder.mNumberFont,&holder.mCinemaFont};
        for(size_t i=0;i<5;++i)if(*fields[i]==fonts[i].get())*fields[i]=nullptr;
        owners.erase(found);return true;
    }
};


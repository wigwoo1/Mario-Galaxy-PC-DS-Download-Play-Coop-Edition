#include "native_disc_archive_loader.hpp"
#include "graphics.hpp"
#include <Native/BtiTexture.hpp>
#include <cassert>
#include <cstdio>

int main(int argc,char** argv){
    assert(argc>=2);bool raw=argc==3;
    NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));auto archive=loader.mount("/ObjectData/Mario.arc");
    unsigned checked=0;bool foundFoo=false;
    for(const auto& file:archive->files()){
        auto name=file.path;for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
        if(name.size()<4||name.substr(name.size()-4)!=".bti")continue;
        const auto* source=archive->get(file.path);size_t size=archive->sizeOf(source);std::vector<uint8_t> original(source,source+size);
        auto reference=decodeBti(BinaryView(original));
        ResTIMG rawHeader;std::memcpy(&rawHeader,source,32);
        if(raw){std::printf("Raw BTI %s: host=%ux%u disk=%ux%u\n",file.path.c_str(),rawHeader.mWidth,rawHeader.mHeight,reference.width,reference.height);if(rawHeader.mWidth!=reference.width||rawHeader.mHeight!=reference.height)return 26;continue;}
        Native::detail::BtiStorage texture(source,size);auto* header=texture.texture();
        assert(header->mWidth==reference.width&&header->mHeight==reference.height);
        assert(header->mFormat==reference.format&&header->mWrapS==reference.wrapS&&header->mWrapT==reference.wrapT);
        assert(header->mMinType==reference.minFilter&&header->mMagType==reference.magFilter);
        assert(std::memcmp(source,original.data(),size)==0);
        assert(std::memcmp(texture.bytes.data()+32,source+32,size-32)==0);
        auto image=header->mImageDataOffset?header->mImageDataOffset:32;
        auto palette=BinaryView(texture.bytes).sub(header->mPaletteDataOffset,size_t(header->mPaletteNum)*2);
        auto decoded=decodeGxTexture(BinaryView(texture.bytes),header->mWidth,header->mHeight,header->mFormat,image,palette,header->mPaletteNum,header->mPaletteFormat);
        assert(decoded.pixels.size()==reference.pixels.size());assert(!std::memcmp(decoded.pixels.data(),reference.pixels.data(),decoded.pixels.size()*sizeof(Pixel)));
        if(name.find("fooline.bti")!=std::string::npos){foundFoo=true;assert(header->mWidth==64&&header->mHeight==128);}
        auto rejects=[](const std::vector<uint8_t>& bad){try{Native::detail::BtiStorage invalid(bad.data(),bad.size());return false;}catch(const std::exception&){return true;}};
        assert(rejects(std::vector<uint8_t>(original.begin(),original.begin()+31)));
        auto bad=original;bad[28]=0xff;assert(rejects(bad));bad=original;bad[6]=3;assert(rejects(bad));
        ++checked;std::printf("PASS real BTI %s %ux%u; immutable archive and pixel parity\n",file.path.c_str(),header->mWidth,header->mHeight);
    }
    assert(foundFoo&&checked);assert(loader.unmount("/ObjectData/Mario.arc"));archive.reset();
    std::puts("PASS standalone BTI bounded conversion and independently decoded pixels");
}

#include <Native/DisplayConfig.hpp>
#include <Game/System/RenderMode.hpp>
#include <revolution/vi.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <initializer_list>
int main(){
    unsigned checked=0;
    for(unsigned format:{VI_NTSC,VI_PAL,VI_MPAL,VI_EURGB60})
    for(unsigned flags=0;flags<16;++flags){
        Native::DisplayConfig config{format,bool(flags&1),bool(flags&2),bool(flags&4),bool(flags&8)};
        Native::setDisplayConfig(config);
        auto* mode=MR::getSuitableRenderMode();assert(mode);
        unsigned expectedFormat=format==VI_MPAL?VI_NTSC:format;
        bool progressive=config.progressive && config.digitalOutput;
        if(format==VI_PAL && (progressive || config.euRgb60))expectedFormat=VI_EURGB60;
        assert(mode->viTVmode==VI_TVMODE(expectedFormat,progressive?VI_PROGRESSIVE:VI_INTERLACE));
        assert(MR::isAspectRatioFlag16Per9()==config.widescreen);
        assert(MR::isProgressiveModeOn()==config.progressive);
        assert(MR::isEuRgb60ModeOn()==config.euRgb60);
        assert(mode->fbWidth==640 && mode->xfbHeight>0);
        ++checked;
    }
    auto before=Native::displayConfig();auto invalid=before;invalid.televisionFormat=99;
    bool rejected=false;try{Native::setDisplayConfig(invalid);}catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && Native::displayConfig().televisionFormat==before.televisionFormat);
    std::printf("PASS %u original render-mode configurations and invalid-update rollback\n",checked);
}

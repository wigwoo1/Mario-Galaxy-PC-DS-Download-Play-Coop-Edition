#pragma once
class GameSystemFontHolder;
namespace nw4r {namespace ut {class ResFont;}}
class NativeGameFontBackend {
public:
    virtual ~NativeGameFontBackend()=default;
    virtual void load(GameSystemFontHolder&)=0;
    virtual void loadEmbedded(GameSystemFontHolder&)=0;
    virtual nw4r::ut::ResFont* miiNameFont(){return nullptr;}
};
// Bind/unbind only while font startup and consumers are quiescent.
void nativeBindGameFontBackend(NativeGameFontBackend*);
void nativeLoadGameFonts(GameSystemFontHolder&);
void nativeUnbindGameFontBackend(NativeGameFontBackend*);

void nativeLoadEmbeddedGameFonts(GameSystemFontHolder&);
nw4r::ut::ResFont* nativeLoadMiiNameFont();


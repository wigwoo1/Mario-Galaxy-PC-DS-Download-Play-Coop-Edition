#include <Native/GameFonts.hpp>
#include <stdexcept>
namespace {NativeGameFontBackend* backend=nullptr;}
void nativeBindGameFontBackend(NativeGameFontBackend* value){backend=value;}
void nativeLoadGameFonts(GameSystemFontHolder& holder){if(!backend)throw std::runtime_error("Native game font backend not bound");backend->load(holder);}
void nativeUnbindGameFontBackend(NativeGameFontBackend* value){if(backend==value)backend=nullptr;}
void nativeLoadEmbeddedGameFonts(GameSystemFontHolder& holder){if(!backend)throw std::runtime_error("Native game font backend not bound");backend->loadEmbedded(holder);}
nw4r::ut::ResFont* nativeLoadMiiNameFont(){
 if(!backend)throw std::runtime_error("Native game font backend not bound");
 auto* font=backend->miiNameFont();if(!font)throw std::runtime_error("Mii name font unavailable");return font;
}

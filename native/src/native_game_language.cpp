#include <Native/GameLanguage.hpp>
#include <revolution/sc.h>
#include <atomic>
#include <stdexcept>
namespace { std::atomic<unsigned char> selected{SC_LANG_ENGLISH}; }
namespace Native {
void setGameLanguage(GameLanguage language) {
 const auto value=static_cast<unsigned char>(language);
 if(value!=SC_LANG_ENGLISH&&value!=SC_LANG_FRENCH&&value!=SC_LANG_SPANISH)
  throw std::invalid_argument("Language is not present on the RMGE disc");
 selected.store(value);
}
GameLanguage gameLanguage() { return static_cast<GameLanguage>(selected.load()); }
}
extern "C" u8 SCGetLanguage(void) { return selected.load(); }

#pragma once
#include <cstdint>
namespace Native {
// Resource language supported by the supplied RMGE disc. Configure before
// constructing GameSystemObjHolder; it snapshots the selected language.
enum class GameLanguage : std::uint8_t { English=1, French=3, Spanish=4 };
void setGameLanguage(GameLanguage language);
GameLanguage gameLanguage();
}

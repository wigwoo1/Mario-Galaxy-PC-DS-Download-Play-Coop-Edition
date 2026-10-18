#pragma once
#include <Native/KeyboardKpad.hpp>
#include <cmath>

// Diagnostic observers run on the game frame thread. They never call original
// selection callbacks or change an actor's nerve.
namespace NativeDiagnosticInput {
enum class Phase { Boot, Title, Menu, Confirm, Start, Quiet };
struct State {
 Phase phase=Phase::Boot; unsigned frames=0; float x=0,y=0;
 bool pointerValid=false,clicked=false,confirmed=false,startClicked=false,ready=false;
};
inline State& state(){static State value;return value;}
inline void observe(Phase phase){auto& s=state();if(s.phase!=phase){s.phase=phase;s.frames=0;s.pointerValid=false;s.ready=false;if(phase==Phase::Start)s.startClicked=false;}}
inline void point(bool valid,float x,float y){auto& s=state();s.pointerValid=valid&&std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<=1&&std::abs(y)<=1;s.x=x;s.y=y;}
inline void observeMenu(bool valid,float x,float y){observe(Phase::Menu);point(valid,x,y);}
inline void observeConfirmation(bool valid,float x,float y,bool ready){observe(Phase::Confirm);point(valid,x,y);state().ready=ready;}
inline void observeStartButton(bool valid,float x,float y,bool ready){observe(Phase::Start);point(valid,x,y);state().ready=ready;}
inline NativeKeyboardState next(bool titleAB,bool selectEmpty,bool createProfile=false,bool selectSaved=false,bool selectStart=false){
 auto& s=state();++s.frames;NativeKeyboardState input{};
 if(s.phase==Phase::Boot||s.phase==Phase::Title){input.jump=s.frames>=5&&(s.frames-5)%30==0;input.starbit=input.jump&&titleAB&&s.phase==Phase::Title;}
 if(s.phase==Phase::Menu&&(selectEmpty||selectSaved)&&!s.clicked){input.pointerValid=s.pointerValid;input.pointerX=s.x;input.pointerY=s.y;if(s.pointerValid&&s.frames>=120){input.jump=true;s.clicked=true;}}
 if(s.phase==Phase::Confirm&&createProfile&&!s.confirmed){input.pointerValid=s.pointerValid;input.pointerX=s.x;input.pointerY=s.y;if(s.pointerValid&&s.ready&&s.frames>=45){input.jump=true;s.confirmed=true;}}
 if(s.phase==Phase::Start&&selectStart&&!s.startClicked){input.pointerValid=s.pointerValid;input.pointerX=s.x;input.pointerY=s.y;if(s.pointerValid&&s.ready&&s.frames>=45){input.jump=true;s.startClicked=true;}}
 return input;
}
}

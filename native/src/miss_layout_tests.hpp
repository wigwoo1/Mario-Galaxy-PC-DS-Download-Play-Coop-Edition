#pragma once
#include "assets.hpp"
#include "native_scene.hpp"
#include "Game/Screen/MissLayout.hpp"
inline void missLayoutTests(){
    NativeScene scene;auto& layout=scene.create<MissLayout>();layout.init(JMapInfoIter());
    assetCheck(layout.nativeDead&&!layout.isActive()&&layout.nativeLayout=="TooBad","Original miss layout initialization mismatch");
    scene.tick();layout.start();assetCheck(!layout.nativeDead&&layout.nativeHidden&&layout.isActive(),"Original miss layout start mismatch");
    scene.tick();assetCheck(!layout.nativeHidden&&layout.nativeAnimation=="Appear"&&layout.nativeEffects.empty(),"Original miss layout first-step animation mismatch");
    layout.start();for(int i=1;i<18;++i)scene.tick();assetCheck(layout.nativeEffects.empty(),"Miss effect emitted early or duplicate start reset state");scene.tick();assetCheck(layout.nativeEffects==std::vector<std::string>{"TooBad"},"Original miss frame-18 effect mismatch");
    layout.nativeAnimStopped=true;scene.tick();assetCheck(!layout.isActive()&&!layout.nativeDead,"Original miss animation end transition mismatch");
    layout.kill();assetCheck(layout.nativeDead&&!layout.isActive(),"Original miss kill reset mismatch");layout.start();scene.tick();assetCheck(layout.isActive()&&!layout.nativeAnimStopped,"Original miss restart retained stopped animation");
}
inline void missLayoutPlaybackTests(){
    NativeScene scene;auto& layout=scene.create<MissLayout>();layout.init(JMapInfoIter());layout.registerNativeAnimation("Appear",61,false);layout.registerNativeAnimation("Wait",10,true);layout.start();
    for(int i=0;i<60;++i)scene.tick();assetCheck(layout.isActive()&&!layout.nativeAnimStopped&&layout.nativeAnimationFrame()==60,"Miss authored animation stopped early");scene.tick();assetCheck(layout.isActive()&&layout.nativeAnimStopped,"Miss authored animation did not stop at duration");scene.tick();assetCheck(!layout.isActive(),"Miss original nerve did not observe animation completion");
    layout.kill();layout.start();scene.tick();assetCheck(layout.isActive()&&!layout.nativeAnimStopped&&layout.nativeAnimationFrame()==1,"Miss authored animation restart failed");
    layout.startNativeAnimation("Wait");for(int i=0;i<30;++i)scene.tick();assetCheck(!layout.nativeAnimStopped&&layout.nativeAnimationFrame()==0,"Looping layout animation stopped or failed to wrap");
}

#pragma once
#include "Game/System/NerveExecutor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include <functional>
enum class DiagnosticMotion {Idle,Run,Rise,Fall};
class DiagnosticMotionController:public NerveExecutor {
    struct MotionNerve:Nerve {
        DiagnosticMotion motion;
        MotionNerve(DiagnosticMotion value):motion(value){}
        void execute(Spine* spine)const override{auto host=static_cast<DiagnosticMotionController*>(static_cast<NerveExecutor*>(spine->mExecutor));host->callback(motion,spine->mStep==0);}
        void executeOnEnd(Spine* spine)const override{auto host=static_cast<DiagnosticMotionController*>(static_cast<NerveExecutor*>(spine->mExecutor));++host->ends;}
    };
    inline static MotionNerve nerves[4]={{DiagnosticMotion::Idle},{DiagnosticMotion::Run},{DiagnosticMotion::Rise},{DiagnosticMotion::Fall}};
    std::function<void(DiagnosticMotion,bool)> callback;
public:
    size_t ends=0;
    explicit DiagnosticMotionController(std::function<void(DiagnosticMotion,bool)> action):NerveExecutor("native diagnostic motion"),callback(std::move(action)){initNerve(&nerves[0]);}
    DiagnosticMotionController(const DiagnosticMotionController&)=delete;
    void tick(DiagnosticMotion desired){auto index=size_t(desired);assetCheck(index<4,"Invalid diagnostic motion");if(!isNerve(&nerves[index]))setNerve(&nerves[index]);updateNerve();}
};
inline void diagnosticMotionTests(){
    std::vector<std::pair<DiagnosticMotion,bool>> observed;DiagnosticMotionController controller([&](auto state,bool first){observed.push_back({state,first});});
    controller.tick(DiagnosticMotion::Idle);controller.tick(DiagnosticMotion::Idle);controller.tick(DiagnosticMotion::Rise);controller.tick(DiagnosticMotion::Rise);controller.tick(DiagnosticMotion::Fall);controller.tick(DiagnosticMotion::Run);
    assetCheck(observed.size()==6&&observed[0].second&&!observed[1].second&&observed[2].second&&!observed[3].second&&observed[4].second&&observed[5].second&&controller.ends==3&&controller.getNerveStep()==1,"Original diagnostic motion transition timing failed");
}

#pragma once
#include <vector>
class ScenarioData;
class NativeScenarioSource {
public:
 virtual ~NativeScenarioSource()=default;
 virtual std::vector<const ScenarioData*> scenarioList()=0;
};
// Startup installs the disc session on its own thread before constructing the
// original SceneController/ScenarioDataParser. Source outlives this scope.
inline thread_local NativeScenarioSource* nativeScenarioStartupSource=nullptr;
class NativeScenarioSourceBinding {
 NativeScenarioSource* previous;
public:
 explicit NativeScenarioSourceBinding(NativeScenarioSource& source)
  :previous(nativeScenarioStartupSource){nativeScenarioStartupSource=&source;}
 ~NativeScenarioSourceBinding(){nativeScenarioStartupSource=previous;}
 NativeScenarioSourceBinding(const NativeScenarioSourceBinding&)=delete;
 NativeScenarioSourceBinding& operator=(const NativeScenarioSourceBinding&)=delete;
};

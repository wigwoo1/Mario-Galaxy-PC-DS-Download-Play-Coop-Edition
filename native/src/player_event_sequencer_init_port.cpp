#include "Game/Player/PlayerEvent.hpp"

// The standalone host probe does not install the production death-event scene
// services. The full runtime candidate uses generated/ProductionEventSequencerInit.cpp.
void EventSequencer::init(const JMapInfoIter&) {}

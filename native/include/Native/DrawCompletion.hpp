#pragma once
namespace Native {
// Renderer calls only after the matching submitted GPU draw-done fence completes.
void dispatchDrawDoneCallback();
}

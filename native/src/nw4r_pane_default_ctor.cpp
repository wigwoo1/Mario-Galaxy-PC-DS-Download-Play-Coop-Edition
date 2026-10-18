#include <nw4r/lyt/pane.h>

namespace nw4r {
namespace lyt {

// Host provider for the default pane constructor referenced by NW4R's
// programmatically-created Picture panes. Match the neutral layout defaults:
// origin placement, unit scale, full alpha, visible, and empty labels.
Pane::Pane()
    : mTranslate(0.0f, 0.0f, 0.0f),
      mRotate(0.0f, 0.0f, 0.0f),
      mScale(1.0f, 1.0f),
      mSize(0.0f, 0.0f),
      mAlpha(0xff),
      mGlbAlpha(0xff),
      mBasePosition(0),
      mFlag(1),
      mName{},
      mUserData{},
      mPadding(0) {
    Init();
}

} // namespace lyt
} // namespace nw4r

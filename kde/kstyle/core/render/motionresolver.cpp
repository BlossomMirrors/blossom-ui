// SPDX-License-Identifier: GPL-2.0-or-later
#include "motionresolver.h"

namespace BlossomUI {
namespace Render {

qreal MotionResolver::scale(const Motion &motion,
                            const WidgetInteractionState &state) {
  const AnimationMode mode =
      (state.pressOpacity >= 0.0) ? AnimationPressed : AnimationNone;
  return motion.scale.resolve(state.enabled, state.pressed, state.hovered,
                              state.focused, mode, state.pressOpacity);
}

} // namespace Render
} // namespace BlossomUI

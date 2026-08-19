#ifndef blossomui_core_render_motionresolver_h
#define blossomui_core_render_motionresolver_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "motiontraits.h"
#include "widgetstate.h"

namespace BlossomUI {
namespace Render {

class MotionResolver {
public:
  static qreal scale(const Motion &motion, const WidgetInteractionState &state);
};

} // namespace Render
} // namespace BlossomUI

#endif

#ifndef blossomui_core_render_widgetstate_h
#define blossomui_core_render_widgetstate_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "blossomui.h"
#include "blossomuianimationdata.h"

#include <QPalette>
#include <QPointF>

namespace BlossomUI {
namespace Render {

//* current interaction state of one widget instance, queried once per
//paint call - replaces every draw*() function re-deriving enabled/sunken/
//mouseOver/hasFocus from option->state by hand
struct WidgetInteractionState {
  bool enabled = true;
  bool pressed = false;
  bool hovered = false;
  bool focused = false;
  AnimationMode mode = AnimationNone;
  qreal opacity = AnimationData::OpacityInvalid;
  //* press/release tween progress, independent of mode/opacity above (a
  //widget can be animating hover and press at once)
  qreal pressOpacity = AnimationData::OpacityInvalid;
  //* where the ripple should originate, in widget-local coordinates; null
  //= center
  QPointF ripplePos;
  QPalette palette;
};

} // namespace Render
} // namespace BlossomUI

#endif

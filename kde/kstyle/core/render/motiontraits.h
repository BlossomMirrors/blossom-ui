#ifndef blossomui_core_render_motiontraits_h
#define blossomui_core_render_motiontraits_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "statestyle.h"

#include <QColor>
#include <Qt>

namespace BlossomUI {
namespace Render {

//* state-driven scale/shift/rotation, e.g. this session's animated
//press-shrink: Motion().pressedScale(-2.0).duration(150).curve(OutCubic)
struct Motion {
  Motion &pressedScale(qreal v) {
    scale.pressed(v);
    return markAnimated();
  }
  Motion &hoveredScale(qreal v) {
    scale.hovered(v);
    return markAnimated();
  }
  Motion &pressedShift(qreal dx, qreal dy) {
    shiftX.pressed(dx);
    shiftY.pressed(dy);
    return markAnimated();
  }
  Motion &hoveredShift(qreal dx, qreal dy) {
    shiftX.hovered(dx);
    shiftY.hovered(dy);
    return markAnimated();
  }
  Motion &pressedRotation(qreal degrees) {
    rotation.pressed(degrees);
    return markAnimated();
  }
  //* metadata only - the actual per-frame progress still comes from the
  //existing WidgetStateEngine, not from a timer owned here
  Motion &duration(int ms) {
    durationMs = ms;
    return *this;
  }
  Motion &curve(Easing e) {
    curveValue = e;
    return markAnimated();
  }

  StateStyle<qreal> scale;
  StateStyle<qreal> shiftX;
  StateStyle<qreal> shiftY;
  StateStyle<qreal> rotation;
  int durationMs = -1;
  Easing curveValue = Easing::Linear;

private:
  Motion &markAnimated() {
    scale.animated(curveValue);
    shiftX.animated(curveValue);
    shiftY.animated(curveValue);
    rotation.animated(curveValue);
    return *this;
  }
};

enum class RippleStyle { None, FromClickPosition, FromCenter };

struct Ripple {
  Ripple &style(RippleStyle s) {
    rippleStyle = s;
    return *this;
  }
  Ripple &color(const QColor &c) {
    rippleColor = c;
    return *this;
  }
  Ripple &maxOpacity(qreal v) {
    opacity = v;
    return *this;
  }
  Ripple &duration(int ms) {
    durationMs = ms;
    return *this;
  }

  RippleStyle rippleStyle = RippleStyle::None;
  QColor rippleColor;
  qreal opacity = 0.4;
  int durationMs = -1;
};

struct Cursor {
  Cursor &enabled(Qt::CursorShape s) {
    enabledShape = s;
    hasEnabled = true;
    return *this;
  }
  Cursor &disabled(Qt::CursorShape s) {
    disabledShape = s;
    hasDisabled = true;
    return *this;
  }

  Qt::CursorShape enabledShape = Qt::ArrowCursor;
  Qt::CursorShape disabledShape = Qt::ArrowCursor;
  bool hasEnabled = false;
  bool hasDisabled = false;
};

} // namespace Render
} // namespace BlossomUI

#endif

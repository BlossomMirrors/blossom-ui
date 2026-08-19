#ifndef blossomui_core_render_statestyle_h
#define blossomui_core_render_statestyle_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "blossomui.h"
#include "blossomuianimationdata.h"
#include "traits.h"

namespace BlossomUI {
namespace Render {

template <typename T> class StateStyle {
public:
  StateStyle() = default;
  explicit StateStyle(const T &normalValue) : _normal(normalValue) {}

  StateStyle &normal(const T &v) {
    _normal = v;
    return *this;
  }
  StateStyle &hovered(const T &v) {
    _hovered = v;
    _hasHovered = true;
    return *this;
  }
  StateStyle &pressed(const T &v) {
    _pressed = v;
    _hasPressed = true;
    return *this;
  }
  StateStyle &focused(const T &v) {
    _focused = v;
    _hasFocused = true;
    return *this;
  }
  StateStyle &disabled(const T &v) {
    _disabled = v;
    _hasDisabled = true;
    return *this;
  }
  StateStyle &animated(Easing curve = Easing::Linear) {
    _animated = true;
    _curve = curve;
    return *this;
  }

  T resolve(bool enabled, bool pressed, bool hovered, bool focused,
            AnimationMode mode = AnimationNone,
            qreal opacity = AnimationData::OpacityInvalid) const {
    const T target = valueFor(enabled, pressed, hovered, focused);
    if (!_animated || mode == AnimationNone || opacity < 0.0)
      return target;

    bool fromEnabled = enabled, fromPressed = pressed, fromHovered = hovered,
         fromFocused = focused;
    switch (mode) {
    case AnimationPressed:
      fromPressed = !pressed;
      break;
    case AnimationHover:
      fromHovered = !hovered;
      break;
    case AnimationFocus:
      fromFocused = !focused;
      break;
    case AnimationEnable:
      fromEnabled = !enabled;
      break;
    default:
      return target;
    }
    const T from = valueFor(fromEnabled, fromPressed, fromHovered, fromFocused);
    return lerp(from, target, applyEasing(_curve, opacity));
  }

private:
  //* priority: disabled > pressed > hovered > focused
  T valueFor(bool enabled, bool pressed, bool hovered, bool focused) const {
    if (!enabled && _hasDisabled)
      return _disabled;
    if (pressed && _hasPressed)
      return _pressed;
    if (hovered && _hasHovered)
      return _hovered;
    if (focused && _hasFocused)
      return _focused;
    return _normal;
  }

  T _normal{};
  T _hovered{};
  T _pressed{};
  T _focused{};
  T _disabled{};
  bool _hasHovered = false;
  bool _hasPressed = false;
  bool _hasFocused = false;
  bool _hasDisabled = false;
  bool _animated = false;
  Easing _curve = Easing::Linear;
};

} // namespace Render
} // namespace BlossomUI

#endif

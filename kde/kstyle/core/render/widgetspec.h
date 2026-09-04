#ifndef blossomui_core_render_widgetspec_h
#define blossomui_core_render_widgetspec_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "contenttraits.h"
#include "motiontraits.h"
#include "statestyle.h"
#include "traits.h"

#include <optional>

namespace BlossomUI {
namespace Render {

//* one widget type's complete visual definition - fill, border, geometry,
//motion, ripple and cursor, all in one place instead of scattered across
//render*Frame()/draw*LabelControl() functions
struct WidgetSpec {
  WidgetSpec &geometry(const Geometry &g) {
    geom = g;
    return *this;
  }
  WidgetSpec &fill(const StateStyle<Fill> &f) {
    fillStyle = f;
    return *this;
  }
  WidgetSpec &border(const StateStyle<Border> &b) {
    borderStyle = b;
    return *this;
  }
  WidgetSpec &border(const Border &b) {
    borderStyle = StateStyle<Border>(b);
    return *this;
  }
  WidgetSpec &progress(const Progress &p) {
    progressStyle = p;
    return *this;
  }
  WidgetSpec &shadow(const StateStyle<Shadow> &s) {
    shadowStyle = s;
    return *this;
  }
  WidgetSpec &shadow(const Shadow &s) {
    shadowStyle = StateStyle<Shadow>(s);
    return *this;
  }
  WidgetSpec &motion(const Motion &m) {
    motionStyle = m;
    return *this;
  }
  WidgetSpec &ripple(const Ripple &r) {
    rippleStyle = r;
    return *this;
  }
  WidgetSpec &cursor(const Cursor &c) {
    cursorStyle = c;
    return *this;
  }
  WidgetSpec &text(const TextStyle &t) {
    textStyle = t;
    return *this;
  }
  WidgetSpec &icon(const Icon &i) {
    iconStyle = i;
    return *this;
  }

  Geometry geom;
  StateStyle<Fill> fillStyle;
  StateStyle<Border> borderStyle{Border::none()};
  std::optional<Progress> progressStyle;
  std::optional<StateStyle<Shadow>> shadowStyle;
  Motion motionStyle;
  Ripple rippleStyle;
  Cursor cursorStyle;
  std::optional<TextStyle> textStyle;
  std::optional<Icon> iconStyle;
};

//* fluent builder for WidgetSpec - "changing a border should be ~1 line"
using WidgetBuilder = WidgetSpec;

} // namespace Render
} // namespace BlossomUI

#endif

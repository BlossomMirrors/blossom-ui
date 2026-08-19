#ifndef blossomui_core_render_controls_misccontrol_h
#define blossomui_core_render_controls_misccontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int Splitter_SplitterWidth = 1;

class MiscControl {
public:
  explicit MiscControl(const Style *style) : _style(style) {}

  bool drawMultiTabBarSeparatorPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawShapedFrameControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawRubberBandControl(const QStyleOption *option, QPainter *painter, const QWidget *) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

#ifndef blossomui_core_render_controls_titlebarcontrol_h
#define blossomui_core_render_controls_titlebarcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int TitleBar_MarginWidth = 4;

class TitleBarControl {
public:
  explicit TitleBarControl(const Style *style) : _style(style) {}

  bool drawTitleBarComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

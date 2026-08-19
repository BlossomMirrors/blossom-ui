#ifndef blossomui_core_render_controls_groupboxcontrol_h
#define blossomui_core_render_controls_groupboxcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int GroupBox_TitleMarginWidth = 4;

class GroupBoxControl {
public:
  explicit GroupBoxControl(const Style *style) : _style(style) {}

  bool drawGroupBoxComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

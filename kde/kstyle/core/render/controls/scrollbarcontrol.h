#ifndef blossomui_core_render_controls_scrollbarcontrol_h
#define blossomui_core_render_controls_scrollbarcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class ScrollBarControl {
public:
  explicit ScrollBarControl(const Style *style) : _style(style) {}

  bool drawPanelScrollAreaCornerPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawScrollBarSliderControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawScrollBarAddLineControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawScrollBarSubLineControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawScrollBarComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

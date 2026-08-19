#ifndef blossomui_core_render_controls_slidercontrol_h
#define blossomui_core_render_controls_slidercontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class SliderControl {
public:
  explicit SliderControl(const Style *style) : _style(style) {}

  bool drawSliderComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
  bool drawDialComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

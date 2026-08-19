#ifndef blossomui_core_render_controls_checkboxcontrol_h
#define blossomui_core_render_controls_checkboxcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class CheckBoxControl {
public:
  explicit CheckBoxControl(const Style *style) : _style(style) {}

  bool drawCheckBoxLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

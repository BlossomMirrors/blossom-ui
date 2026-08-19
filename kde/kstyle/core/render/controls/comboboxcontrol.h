#ifndef blossomui_core_render_controls_comboboxcontrol_h
#define blossomui_core_render_controls_comboboxcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "frame.h"

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ComboBox_FrameWidth = 5 + Frame_FrameWidth;

class ComboBoxControl {
public:
  explicit ComboBoxControl(const Style *style) : _style(style) {}

  bool drawComboBoxLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawComboBoxComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

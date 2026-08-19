#ifndef blossomui_core_render_controls_tooltipcontrol_h
#define blossomui_core_render_controls_tooltipcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ToolTip_FrameWidth = 3;

class ToolTipControl {
public:
  explicit ToolTipControl(const Style *style) : _style(style) {}

  bool drawPanelTipLabelPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

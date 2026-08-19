#ifndef blossomui_core_render_controls_toolbarcontrol_h
#define blossomui_core_render_controls_toolbarcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ToolBar_FrameWidth = 8;
static constexpr int ToolBar_HandleExtent = 10;
static constexpr int ToolBar_HandleWidth = 6;
static constexpr int ToolBar_SeparatorWidth = 8;
static constexpr int ToolBar_ItemSpacing = 0;
static constexpr int ToolBar_SeparatorVerticalMargin = 2;

class ToolBarControl {
public:
  explicit ToolBarControl(const Style *style) : _style(style) {}

  bool drawToolBarBackgroundControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

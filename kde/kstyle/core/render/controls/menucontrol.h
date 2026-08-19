#ifndef blossomui_core_render_controls_menucontrol_h
#define blossomui_core_render_controls_menucontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

// widget-owned size constants (logical pixels)
// menu items
static constexpr int MenuItem_MarginWidth = 5;
static constexpr int MenuItem_MarginHeight = 0;
static constexpr int MenuItem_ItemSpacing = 4;
static constexpr int MenuItem_AcceleratorSpace = 16;
// menu bar items
static constexpr int MenuBarItem_MarginWidth = 10;
static constexpr int MenuBarItem_MarginHeight = 6;

class MenuControl {
public:
  explicit MenuControl(const Style *style) : _style(style) {}

  bool drawPanelMenuPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawMenuBarEmptyAreaControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawMenuBarItemControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawMenuItemControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

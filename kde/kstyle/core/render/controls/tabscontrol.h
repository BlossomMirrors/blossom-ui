#ifndef blossomui_core_render_controls_tabscontrol_h
#define blossomui_core_render_controls_tabscontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class TabsControl {
public:
  explicit TabsControl(const Style *style) : _style(style) {}

  bool drawTabBarTabLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawTabBarTabShapeControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawToolBoxTabLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawToolBoxTabShapeControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawDockWidgetTitleControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

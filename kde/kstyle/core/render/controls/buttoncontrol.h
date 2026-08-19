#ifndef blossomui_core_render_controls_buttoncontrol_h
#define blossomui_core_render_controls_buttoncontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include <QPalette>
#include <QRect>

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

//* QPushButton/QToolButton: spec-driven glue between QStyle's draw entry
//points and the render engine. Design lives in style/widgets/buttonspec.
class ButtonControl {
public:
  explicit ButtonControl(const Style *style) : _style(style) {}

  bool drawPanelCommand(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawPanelTool(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawTabBarPanelTool(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawDropDownIndicator(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawPushLabel(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawToolLabel(const QStyleOption *, QPainter *, const QWidget *) const;
  bool drawToolComplex(const QStyleOptionComplex *, QPainter *, const QWidget *) const;

  static void polish(QWidget *);
  static bool polishAutoRaise(QWidget *);

private:
  QRect menuArrowSplit(const QStyleOption *, QPainter *, const QRect &,
                       QPalette::ColorRole) const;
  bool isDolphinNavButton(const QWidget *) const;

  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

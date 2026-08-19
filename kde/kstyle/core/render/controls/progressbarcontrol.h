#ifndef blossomui_core_render_controls_progressbarcontrol_h
#define blossomui_core_render_controls_progressbarcontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class ProgressBarControl {
public:
  explicit ProgressBarControl(const Style *style) : _style(style) {}

  bool drawProgressBarControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawProgressBarContentsControl(const QStyleOption *option, QPainter *painter, const QWidget *) const;
  bool drawProgressBarGrooveControl(const QStyleOption *option, QPainter *painter, const QWidget *) const;
  bool drawProgressBarLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

#ifndef blossomui_core_render_controls_headercontrol_h
#define blossomui_core_render_controls_headercontrol_h

// SPDX-License-Identifier: GPL-2.0-or-later

class QPainter;
class QStyleOption;
class QStyleOptionComplex;
class QWidget;

namespace BlossomUI {
class Style;

namespace Render {

class HeaderControl {
public:
  explicit HeaderControl(const Style *style) : _style(style) {}

  bool drawHeaderSectionControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
  bool drawHeaderEmptyAreaControl(const QStyleOption *option, QPainter *painter, const QWidget *) const;

private:
  const Style *_style;
};

} // namespace Render
} // namespace BlossomUI

#endif

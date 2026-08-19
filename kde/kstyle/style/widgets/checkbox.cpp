// SPDX-License-Identifier: GPL-2.0-or-later
#include "checkbox.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

const WidgetSpec CheckBoxBoxSpec =
    WidgetBuilder().geometry(Geometry().frameInset(1).radius(RadiusRole::Frame));

Fill checkBoxFill(const QPalette &palette, bool on, bool partial, bool mouseOver,
                  bool isInMenu, qreal animation) {
  if (on || partial) {
    QColor background = palette.color(QPalette::Highlight);
    if (animation >= 0.0)
      background = KColorUtils::mix(palette.color(QPalette::Button), background,
                                    qBound(0.0, animation, 1.0));
    if (isInMenu)
      background = background.lighter(110);
    return Fill(mouseOver ? background.lighter(105) : background);
  }
  QColor background = palette.color(QPalette::Button);
  if (isInMenu)
    background = background.lighter(110);
  return Fill(mouseOver ? background.lighter(105) : background);
}

Border checkBoxBorder(const QPalette &palette, const Fill &fill, bool mouseOver,
                      qreal animation) {
  const qreal alpha = mouseOver ? (animation >= 0.0 ? 0.22 : 0.16)
                                : (animation >= 0.0 ? 0.15 : 0.1);
  return Border(
      KColorUtils::mix(fill.brush.color(), palette.color(QPalette::WindowText), alpha),
      1.0);
}

} // namespace Render
} // namespace BlossomUI

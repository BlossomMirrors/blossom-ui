// SPDX-License-Identifier: GPL-2.0-or-later
#include "radiobutton.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

const WidgetSpec RadioButtonSpec =
    WidgetBuilder().geometry(Geometry().frameInset(1).shape(Shape::Ellipse));

Fill radioButtonFill(const Helper *helper, const QPalette &palette, bool on, bool mouseOver, bool isInMenu, qreal animation)
{
    // unchecked radios hover with the same surface as buttons
    const QColor base = mouseOver ? helper->hoverSurface(palette) : palette.color(QPalette::Button);
    QColor active = palette.color(QPalette::Highlight);
    if (isInMenu)
        active = active.lighter(110);
    if (mouseOver)
        active = active.lighter(105);
    QColor background = on ? active : base;
    if (animation >= 0.0)
        background = KColorUtils::mix(base, active, qBound(0.0, animation, 1.0));
    return Fill(background);
}

Border radioButtonBorder(const QPalette &palette, const Fill &fill, bool mouseOver,
                         qreal) {
  return Border(
      KColorUtils::mix(fill.brush.color(), palette.color(QPalette::WindowText),
                       mouseOver ? 0.2 : 0.14),
      1.0);
}

} // namespace Render
} // namespace BlossomUI

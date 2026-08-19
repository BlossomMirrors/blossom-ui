// SPDX-License-Identifier: GPL-2.0-or-later
#include "switch.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

const WidgetSpec SwitchTrackSpec =
    WidgetBuilder().geometry(Geometry().frameInset(1).shape(Shape::Ellipse));

const WidgetSpec SwitchThumbSpec = WidgetBuilder().geometry(Geometry().shape(Shape::Ellipse));

Fill switchTrackFill(const QPalette &palette, bool mouseOver) {
  QColor color = palette.color(QPalette::Button);
  return Fill(mouseOver ? color.lighter(105) : color);
}

Fill switchFillTrailFill(const QPalette &palette, bool mouseOver) {
  QColor color = palette.color(QPalette::Highlight);
  return Fill(mouseOver ? color.lighter(105) : color);
}

Border switchTrackBorder(const QPalette &palette, const Fill &track, const Fill &trail,
                         bool mouseOver, qreal progress) {
  const QColor text = palette.color(QPalette::WindowText);
  const qreal alpha = mouseOver ? 0.16 : 0.1;
  return Border(KColorUtils::mix(KColorUtils::mix(track.brush.color(), text, alpha),
                                 KColorUtils::mix(trail.brush.color(), text, alpha),
                                 progress),
               1.0);
}

Fill switchThumbFill(const QPalette &palette) {
  const QColor window = palette.color(QPalette::Window);
  const QColor text = palette.color(QPalette::WindowText);
  return Fill((KColorUtils::luma(window) > 0.5) ? window : text);
}

Fill switchThumbBorder(const Fill &thumb, const QPalette &palette) {
  return Fill(KColorUtils::mix(thumb.brush.color(),
                               palette.color(QPalette::WindowText), 0.12));
}

Fill switchThumbCrossColor(const Fill &thumb, qreal opacity) {
  QColor color = KColorUtils::mix(thumb.brush.color(), QColor(Qt::black), 0.55);
  color.setAlphaF(color.alphaF() * opacity);
  return Fill(color);
}

Fill switchThumbCheckColor(const Fill &thumb, const Fill &trail, qreal opacity) {
  QColor color = KColorUtils::mix(thumb.brush.color(), trail.brush.color(), 0.85);
  color.setAlphaF(color.alphaF() * opacity);
  return Fill(color);
}

} // namespace Render
} // namespace BlossomUI

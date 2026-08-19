// SPDX-License-Identifier: GPL-2.0-or-later
#include "ripplerenderer.h"

#include "blossomuihelper.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace BlossomUI {
namespace Render {

void RippleRenderer::paint(QPainter *painter, const QRectF &rect, qreal radius,
                           const Ripple &ripple,
                           const WidgetInteractionState &state) const {
  if (ripple.rippleStyle == RippleStyle::None || state.pressOpacity < 0.0)
    return;

  painter->save();
  QPainterPath clipPath;
  clipPath.addRoundedRect(rect, radius, radius);
  painter->setClipPath(clipPath, Qt::IntersectClip);
  painter->setPen(Qt::NoPen);

  const QPointF center = (ripple.rippleStyle == RippleStyle::FromCenter ||
                          state.ripplePos.isNull() || !rect.contains(state.ripplePos))
                             ? rect.center()
                             : state.ripplePos;

  qreal maxDist = 0;
  for (const QPointF &c :
       {rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight()}) {
    const QPointF d = c - center;
    maxDist = qMax(maxDist, qSqrt(d.x() * d.x() + d.y() * d.y()));
  }
  const qreal initRadius = qMin(rect.width(), rect.height()) / 4.0;
  const QColor color = ripple.rippleColor.isValid() ? ripple.rippleColor
                                                     : _helper->focusColor(state.palette);

  if (state.pressed) {
    const qreal r = initRadius + (maxDist - initRadius) * state.pressOpacity;
    painter->setBrush(
        _helper->alphaColor(color, ripple.opacity * (1.0 - state.pressOpacity)));
    painter->drawEllipse(center, r, r);
  } else {
    painter->setBrush(_helper->alphaColor(
        color, ripple.opacity * 0.5 * qSin(state.pressOpacity * M_PI)));
    painter->drawRoundedRect(rect, radius, radius);
  }
  painter->restore();
}

} // namespace Render
} // namespace BlossomUI

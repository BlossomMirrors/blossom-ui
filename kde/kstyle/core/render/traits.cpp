// SPDX-License-Identifier: GPL-2.0-or-later
#include "traits.h"

#include <KColorUtils>
#include <QtMath>

namespace BlossomUI {
namespace Render {

qreal applyEasing(Easing curve, qreal t) {
  switch (curve) {
  case Easing::OutCubic: {
    const qreal f = t - 1.0;
    return f * f * f + 1.0;
  }
  case Easing::OutBack: {
    const qreal c1 = 1.70158;
    const qreal c3 = c1 + 1.0;
    const qreal f = t - 1.0;
    return 1.0 + c3 * f * f * f + c1 * f * f;
  }
  case Easing::Linear:
  default:
    return t;
  }
}

qreal lerp(qreal a, qreal b, qreal t) { return a + (b - a) * t; }

QColor lerp(const QColor &a, const QColor &b, qreal t) {
  if (!a.isValid())
    return b;
  if (!b.isValid())
    return a;
  return KColorUtils::mix(a, b, t);
}

Fill lerp(const Fill &a, const Fill &b, qreal t) {
  // solid colors tween smoothly; anything else (gradient/pattern) snaps
  // at the midpoint since brushes in general aren't interpolable
  if (a.brush.style() == Qt::SolidPattern && b.brush.style() == Qt::SolidPattern) {
    Fill result(lerp(a.brush.color(), b.brush.color(), t));
    result.opacity = lerp(a.opacity, b.opacity, t);
    return result;
  }
  return t < 0.5 ? a : b;
}

Border lerp(const Border &a, const Border &b, qreal t) {
  return Border(lerp(a.color, b.color, t), lerp(a.width, b.width, t),
                t < 0.5 ? a.align : b.align);
}

Shadow lerp(const Shadow &a, const Shadow &b, qreal t) {
  Shadow result;
  result.xOffset = qRound(lerp(qreal(a.xOffset), qreal(b.xOffset), t));
  result.yOffset = qRound(lerp(qreal(a.yOffset), qreal(b.yOffset), t));
  result.blur = qRound(lerp(qreal(a.blur), qreal(b.blur), t));
  result.color = lerp(a.color, b.color, t);
  return result;
}

} // namespace Render
} // namespace BlossomUI

// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"

namespace BlossomUI {

QRectF Helper::strokedRect(const QRectF &rect, const qreal penWidth) const {
  /* With a pen stroke width of 1, the rectangle should have each of its
   * sides moved inwards by half a pixel. This allows the stroke to be
   * pixel perfect instead of blurry from sitting between pixels and
   * prevents the rectangle with a stroke from becoming larger than the
   * original size of the rectangle.
   */
  qreal adjustment = 0.5 * penWidth;
  return QRectF(rect).adjusted(adjustment, adjustment, -adjustment,
                               -adjustment);
}

QPainterPath Helper::roundedPath(const QRectF &rect, Corners corners,
                                 qreal radius) const {
  radius = qMin(radius, 0.5 * qMin(rect.width(), rect.height()));

  QPainterPath path;

  // simple cases
  if (corners == 0) {
    path.addRect(rect);
    return path;
  }

  if (corners == AllCorners) {
    path.addRoundedRect(rect, radius, radius);
    return path;
  }

  const QSizeF cornerSize(2 * radius, 2 * radius);

  // rotate counterclockwise
  // top left corner
  if (corners & CornerTopLeft) {
    path.moveTo(rect.topLeft() + QPointF(radius, 0));
    path.arcTo(QRectF(rect.topLeft(), cornerSize), 90, 90);

  } else
    path.moveTo(rect.topLeft());

  // bottom left corner
  if (corners & CornerBottomLeft) {
    path.lineTo(rect.bottomLeft() - QPointF(0, radius));
    path.arcTo(QRectF(rect.bottomLeft() - QPointF(0, 2 * radius), cornerSize),
               180, 90);

  } else
    path.lineTo(rect.bottomLeft());

  // bottom right corner
  if (corners & CornerBottomRight) {
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * radius, 2 * radius),
                      cornerSize),
               270, 90);

  } else
    path.lineTo(rect.bottomRight());

  // top right corner
  if (corners & CornerTopRight) {
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.arcTo(QRectF(rect.topRight() - QPointF(2 * radius, 0), cornerSize), 0,
               90);

  } else
    path.lineTo(rect.topRight());

  path.closeSubpath();
  return path;
}

QRegion Helper::roundedRectRegionBottomCorners(int w, int h, int radius,
                                               qreal devicePixelRatio) {
  if (w <= 0 || h <= 0)
    return QRegion();
  if (devicePixelRatio > 1.0) {
    const int pw = qRound(qRound(w * devicePixelRatio) / devicePixelRatio);
    const int ph = qRound(qRound(h * devicePixelRatio) / devicePixelRatio);
    w = qMax(1, pw);
    h = qMax(1, ph);
  }
  radius = qBound(0, radius, qMin(w, h) / 2);
  const QRectF rect(0, 0, w, h);
  const qreal r = qreal(radius);
  const QSizeF cornerSize(2 * r, 2 * r);
  QPainterPath path;
  path.moveTo(rect.topLeft());
  path.lineTo(rect.bottomLeft() - QPointF(0, r));
  path.arcTo(QRectF(rect.bottomLeft() - QPointF(0, 2 * r), cornerSize), 180,
             90);
  path.lineTo(rect.bottomRight() - QPointF(r, 0));
  path.arcTo(QRectF(rect.bottomRight() - QPointF(2 * r, 2 * r), cornerSize),
             270, 90);
  path.lineTo(rect.topRight());
  path.lineTo(rect.topLeft());
  path.closeSubpath();
  return QRegion(path.toFillPolygon().toPolygon());
}

} // namespace BlossomUI

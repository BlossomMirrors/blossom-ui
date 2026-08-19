// SPDX-License-Identifier: GPL-2.0-or-later
#include "borderrenderer.h"

#include <QPainter>

namespace BlossomUI {
namespace Render {

void BorderRenderer::paint(QPainter *painter, const QRectF &rect, Shape shape,
                           qreal radius, const Border &border) {
  if (!border.isValid())
    return;

  QPen pen(border.color, border.width);
  pen.setCosmetic(true);
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);

  QRectF strokeRect = rect;
  const qreal half = border.width / 2.0;
  if (border.align == BorderAlign::Inside)
    strokeRect.adjust(half, half, -half, -half);
  else if (border.align == BorderAlign::Outside)
    strokeRect.adjust(-half, -half, half, half);

  if (shape == Shape::Ellipse)
    painter->drawEllipse(strokeRect);
  else
    painter->drawRoundedRect(strokeRect, radius, radius);
}

} // namespace Render
} // namespace BlossomUI

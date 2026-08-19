// SPDX-License-Identifier: GPL-2.0-or-later
#include "fillrenderer.h"

#include <QPainter>

namespace BlossomUI {
namespace Render {

void FillRenderer::paint(QPainter *painter, const QRectF &rect, Shape shape,
                         qreal radius, const Fill &fill) {
  if (!fill.isValid())
    return;

  painter->setCompositionMode(fill.compositeMode);
  painter->setOpacity(fill.opacity);
  painter->setBrush(fill.brush);
  painter->setPen(Qt::NoPen);
  if (shape == Shape::Ellipse)
    painter->drawEllipse(rect);
  else
    painter->drawRoundedRect(rect, radius, radius);
  painter->setOpacity(1.0);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

} // namespace Render
} // namespace BlossomUI

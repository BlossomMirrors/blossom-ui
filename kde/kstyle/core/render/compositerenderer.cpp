// SPDX-License-Identifier: GPL-2.0-or-later
#include "compositerenderer.h"

#include "motionresolver.h"

namespace BlossomUI {
namespace Render {

static void paintScaledContent(QPainter *painter, const QRectF &rect,
                               const Part &part,
                               const WidgetInteractionState &state,
                               const ContentRenderer &content) {
  const qreal shrink = MotionResolver::scale(part.motion, state);
  if (qFuzzyIsNull(shrink) || rect.width() <= 0 || rect.height() <= 0) {
    content.paint(painter, rect, part.contentLayout, part.content, state);
    return;
  }

  const qreal sx = (rect.width() - 2 * shrink) / rect.width();
  const qreal sy = (rect.height() - 2 * shrink) / rect.height();
  painter->save();
  painter->translate(rect.center());
  painter->scale(sx, sy);
  painter->translate(-rect.center());
  content.paint(painter, rect, part.contentLayout, part.content, state);
  painter->restore();
}

void CompositeRenderer::render(QPainter *painter, const QRect &rect,
                               const QVector<Part> &parts,
                               const WidgetInteractionState &state) const {
  for (const Part &part : parts) {
    const QRectF partRect = part.placement.resolve(QRectF(rect), state);
    switch (part.kind) {
    case Part::Kind::Shape:
      _shapes.render(painter, partRect, part.shape, state);
      break;
    case Part::Kind::Content:
      paintScaledContent(painter, partRect, part, state, _content);
      break;
    case Part::Kind::Custom:
      if (part.custom.draw)
        part.custom.draw(painter, partRect, state);
      break;
    case Part::Kind::Glyph:
      break;
    }
  }
}

} // namespace Render
} // namespace BlossomUI

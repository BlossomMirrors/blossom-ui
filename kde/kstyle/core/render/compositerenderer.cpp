// SPDX-License-Identifier: GPL-2.0-or-later
#include "compositerenderer.h"

namespace BlossomUI {
namespace Render {

void CompositeRenderer::render(QPainter *painter, const QRect &rect,
                               const QVector<Part> &parts,
                               const WidgetInteractionState &state) const {
  for (const Part &part : parts) {
    const QRect partRect = part.placement.resolve(QRectF(rect), state).toRect();
    switch (part.kind) {
    case Part::Kind::Shape:
      _shapes.render(painter, partRect, part.shape, state);
      break;
    case Part::Kind::Content:
      _content.paint(painter, partRect, part.contentLayout, part.content, state);
      break;
    case Part::Kind::Custom:
      if (part.custom.draw)
        part.custom.draw(painter, QRectF(partRect), state);
      break;
    case Part::Kind::Glyph:
      break;
    }
  }
}

} // namespace Render
} // namespace BlossomUI

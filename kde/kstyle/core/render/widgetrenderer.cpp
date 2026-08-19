// SPDX-License-Identifier: GPL-2.0-or-later
#include "widgetrenderer.h"

#include "borderrenderer.h"
#include "fillrenderer.h"
#include "motionresolver.h"

#include <QPainter>
#include <QtMath>

namespace BlossomUI {
namespace Render {

qreal WidgetRenderer::pressShrink(const WidgetSpec &spec,
                                  const WidgetInteractionState &state) const {
  return MotionResolver::scale(spec.motionStyle, state);
}

QRectF WidgetRenderer::contentRect(const QRectF &rect, const WidgetSpec &spec,
                                   const WidgetInteractionState &state) const {
  const qreal inset = spec.geom.inset;
  QRectF r(rect.adjusted(inset, inset, -inset, -inset));
  const qreal shrink = pressShrink(spec, state);
  r.adjust(shrink, shrink, -shrink, -shrink);
  return r;
}

void WidgetRenderer::render(QPainter *painter, const QRectF &rect,
                            const WidgetSpec &spec,
                            const WidgetInteractionState &state) const {
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF frameRect = contentRect(rect, spec, state);
  const qreal maxRadius = qMin(frameRect.height(), frameRect.width()) / 2.0;
  const qreal radius = qMin(_radius.resolve(spec.geom), maxRadius);

  const Fill fill = spec.fillStyle.resolve(state.enabled, state.pressed,
                                           state.hovered, state.focused);
  FillRenderer::paint(painter, frameRect, spec.geom.frameShape, radius, fill);

  const Border border = spec.borderStyle.resolve(state.enabled, state.pressed,
                                                  state.hovered, state.focused);
  BorderRenderer::paint(painter, frameRect, spec.geom.frameShape, radius, border);

  _ripple.paint(painter, frameRect, radius, spec.rippleStyle, state);
}

} // namespace Render
} // namespace BlossomUI

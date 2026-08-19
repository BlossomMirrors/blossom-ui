// SPDX-License-Identifier: GPL-2.0-or-later
#include "placement.h"

namespace BlossomUI {
namespace Render {

Placement Placement::fill() {
  Placement p;
  p.compute = [](const QRectF &bounds, const WidgetInteractionState &) {
    return bounds;
  };
  return p;
}

Placement Placement::centered(qreal w, qreal h) {
  Placement p;
  p.compute = [w, h](const QRectF &bounds, const WidgetInteractionState &) {
    return QRectF(bounds.center().x() - w / 2.0, bounds.center().y() - h / 2.0,
                  w, h);
  };
  return p;
}

Placement Placement::anchoredLeft(qreal w) {
  Placement p;
  p.compute = [w](const QRectF &bounds, const WidgetInteractionState &) {
    return QRectF(bounds.left(), bounds.top(), w, bounds.height());
  };
  return p;
}

Placement Placement::anchoredRight(qreal w) {
  Placement p;
  p.compute = [w](const QRectF &bounds, const WidgetInteractionState &) {
    return QRectF(bounds.right() - w, bounds.top(), w, bounds.height());
  };
  return p;
}

Placement Placement::inset(qreal margin) {
  Placement p;
  p.compute = [margin](const QRectF &bounds, const WidgetInteractionState &) {
    return bounds.adjusted(margin, margin, -margin, -margin);
  };
  return p;
}

Placement Placement::slide(StateStyle<qreal> position, qreal partSize) {
  Placement p;
  p.compute = [position, partSize](const QRectF &bounds,
                                    const WidgetInteractionState &state) {
    const qreal t = position.resolve(state.enabled, state.pressed,
                                      state.hovered, state.focused, state.mode,
                                      state.opacity);
    const qreal x = bounds.left() + (bounds.width() - partSize) * t;
    return QRectF(x, bounds.top(), partSize, bounds.height());
  };
  return p;
}

QRectF Placement::resolve(const QRectF &bounds,
                          const WidgetInteractionState &state) const {
  return compute ? compute(bounds, state) : bounds;
}

QRectF SlidingIndicator::resolve() const {
  const qreal t = applyEasing(curve, progress);
  const qreal x = fromRect.x() + (toRect.x() - fromRect.x()) * t;
  const qreal y = fromRect.y() + (toRect.y() - fromRect.y()) * t;
  const qreal w = fromRect.width() + (toRect.width() - fromRect.width()) * t;
  const qreal h = fromRect.height() + (toRect.height() - fromRect.height()) * t;
  return QRectF(x, y, w, h);
}

} // namespace Render
} // namespace BlossomUI

#ifndef blossomui_core_render_placement_h
#define blossomui_core_render_placement_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "motiontraits.h"
#include "statestyle.h"
#include "widgetstate.h"

#include <QPainter>
#include <QRectF>
#include <functional>

namespace BlossomUI {
namespace Render {

class Placement {
public:
  static Placement fill();
  static Placement centered(qreal w, qreal h);
  static Placement anchoredLeft(qreal w);
  static Placement anchoredRight(qreal w);
  static Placement inset(qreal margin);
  static Placement slide(StateStyle<qreal> position, qreal partSize);

  QRectF resolve(const QRectF &bounds, const WidgetInteractionState &state) const;

private:
  std::function<QRectF(const QRectF &, const WidgetInteractionState &)> compute;
};

//* rect morphs between two other parts' rects over time, e.g. the tab
//bar's selected-tab pill - composite-level, not a per-part Placement
struct SlidingIndicator {
  QRectF fromRect;
  QRectF toRect;
  qreal progress = 1.0;
  Easing curve = Easing::OutCubic;
  bool clipToContainer = false;

  QRectF resolve() const;
};

//* raw draw-callback escape hatch for shapes that don't reduce to
//fill+border+glyph
struct CustomPart {
  std::function<void(QPainter *, const QRectF &, const WidgetInteractionState &)>
      draw;
};

} // namespace Render
} // namespace BlossomUI

#endif

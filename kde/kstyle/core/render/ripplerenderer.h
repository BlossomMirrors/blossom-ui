#ifndef blossomui_core_render_ripplerenderer_h
#define blossomui_core_render_ripplerenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "motiontraits.h"
#include "widgetstate.h"

#include <QRectF>

class QPainter;

namespace BlossomUI {
class Helper;

namespace Render {

class RippleRenderer {
public:
  explicit RippleRenderer(const Helper *helper) : _helper(helper) {}

  void paint(QPainter *painter, const QRectF &rect, qreal radius,
            const Ripple &ripple, const WidgetInteractionState &state) const;

private:
  const Helper *_helper;
};

} // namespace Render
} // namespace BlossomUI

#endif

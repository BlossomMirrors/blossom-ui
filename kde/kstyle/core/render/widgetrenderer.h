#ifndef blossomui_core_render_widgetrenderer_h
#define blossomui_core_render_widgetrenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "radiusresolver.h"
#include "ripplerenderer.h"
#include "widgetspec.h"
#include "widgetstate.h"

#include <QRect>
#include <QRectF>

class QPainter;

namespace BlossomUI {
class Helper;

namespace Render {

class WidgetRenderer {
public:
  explicit WidgetRenderer(const Helper *helper)
      : _helper(helper), _radius(helper), _ripple(helper) {}

  void render(QPainter *painter, const QRectF &rect, const WidgetSpec &spec,
              const WidgetInteractionState &state) const;

  QRectF contentRect(const QRectF &rect, const WidgetSpec &spec,
                     const WidgetInteractionState &state) const;

  qreal pressShrink(const WidgetSpec &spec,
                    const WidgetInteractionState &state) const;

private:
  const Helper *_helper;
  RadiusResolver _radius;
  RippleRenderer _ripple;
};

} // namespace Render
} // namespace BlossomUI

#endif

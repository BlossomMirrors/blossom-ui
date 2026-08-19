#ifndef blossomui_core_render_borderrenderer_h
#define blossomui_core_render_borderrenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QRectF>

class QPainter;

namespace BlossomUI {
namespace Render {

class BorderRenderer {
public:
  static void paint(QPainter *painter, const QRectF &rect, Shape shape,
                    qreal radius, const Border &border);
};

} // namespace Render
} // namespace BlossomUI

#endif

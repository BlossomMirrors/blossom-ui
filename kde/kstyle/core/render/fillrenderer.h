#ifndef blossomui_core_render_fillrenderer_h
#define blossomui_core_render_fillrenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QRectF>

class QPainter;

namespace BlossomUI {
namespace Render {

class FillRenderer {
public:
  static void paint(QPainter *painter, const QRectF &rect, Shape shape,
                    qreal radius, const Fill &fill);
};

} // namespace Render
} // namespace BlossomUI

#endif

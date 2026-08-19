// SPDX-License-Identifier: GPL-2.0-or-later
#include "frame.h"

namespace BlossomUI {
namespace Render {

const WidgetSpec PlainFrameSpec =
    WidgetBuilder().geometry(Geometry().frameInset(2).radius(RadiusRole::Frame));

const WidgetSpec MenuFrameSpec = WidgetBuilder().geometry(Geometry().fixedRadius(0));

Border plainFrameBorder(const QPalette &palette) {
  QColor color = palette.color(QPalette::WindowText);
  color.setAlphaF(color.alphaF() * 0.16);
  return Border(color, 1.0, BorderAlign::Inside);
}

} // namespace Render
} // namespace BlossomUI

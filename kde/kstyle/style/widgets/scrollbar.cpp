// SPDX-License-Identifier: GPL-2.0-or-later
#include "scrollbar.h"

#include "blossomuihelper.h"

namespace BlossomUI {
namespace Render {

Fill scrollBarHandleFill(const Helper *helper, const QPalette &palette, bool sunken,
                         qreal grooveAnimationOpacity) {
  QColor color = sunken ? palette.color(QPalette::Highlight)
                        : helper->alphaColor(palette.color(QPalette::WindowText), 0.35);
  color.setAlphaF(color.alphaF() * (0.7 + 0.3 * grooveAnimationOpacity));
  return Fill(color);
}

} // namespace Render
} // namespace BlossomUI

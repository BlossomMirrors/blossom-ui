// SPDX-License-Identifier: GPL-2.0-or-later
#include "tabbar.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill tabFill(const QPalette &palette, bool selected, bool mouseOver, bool animated,
            qreal opacity) {
  if (selected) {
    QColor color = palette.color(QPalette::Highlight);
    color.setAlphaF(0.18);
    return Fill(color);
  }
  const QColor transparent(0, 0, 0, 0);
  const QColor hover = palette.color(QPalette::Button);
  if (animated)
    return Fill(KColorUtils::mix(transparent, hover, opacity));
  if (mouseOver)
    return Fill(hover);
  return Fill(transparent);
}

} // namespace Render
} // namespace BlossomUI

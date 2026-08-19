// SPDX-License-Identifier: GPL-2.0-or-later
#include "itemview.h"

namespace BlossomUI {
namespace Render {

Fill itemViewSelectionFill(const QPalette &palette, QPalette::ColorGroup group,
                           bool selected, bool mouseOver,
                           const QColor &customBackground) {
  if (customBackground.isValid())
    return Fill(customBackground);
  if (selected) {
    QColor color = palette.color(group, QPalette::Highlight);
    color.setAlphaF(mouseOver ? 0.28 : 0.18);
    return Fill(color);
  }
  if (mouseOver)
    return Fill(palette.color(group, QPalette::Button));
  return Fill();
}

} // namespace Render
} // namespace BlossomUI

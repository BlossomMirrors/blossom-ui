// SPDX-License-Identifier: GPL-2.0-or-later
#include "itemview.h"

#include "selectionstyle.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill itemViewSelectionFill(const QPalette &palette, QPalette::ColorGroup group,
                           bool selected, bool mouseOver,
                           const QColor &customBackground) {
  if (customBackground.isValid())
    return Fill(customBackground);
  if (selected)
    return Fill(SelectionStyle::tint(palette, group, mouseOver));
  if (mouseOver)
    return Fill(palette.color(group, QPalette::Button));
  return Fill();
}

} // namespace Render
} // namespace BlossomUI

// SPDX-License-Identifier: GPL-2.0-or-later
#include "spinbox.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill spinBoxArrowColor(const Helper *helper, const QPalette &palette, bool animated,
                       qreal opacity, bool subControlHover, bool atLimit) {
  QColor color = helper->arrowColor(palette, QPalette::Text);
  if (animated) {
    const QColor highlight = helper->hoverColor(palette);
    color = KColorUtils::mix(color, highlight, opacity);
  } else if (subControlHover) {
    color = helper->focusColor(palette);
  } else if (atLimit) {
    color = helper->arrowColor(palette, QPalette::Disabled, QPalette::Text);
  }
  return Fill(color);
}

} // namespace Render
} // namespace BlossomUI

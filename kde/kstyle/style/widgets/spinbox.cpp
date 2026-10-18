// SPDX-License-Identifier: GPL-2.0-or-later
#include "spinbox.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill spinBoxArrowColor(const Helper *helper, const QPalette &palette, bool animated,
                       qreal opacity, bool subControlHover, bool atLimit) {
    Q_UNUSED(animated)
    Q_UNUSED(opacity)
    Q_UNUSED(subControlHover)
    QColor color = helper->arrowColor(palette, QPalette::Text);
    if (atLimit) {
        color = helper->arrowColor(palette, QPalette::Disabled, QPalette::Text);
    }
  return Fill(color);
}

} // namespace Render
} // namespace BlossomUI

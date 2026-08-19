// SPDX-License-Identifier: GPL-2.0-or-later
#include "card.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill cardBackgroundFill(const QColor &windowColor) {
  const bool isDark = windowColor.lightness() < 128;
  return Fill(isDark ? KColorUtils::mix(windowColor, QColor(255, 255, 255), 0.12)
                     : KColorUtils::mix(windowColor, QColor(0, 0, 0), 0.04));
}

} // namespace Render
} // namespace BlossomUI

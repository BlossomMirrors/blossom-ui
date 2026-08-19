// SPDX-License-Identifier: GPL-2.0-or-later
#include "progressbar.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill progressBarBusyFirst(const QPalette &palette) {
  return Fill(palette.color(QPalette::Highlight));
}

Fill progressBarBusySecond(const QPalette &palette) {
  return Fill(KColorUtils::mix(palette.color(QPalette::Highlight),
                               palette.color(QPalette::Window), 0.7));
}

} // namespace Render
} // namespace BlossomUI

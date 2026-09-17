// SPDX-License-Identifier: GPL-2.0-or-later
#include "progressbar.h"

namespace BlossomUI {
namespace Render {

Fill progressBarBusyFirst(const QPalette &palette) {
  return Fill(palette.color(QPalette::Highlight));
}

} // namespace Render
} // namespace BlossomUI

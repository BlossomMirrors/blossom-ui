#ifndef blossomui_style_widgets_progressbar_h
#define blossomui_style_widgets_progressbar_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QPalette>

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ProgressBar_BusyIndicatorSize = 14;
static constexpr int ProgressBar_Thickness = 4;
static constexpr int ProgressBar_ItemSpacing = 4;

Fill progressBarBusyFirst(const QPalette &palette);
Fill progressBarBusySecond(const QPalette &palette);

} // namespace Render
} // namespace BlossomUI

#endif

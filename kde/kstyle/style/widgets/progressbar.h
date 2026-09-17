#ifndef blossomui_style_widgets_progressbar_h
#define blossomui_style_widgets_progressbar_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QPalette>

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ProgressBar_Thickness = 10;
static constexpr int ProgressBar_BusySteps = 1000;
static constexpr qreal ProgressBar_BusySegmentFraction = 0.3;
static constexpr int ProgressBar_ItemSpacing = 4;

Fill progressBarBusyFirst(const QPalette &palette);

} // namespace Render
} // namespace BlossomUI

#endif

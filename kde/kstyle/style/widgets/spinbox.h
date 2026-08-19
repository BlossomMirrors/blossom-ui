#ifndef blossomui_style_widgets_spinbox_h
#define blossomui_style_widgets_spinbox_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"
#include "frame.h"

namespace BlossomUI {
class Helper;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int SpinBox_FrameWidth = LineEdit_FrameWidth;
static constexpr int SpinBox_ArrowButtonWidth = 20;

Fill spinBoxArrowColor(const Helper *helper, const QPalette &palette, bool animated,
                       qreal opacity, bool subControlHover, bool atLimit);

} // namespace Render
} // namespace BlossomUI

#endif

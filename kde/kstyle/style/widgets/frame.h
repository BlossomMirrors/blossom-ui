#ifndef blossomui_style_widgets_frame_h
#define blossomui_style_widgets_frame_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "widgetspec.h"

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
// frames
static constexpr int Frame_FrameWidth = 2;
static constexpr int LineEdit_FrameWidth = 5 + Frame_FrameWidth;
static constexpr int LineEdit_HPadding = 4;

//* plain rounded frame with a faint WindowText-tinted border - groupbox,
//tooltip/menu frame base shape
extern const WidgetSpec PlainFrameSpec;
extern const WidgetSpec MenuFrameSpec;

Border plainFrameBorder(const QPalette &palette);

} // namespace Render
} // namespace BlossomUI

#endif

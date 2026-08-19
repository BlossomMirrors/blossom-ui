#ifndef blossomui_style_widgets_checkbox_h
#define blossomui_style_widgets_checkbox_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "widgetspec.h"
#include "frame.h"

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int CheckBox_Size = 16 + (Frame_FrameWidth - 1) * 2;
static constexpr int CheckBox_FocusMarginWidth = 2;
static constexpr int CheckBox_ItemSpacing = 4;

extern const WidgetSpec CheckBoxBoxSpec;

Fill checkBoxFill(const QPalette &palette, bool on, bool partial, bool mouseOver,
                  bool isInMenu, qreal animation);
Border checkBoxBorder(const QPalette &palette, const Fill &fill, bool mouseOver,
                      qreal animation);

} // namespace Render
} // namespace BlossomUI

#endif

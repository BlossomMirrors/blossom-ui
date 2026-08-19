#ifndef blossomui_style_widgets_radiobutton_h
#define blossomui_style_widgets_radiobutton_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "widgetspec.h"

namespace BlossomUI {
namespace Render {

extern const WidgetSpec RadioButtonSpec;

Fill radioButtonFill(const QPalette &palette, bool on, bool mouseOver,
                     bool isInMenu, qreal animation);
Border radioButtonBorder(const QPalette &palette, const Fill &fill,
                         bool mouseOver, qreal animation);

} // namespace Render
} // namespace BlossomUI

#endif

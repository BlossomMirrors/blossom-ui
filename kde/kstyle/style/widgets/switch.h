#ifndef blossomui_style_widgets_switch_h
#define blossomui_style_widgets_switch_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "widgetspec.h"

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int Switch_Width = 46;
static constexpr int Switch_Height = 26;
static constexpr int Switch_ThumbMargin = 3;

extern const WidgetSpec SwitchTrackSpec;
extern const WidgetSpec SwitchThumbSpec;

Fill switchTrackFill(const QPalette &palette, bool mouseOver);
Fill switchFillTrailFill(const QPalette &palette, bool mouseOver);
Border switchTrackBorder(const QPalette &palette, const Fill &track,
                         const Fill &trail, bool mouseOver, qreal progress);

Fill switchThumbFill(const QPalette &palette);
Fill switchThumbBorder(const Fill &thumb, const QPalette &palette);
Fill switchThumbCrossColor(const Fill &thumb, qreal opacity);
Fill switchThumbCheckColor(const Fill &thumb, const Fill &trail, qreal opacity);

} // namespace Render
} // namespace BlossomUI

#endif

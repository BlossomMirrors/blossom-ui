#ifndef blossomui_style_widgets_slider_h
#define blossomui_style_widgets_slider_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

namespace BlossomUI {
class Helper;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int Slider_TickLength = 8;
static constexpr int Slider_TickMarginWidth = 2;
static constexpr int Slider_GrooveThickness = 8;
static constexpr int Slider_ControlThickness = 20;
static constexpr int Slider_HoverMargin = 5;

Fill sliderGrooveFill(const Helper *helper, const QPalette &palette);
Fill sliderGrooveHighlight(const QPalette &palette);
Fill sliderHandleFill(const QPalette &palette);
Fill sliderHandleOutline(const QPalette &palette);

Fill dialGrooveFill(const QPalette &palette);
Fill dialHighlight(const QPalette &palette);
Fill dialHandleOutline(const QPalette &palette);

} // namespace Render
} // namespace BlossomUI

#endif

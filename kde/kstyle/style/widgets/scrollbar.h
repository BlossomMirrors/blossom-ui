#ifndef blossomui_style_widgets_scrollbar_h
#define blossomui_style_widgets_scrollbar_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

namespace BlossomUI {
class Helper;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ScrollBar_Extend = 21;
static constexpr int ScrollBar_SliderWidth = 8;
static constexpr int ScrollBar_MinSliderHeight = 20;
static constexpr int ScrollBar_NoButtonHeight = (ScrollBar_Extend - ScrollBar_SliderWidth) / 2;
static constexpr int ScrollBar_SingleButtonHeight = ScrollBar_Extend;
static constexpr int ScrollBar_DoubleButtonHeight = 2 * ScrollBar_Extend;

Fill scrollBarHandleFill(const Helper *helper, const QPalette &palette, bool sunken,
                         qreal grooveAnimationOpacity);

} // namespace Render
} // namespace BlossomUI

#endif

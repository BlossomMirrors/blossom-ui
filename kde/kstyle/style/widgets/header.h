#ifndef blossomui_style_widgets_header_h
#define blossomui_style_widgets_header_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

namespace BlossomUI {
class Helper;

namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int Header_MarginWidth = 6;
static constexpr int Header_ItemSpacing = 4;
static constexpr int Header_ArrowSize = 10;

Fill headerSectionFill(const Helper *helper, const QPalette &palette, bool sunken,
                       bool mouseOver, bool animated, qreal opacity);

} // namespace Render
} // namespace BlossomUI

#endif

#ifndef blossomui_style_widgets_tabbar_h
#define blossomui_style_widgets_tabbar_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QPalette>

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int TabBar_TabMarginHeight = 8;
static constexpr int TabBar_TabMarginWidth = 12;
static constexpr int TabBar_TabMinWidth = 85;
static constexpr int TabBar_TabMinHeight = 36;
static constexpr int TabBar_TabItemSpacing = 8;
static constexpr int TabBar_TabOverlap = 0;
static constexpr int TabBar_BaseOverlap = 2;
// tab widget
static constexpr int TabWidget_MarginWidth = 4;
// toolbox
static constexpr int ToolBox_TabMinWidth = 80;
static constexpr int ToolBox_TabItemSpacing = 4;
static constexpr int ToolBox_TabMarginWidth = 8;

Fill tabFill(const QPalette &palette, bool selected, bool mouseOver, bool animated,
            qreal opacity);

} // namespace Render
} // namespace BlossomUI

#endif

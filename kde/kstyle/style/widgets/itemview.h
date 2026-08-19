#ifndef blossomui_style_widgets_itemview_h
#define blossomui_style_widgets_itemview_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

#include <QPalette>

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int ItemView_ArrowSize = 10;
static constexpr int ItemView_ItemMarginWidth = 12;
static constexpr int ItemView_ItemMarginLeft = 2;
static constexpr int ItemView_ItemMarginRight = 2;
static constexpr int ItemView_ItemMarginTop = 1;
static constexpr int ItemView_ItemMarginBottom = 1;
static constexpr int ItemView_FirstItemTopMarginHeight = 2;
static constexpr int ItemView_ItemPaddingWidth = 4;
static constexpr int ItemView_ItemPaddingHeight = 3;
static constexpr int ItemView_IconTextSpacing = 6;

Fill itemViewSelectionFill(const QPalette &palette, QPalette::ColorGroup group,
                           bool selected, bool mouseOver,
                           const QColor &customBackground = QColor());

} // namespace Render
} // namespace BlossomUI

#endif

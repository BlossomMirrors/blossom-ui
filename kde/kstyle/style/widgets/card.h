#ifndef blossomui_style_widgets_card_h
#define blossomui_style_widgets_card_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "frame.h"
#include "widgetspec.h"

#include <QPalette>

namespace BlossomUI {
namespace Render {

// widget-owned size constants (logical pixels)
static constexpr int Card_Padding = 12;
static constexpr int Card_RadiusBias = 2;
static constexpr int Card_BorderWidth = 1;
static constexpr int Card_ShadowYOffset = 1;
static constexpr int Card_ShadowBlur = 2;
// alpha, 0-255
static constexpr int Card_ShadowAlphaDark = 110;
static constexpr int Card_ShadowAlphaLight = 45;
static constexpr qreal Card_BorderAlpha = 0.10;

extern const WidgetSpec CardSpec;

//* card surface with its fill, border and elevation resolved for a palette
WidgetSpec cardFrame(const QPalette &palette, const QColor &background);

//* painted corner radius of a card, for hand-rolled card painters that can't
//go through WidgetRenderer (the Dolphin overlay)
int cardRadius();

//* Dolphin "card" background - lighter than the window in dark themes,
//darker in light themes
Fill cardBackgroundFill(const QColor &windowColor);

Border cardBorder(const QPalette &palette);

} // namespace Render
} // namespace BlossomUI

#endif

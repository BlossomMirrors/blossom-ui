#ifndef blossomui_style_widgets_card_h
#define blossomui_style_widgets_card_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "traits.h"

namespace BlossomUI {
namespace Render {

//* Dolphin "card" background - lighter than the window in dark themes,
//darker in light themes
Fill cardBackgroundFill(const QColor &windowColor);

} // namespace Render
} // namespace BlossomUI

#endif

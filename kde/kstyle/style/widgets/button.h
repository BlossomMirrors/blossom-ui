#ifndef blossomui_style_widgets_button_h
#define blossomui_style_widgets_button_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "contentrenderer.h"
#include "widgetspec.h"
#include "widgetstate.h"
#include "frame.h"

namespace BlossomUI {
class Helper;

namespace Render {

// widget-owned size constants (logical pixels)
// buttons
static constexpr int Button_MinWidth = 80;
static constexpr int Button_MarginWidth = 6;
static constexpr int Button_ItemSpacing = 4;
static constexpr int Button_PressedShrink = 2;
// tool buttons
static constexpr int ToolButton_MarginWidth = 6;
static constexpr int ToolButton_ItemSpacing = 4;
// shared: button/combobox/menu dropdown arrow width
static constexpr int MenuButton_IndicatorWidth = 20;

extern const WidgetSpec ButtonSpec;
extern const WidgetSpec ToolButtonSpec;

//* raised button frame: base fill plus its hover/pressed variants
WidgetSpec buttonFrame(const Helper *, const WidgetInteractionState &,
                       bool defaultButton = false);

//* flat/autoRaise ghost frame: hover and press tint only, no resting fill
WidgetSpec toolButtonFrame(const Helper *, const WidgetInteractionState &);

ContentLayout buttonContent(int itemSpacing);
ContentLayout toolButtonContent(Qt::ToolButtonStyle, const QSize &iconSize,
                                bool leftAligned);

QPalette::ColorRole buttonTextRole(bool flat, bool focused, bool pressed);
QPalette::ColorRole toolButtonTextRole(bool flat, bool focused, bool pressed,
                                       bool hovered);
QIcon::Mode buttonIconMode(bool enabled, bool flat, bool focused, bool pressed,
                           bool hovered);
QIcon::Mode toolButtonIconMode(bool enabled, bool flat, bool focused,
                               bool pressed, bool hovered);

} // namespace Render
} // namespace BlossomUI

#endif

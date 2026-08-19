#ifndef blossomui_core_render_stateextractor_h
#define blossomui_core_render_stateextractor_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "blossomui.h"
#include "widgetstate.h"

class QStyleOption;
class QWidget;

namespace BlossomUI {
class WidgetStateEngine;

namespace Render {

enum class FocusRule { Standard, IncludesSunken };

struct StateFlags {
  FocusRule focusRule = FocusRule::Standard;
};

QPointF ripplePosition(const QObject *styleObject, bool isQtQuick);

WidgetInteractionState extractState(const QStyleOption *option, const QWidget *widget,
                                    bool isQtQuick, WidgetStateEngine &engine,
                                    AnimationModes transitions,
                                    StateFlags flags = StateFlags());

} // namespace Render
} // namespace BlossomUI

#endif

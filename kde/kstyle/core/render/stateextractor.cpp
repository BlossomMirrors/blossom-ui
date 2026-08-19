// SPDX-License-Identifier: GPL-2.0-or-later
#include "stateextractor.h"

#include "blossomuianimationdata.h"
#include "blossomuiwidgetstateengine.h"

#include <QStyleOption>
#include <QWidget>

#if BLOSSOMUI_HAVE_QTQUICK
#include <QCursor>
#include <QQuickItem>
#endif

namespace BlossomUI {
namespace Render {

QPointF ripplePosition(const QObject *styleObject, bool isQtQuick) {
  QPointF pos = styleObject
                    ? styleObject->property("blossomui-ripple-pos").toPointF()
                    : QPointF();
#if BLOSSOMUI_HAVE_QTQUICK
  if (pos.isNull() && isQtQuick && styleObject) {
    const auto *item = static_cast<const QQuickItem *>(styleObject);
    pos = item->mapFromGlobal(QCursor::pos());
  }
#else
  Q_UNUSED(isQtQuick)
#endif
  return pos;
}

WidgetInteractionState extractState(const QStyleOption *option, const QWidget *widget,
                                    bool isQtQuick, WidgetStateEngine &engine,
                                    AnimationModes transitions, StateFlags flags) {
  const QObject *styleObject = widget ? widget : option->styleObject;
  const QStyle::State &qstate(option->state);

  WidgetInteractionState state;
  state.enabled = qstate & QStyle::State_Enabled;
  state.hovered = state.enabled && (qstate & QStyle::State_MouseOver);
  state.pressed = qstate & (QStyle::State_On | QStyle::State_Sunken);

  if (flags.focusRule == FocusRule::IncludesSunken) {
    state.focused = state.enabled &&
                    (qstate & (QStyle::State_HasFocus | QStyle::State_Sunken));
  } else {
    state.focused = state.enabled && (qstate & QStyle::State_HasFocus) &&
                    !(widget && widget->focusProxy());
  }

  if (transitions & AnimationHover)
    engine.updateState(styleObject, AnimationHover, state.hovered);
  if (transitions & AnimationPressed)
    engine.updateState(styleObject, AnimationPressed, state.pressed,
                       AnimationForwardOnly | AnimationLongDuration);
  if (transitions & AnimationFocus)
    engine.updateState(styleObject, AnimationFocus, state.focused && !state.hovered);

  state.mode = engine.buttonAnimationMode(styleObject);
  state.opacity = engine.buttonOpacity(styleObject);
  state.pressOpacity = engine.opacity(styleObject, AnimationPressed);
  state.ripplePos = ripplePosition(styleObject, isQtQuick);
  state.palette = option->palette;

  return state;
}

} // namespace Render
} // namespace BlossomUI

#pragma once

#include <QPainter>
#include <QStyleOption>

namespace BlossomUI {

// Cast a QStyleOption to a specific subtype and return true (no-op) if the cast
// fails. Only usable inside Style draw methods that return bool.
#define CAST_OPTION(TYPE, VAR)                                                 \
  const auto VAR(qstyleoption_cast<const TYPE *>(option));                     \
  if (!VAR)                                                                    \
  return true

// Common widget state flags extracted from a QStyleOption.
// Covers the pattern that appears in nearly every draw method.
struct StyleState {
  bool enabled = false;
  bool mouseOver = false;
  bool hasFocus = false;
  bool sunken = false;
  bool windowActive = false;

  static StyleState from(const QStyleOption *option,
                         const QWidget *widget = nullptr) {
    const auto &state = option->state;
    const bool enabled = state & QStyle::State_Enabled;
    return {
        enabled,
        enabled && bool(state & QStyle::State_MouseOver),
        enabled && bool(state & QStyle::State_HasFocus),
        bool(state & (QStyle::State_On | QStyle::State_Sunken)),
        widget && widget->isActiveWindow(),
    };
  }

  // Variant for input widgets: mouseOver/hasFocus only count when isInputWidget
  // is true.
  static StyleState fromInput(const QStyleOption *option, const QWidget *widget,
                              bool isInputWidget) {
    const auto &state = option->state;
    const bool enabled = state & QStyle::State_Enabled;
    return {
        enabled,
        enabled && isInputWidget && bool(state & QStyle::State_MouseOver),
        enabled && isInputWidget && bool(state & QStyle::State_HasFocus),
        bool(state & (QStyle::State_On | QStyle::State_Sunken)),
        widget && widget->isActiveWindow(),
    };
  }
};

// RAII wrapper for QPainter::save/restore.
// Guarantees restore() even if an early return fires mid-method.
struct PainterGuard {
  explicit PainterGuard(QPainter *painter) : _painter(painter) {
    _painter->save();
  }
  ~PainterGuard() { _painter->restore(); }

  PainterGuard(const PainterGuard &) = delete;
  PainterGuard &operator=(const PainterGuard &) = delete;

private:
  QPainter *_painter;
};

} // namespace BlossomUI

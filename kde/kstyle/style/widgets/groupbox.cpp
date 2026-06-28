// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"

#include <KColorUtils>
#include <QApplication>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGroupBox>

namespace BlossomUI {

bool Style::drawGroupBoxComplexControl(const QStyleOptionComplex *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // base class method
  ParentStyleClass::drawComplexControl(CC_GroupBox, option, painter, widget);

  // cast option and check
  const auto groupBoxOption =
      qstyleoption_cast<const QStyleOptionGroupBox *>(option);
  if (!groupBoxOption)
    return true;

  // do nothing if either label is not selected or groupbox is empty
  if (!(option->subControls & QStyle::SC_GroupBoxLabel) ||
      groupBoxOption->text.isEmpty()) {
    return true;
  }

  // store palette and rect
  const auto &palette(option->palette);

  // check focus state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool hasFocus(enabled && (option->state & State_HasFocus));
  if (!hasFocus)
    return true;

  // alignment
  const int textFlags(groupBoxOption->textAlignment | _mnemonics->textFlags());

  // update animation state
  _animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                               hasFocus);
  const bool isFocusAnimated(
      _animations->widgetStateEngine().isAnimated(widget, AnimationFocus));
  const qreal opacity(
      _animations->widgetStateEngine().opacity(widget, AnimationFocus));

  // get relevant rect
  auto textRect = subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
  textRect = option->fontMetrics.boundingRect(textRect, textFlags,
                                              groupBoxOption->text);

  // focus color
  QColor focusColor;
  if (isFocusAnimated)
    focusColor = _helper->alphaColor(_helper->focusColor(palette), opacity);
  else if (hasFocus)
    focusColor = _helper->focusColor(palette);

  // render focus
  _helper->renderFocusLine(painter, textRect, focusColor);

  return true;
}

} // namespace BlossomUI

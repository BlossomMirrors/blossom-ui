// SPDX-License-Identifier: GPL-2.0-or-later
#include "groupboxcontrol.h"
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "frame.h"
#include "widgetrenderer.h"

#include <KColorUtils>
#include <QApplication>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGroupBox>

namespace BlossomUI {

bool Render::GroupBoxControl::drawGroupBoxComplexControl(const QStyleOptionComplex *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // base class method
  _style->ParentStyleClass::drawComplexControl(QStyle::CC_GroupBox, option, painter, widget);

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
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool hasFocus(enabled && (option->state & QStyle::State_HasFocus));
  if (!hasFocus)
    return true;

  // alignment
  const int textFlags(groupBoxOption->textAlignment | _style->_mnemonics->textFlags());

  // update animation state
  _style->_animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                               hasFocus);
  const bool isFocusAnimated(
      _style->_animations->widgetStateEngine().isAnimated(widget, AnimationFocus));
  const qreal opacity(
      _style->_animations->widgetStateEngine().opacity(widget, AnimationFocus));

  // get relevant rect
  auto textRect = _style->subControlRect(QStyle::CC_GroupBox, option, QStyle::SC_GroupBoxLabel, widget);
  textRect = option->fontMetrics.boundingRect(textRect, textFlags,
                                              groupBoxOption->text);

  // focus color
  QColor focusColor;
  if (isFocusAnimated)
    focusColor = _style->_helper->alphaColor(_style->_helper->focusColor(palette), opacity);
  else if (hasFocus)
    focusColor = _style->_helper->focusColor(palette);

  // render focus
  _style->_helper->renderFocusLine(painter, textRect, focusColor);

  return true;
}

void Helper::renderGroupBox(QPainter *painter, const QRect &rect,
                            const QColor &color, const bool mouseOver) const {
  Q_UNUSED(mouseOver)

  Render::WidgetSpec spec = Render::PlainFrameSpec;
  spec.geom.fixedRadius(frameRadius(PenWidth::NoPen, -1) + spec.geom.inset);
  spec.fill(Render::StateStyle<Render::Fill>(Render::Fill(color)));
  spec.border(Render::plainFrameBorder(QApplication::palette()));

  Render::WidgetInteractionState wstate;
  wstate.enabled = true;
  wstate.palette = QApplication::palette();

  Render::WidgetRenderer(this).render(painter, rect, spec, wstate);
}

//* checkable group boxes: enable hover
bool Style::polishCheckableGroupBox(QWidget *widget) {
  auto groupBox = qobject_cast<QGroupBox *>(widget);
  if (!groupBox)
    return false;
  if (groupBox->isCheckable()) {
    groupBox->setAttribute(Qt::WA_Hover);
  }
  return true;
}
bool Style::drawGroupBoxComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::GroupBoxControl(this).drawGroupBoxComplexControl(option, painter, widget);
}
} // namespace BlossomUI

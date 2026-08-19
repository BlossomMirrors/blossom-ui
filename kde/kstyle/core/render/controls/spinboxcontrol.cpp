// SPDX-License-Identifier: GPL-2.0-or-later
#include "spinboxcontrol.h"
#include "blossomuistyle.h"
#include "frame.h"
#include "spinbox.h"

#include <KColorUtils>
#include <QApplication>
#include <QPainter>
#include <QSpinBox>
#include <QStyleOptionSpinBox>

namespace BlossomUI {

bool Render::SpinBoxControl::drawSpinBoxComplexControl(const QStyleOptionComplex *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  const auto spinBoxOption(
      qstyleoption_cast<const QStyleOptionSpinBox *>(option));
  if (!spinBoxOption)
    return true;

  // store palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  if (option->subControls & QStyle::SC_SpinBoxFrame) {
    // detect flat spinboxes
    bool flat(!spinBoxOption->frame);
    flat |= (rect.height() <
             2 * Render::Frame_FrameWidth + Render::SpinBox_ArrowButtonWidth);
    if (flat) {
      const auto &background = palette.color(QPalette::Base);

      painter->setBrush(background);
      painter->setPen(Qt::NoPen);
      painter->drawRect(rect);

    } else {
      _style->drawPrimitive(QStyle::PE_FrameLineEdit, option, painter, widget);
    }
  }

  if (option->subControls & QStyle::SC_SpinBoxUp)
    _style->renderSpinBoxArrow(QStyle::SC_SpinBoxUp, spinBoxOption, painter, widget);
  if (option->subControls & QStyle::SC_SpinBoxDown)
    _style->renderSpinBoxArrow(QStyle::SC_SpinBoxDown, spinBoxOption, painter, widget);

  return true;
}
bool Style::drawSpinBoxComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::SpinBoxControl(this).drawSpinBoxComplexControl(option, painter, widget);
}
} // namespace BlossomUI

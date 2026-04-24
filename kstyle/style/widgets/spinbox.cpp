// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuistyle.h"

#include <KColorUtils>
#include <QApplication>
#include <QPainter>
#include <QSpinBox>
#include <QStyleOptionSpinBox>

namespace BlossomUI {

bool Style::drawSpinBoxComplexControl(const QStyleOptionComplex *option,
                                      QPainter *painter,
                                      const QWidget *widget) const {
  const auto spinBoxOption(
      qstyleoption_cast<const QStyleOptionSpinBox *>(option));
  if (!spinBoxOption)
    return true;

  // store palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  if (option->subControls & SC_SpinBoxFrame) {
    // detect flat spinboxes
    bool flat(!spinBoxOption->frame);
    flat |= (rect.height() <
             2 * Metrics::Frame_FrameWidth + Metrics::SpinBox_ArrowButtonWidth);
    if (flat) {
      const auto &background = palette.color(QPalette::Base);

      painter->setBrush(background);
      painter->setPen(Qt::NoPen);
      painter->drawRect(rect);

    } else {
      drawPrimitive(PE_FrameLineEdit, option, painter, widget);
    }
  }

  if (option->subControls & SC_SpinBoxUp)
    renderSpinBoxArrow(SC_SpinBoxUp, spinBoxOption, painter, widget);
  if (option->subControls & SC_SpinBoxDown)
    renderSpinBoxArrow(SC_SpinBoxDown, spinBoxOption, painter, widget);

  return true;
}

} // namespace BlossomUI

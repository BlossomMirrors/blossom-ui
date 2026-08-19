// SPDX-License-Identifier: GPL-2.0-or-later
#include "tooltipcontrol.h"
#include "blossomuishadowhelper.h"
#include "blossomuistyle.h"

#include <QPainter>

namespace BlossomUI {

bool Render::ToolTipControl::drawPanelTipLabelPrimitive(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // force registration of widget
  if (widget && widget->window()) {
    _style->_shadowHelper->registerWidget(widget->window(), true);
  }

  const auto &palette(option->palette);
  const auto &background = palette.color(QPalette::ToolTipBase);
  // const auto outline( KColorUtils::mix( palette.color( QPalette::ToolTipBase
  // ), palette.color( QPalette::ToolTipText ), 0.25 ) );
  const bool hasAlpha(_style->_helper->hasAlphaChannel(widget));

  _style->_helper->renderMenuFrame(painter, option->rect, background, QColor(),
                           hasAlpha);
  return true;
}

//* QTipLabel: translucent background
bool Style::polishTipLabel(QWidget *widget) {
  if (!widget->inherits("QTipLabel"))
    return false;
  setTranslucentBackground(widget);
  return true;
}
bool Style::drawPanelTipLabelPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ToolTipControl(this).drawPanelTipLabelPrimitive(option, painter, widget);
}
} // namespace BlossomUI

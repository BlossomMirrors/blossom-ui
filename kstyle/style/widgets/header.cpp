// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "private.h"

#include <KColorUtils>
#include <QPainter>
#include <QStyleOptionHeader>

namespace BlossomUI {

bool Style::drawHeaderSectionControl(const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const {
  const auto &rect(option->rect);
  const auto &palette(option->palette);
  const auto &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool sunken(enabled && (state & (State_On | State_Sunken)));

  const auto headerOption(
      qstyleoption_cast<const QStyleOptionHeader *>(option));
  if (!headerOption)
    return true;

  const bool horizontal(headerOption->orientation == Qt::Horizontal);
  const bool isFirst(
      horizontal &&
      (headerOption->position == QStyleOptionHeader::Beginning ||
       headerOption->position == QStyleOptionHeader::OnlyOneSection));
  const bool isLast(
      horizontal &&
      (headerOption->position == QStyleOptionHeader::End ||
       headerOption->position == QStyleOptionHeader::OnlyOneSection));
  const bool isCorner(widget && widget->inherits("QTableCornerButton"));
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // update animation state
  _animations->headerViewEngine().updateState(widget, rect.topLeft(),
                                              mouseOver);
  const bool animated(enabled && _animations->headerViewEngine().isAnimated(
                                     widget, rect.topLeft()));
  const qreal opacity(
      _animations->headerViewEngine().opacity(widget, rect.topLeft()));

  // fill
  const auto &normal = palette.color(QPalette::Button);
  const auto focus(KColorUtils::mix(normal, _helper->focusColor(palette), 0.2));
  const auto hover(KColorUtils::mix(normal, _helper->hoverColor(palette), 0.2));

  QColor color;
  if (sunken)
    color = focus;
  else if (animated)
    color = KColorUtils::mix(normal, hover, opacity);
  else if (mouseOver)
    color = hover;
  else
    color = normal;

  // Table topbar: round top-left and top-right with same radius for consistent
  // corners
  const int radius = 6;
  Corners corners = Corners();
  if (horizontal) {
    if (isFirst && isLast)
      corners = CornersTop;
    else if (isFirst)
      corners = reverseLayout ? CornerTopRight : CornerTopLeft;
    else if (isLast)
      corners = reverseLayout ? CornerTopLeft : CornerTopRight;
  }
  if (corners) {
    _helper->renderSelection(painter, rect, color, corners, radius);
  } else {
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
  }

  // outline - skip bottom line for Dolphin table header so header and content
  // blend (no visible gap)
  painter->setBrush(Qt::NoBrush);
  painter->setPen(
      _helper->alphaColor(palette.color(QPalette::WindowText), 0.1));

  if (isCorner) {
    if (reverseLayout)
      painter->drawPoint(rect.bottomLeft());
    else
      painter->drawPoint(rect.bottomRight());

  } else if (horizontal && !_app.isDolphin) {
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

  } else if (!horizontal) {
    if (reverseLayout)
      painter->drawLine(rect.topLeft(), rect.bottomLeft());
    else
      painter->drawLine(rect.topRight(), rect.bottomRight());
  }

  // separators
  painter->setPen(
      _helper->alphaColor(palette.color(QPalette::WindowText), 0.2));

  if (horizontal) {
    if (headerOption->section != 0 || isFirst) {
      if (reverseLayout)
        painter->drawLine(rect.topLeft(), rect.bottomLeft() - QPoint(0, 1));
      else
        painter->drawLine(rect.topRight(), rect.bottomRight() - QPoint(0, 1));
    }

  } else {
    if (reverseLayout)
      painter->drawLine(rect.bottomLeft() + QPoint(1, 0), rect.bottomRight());
    else
      painter->drawLine(rect.bottomLeft(), rect.bottomRight() - QPoint(1, 0));
  }

  return true;
}

bool Style::drawHeaderEmptyAreaControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *) const {
  // use the same background as in drawHeaderPrimitive
  const auto &rect(option->rect);
  auto palette(option->palette);

  const bool horizontal(option->state & QStyle::State_Horizontal);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // fill
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setBrush(palette.color(QPalette::Button));
  painter->setPen(Qt::NoPen);
  painter->drawRect(rect);

  // outline
  painter->setBrush(Qt::NoBrush);
  painter->setPen(
      _helper->alphaColor(palette.color(QPalette::ButtonText), 0.1));

  if (horizontal) {
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

  } else {
    if (reverseLayout)
      painter->drawLine(rect.topLeft(), rect.bottomLeft());
    else
      painter->drawLine(rect.topRight(), rect.bottomRight());
  }

  return true;
}
} // namespace BlossomUI

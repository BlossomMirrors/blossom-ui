// SPDX-License-Identifier: GPL-2.0-or-later
#include "titlebarcontrol.h"
#include "blossomuistyle.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionTitleBar>

namespace BlossomUI {

bool Render::TitleBarControl::drawTitleBarComplexControl(const QStyleOptionComplex *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto titleBarOption(
      qstyleoption_cast<const QStyleOptionTitleBar *>(option));
  if (!titleBarOption)
    return true;

  const bool windowActive(widget && widget->isActiveWindow());

  // store palette and rect
  auto palette(option->palette);
  const auto &rect(option->rect);

  const QStyle::State &flags(option->state);
  const bool enabled(flags & QStyle::State_Enabled);
  const bool active(enabled &&
                    (titleBarOption->titleBarState & Qt::WindowActive));

  if (titleBarOption->subControls & QStyle::SC_TitleBarLabel) {
    // render background
    painter->setClipRect(rect);
    const auto background(_style->_helper->titleBarColor(active));
    _style->_helper->renderTabWidgetFrame(painter, rect.adjusted(-1, -1, 1, 3),
                                  background, CornersTop, windowActive);

    const bool useSeparator(
        active &&
        _style->_helper->titleBarColor(active) != palette.color(QPalette::Window) &&
        !(titleBarOption->titleBarState & Qt::WindowMinimized));

    if (useSeparator) {
      painter->setRenderHint(QPainter::Antialiasing, false);
      painter->setBrush(Qt::NoBrush);
      painter->setPen(palette.color(QPalette::Highlight));
      painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    }

    // render text
    palette.setColor(QPalette::WindowText, _style->_helper->titleBarTextColor(active));
    const auto textRect(
        _style->subControlRect(QStyle::CC_TitleBar, option, QStyle::SC_TitleBarLabel, widget));
    _style->ParentStyleClass::drawItemText(painter, textRect, Qt::AlignCenter, palette,
                                   active, titleBarOption->text,
                                   QPalette::WindowText);
  }

  // buttons
  static const QList<QStyle::SubControl> subControls = {
      QStyle::SC_TitleBarMinButton, QStyle::SC_TitleBarMaxButton, QStyle::SC_TitleBarCloseButton,
      QStyle::SC_TitleBarNormalButton, QStyle::SC_TitleBarSysMenu};

  // loop over supported buttons
  foreach (const QStyle::SubControl &subControl, subControls) {
    // skip if not requested
    if (!(titleBarOption->subControls & subControl))
      continue;

    // find matching icon
    QIcon icon;
    switch (subControl) {
    case QStyle::SC_TitleBarMinButton:
      icon = _style->standardIcon(QStyle::SP_TitleBarMinButton, option, widget);
      break;
    case QStyle::SC_TitleBarMaxButton:
      icon = _style->standardIcon(QStyle::SP_TitleBarMaxButton, option, widget);
      break;
    case QStyle::SC_TitleBarCloseButton:
      icon = _style->standardIcon(QStyle::SP_TitleBarCloseButton, option, widget);
      break;
    case QStyle::SC_TitleBarNormalButton:
      icon = _style->standardIcon(QStyle::SP_TitleBarNormalButton, option, widget);
      break;
    case QStyle::SC_TitleBarSysMenu:
      icon = titleBarOption->icon;
      break;
    default:
      break;
    }

    // check icon
    if (icon.isNull())
      continue;

    // define icon rect
    auto iconRect(_style->subControlRect(QStyle::CC_TitleBar, option, subControl, widget));
    if (iconRect.isEmpty())
      continue;

    // active state
    const bool subControlActive(titleBarOption->activeSubControls & subControl);

    // mouse over state
    const bool mouseOver(!subControlActive && widget &&
                         iconRect.translated(widget->mapToGlobal(QPoint(0, 0)))
                             .contains(QCursor::pos()));

    // adjust iconRect
    const int iconWidth(_style->pixelMetric(QStyle::PM_SmallIconSize, option, widget));
    const QSize iconSize(iconWidth, iconWidth);
    iconRect = _style->centerRect(iconRect, iconSize);

    // set icon mode and state
    QIcon::Mode iconMode;
    QIcon::State iconState;

    if (!enabled) {
      iconMode = QIcon::Disabled;
      iconState = QIcon::Off;

    } else {
      if (mouseOver)
        iconMode = QIcon::Active;
      else if (active)
        iconMode = QIcon::Selected;
      else
        iconMode = QIcon::Normal;

      iconState = subControlActive ? QIcon::On : QIcon::Off;
    }

    // get pixmap and render
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap = _style->_helper->coloredIcon(icon, option->palette, iconSize,
                                                dpr, iconMode, iconState);
    _style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  return true;
}

void Helper::renderDecorationButton(QPainter *painter, const QRect &rect,
                                    const QColor &color, ButtonType buttonType,
                                    bool inverted) const {
  painter->save();
  painter->setViewport(rect);
  painter->setWindow(0, 0, 18, 18);
  painter->setRenderHints(QPainter::Antialiasing);

  // initialize pen
  QPen pen;
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::MiterJoin);

  if (inverted) {
    // render circle
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(QRectF(0, 0, 18, 18));

    // take out the inner part
    painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter->setBrush(Qt::NoBrush);
    pen.setColor(Qt::black);

  } else {
    painter->setBrush(Qt::NoBrush);
    pen.setColor(color);
  }

  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::MiterJoin);
  pen.setWidthF(PenWidth::Symbol * qMax(1.0, 18.0 / rect.width()));
  painter->setPen(pen);

  switch (buttonType) {
  case ButtonClose: {
    painter->drawLine(QPointF(5, 5), QPointF(13, 13));
    painter->drawLine(13, 5, 5, 13);
    break;
  }

  case ButtonMaximize: {
    painter->drawPolyline(
        QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
    break;
  }

  case ButtonMinimize: {
    painter->drawPolyline(
        QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
    break;
  }

  case ButtonRestore: {
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5),
                                          QPointF(13.5, 9), QPointF(9, 13.5)});
    break;
  }

  default:
    break;
  }

  painter->restore();
}
bool Style::drawTitleBarComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::TitleBarControl(this).drawTitleBarComplexControl(option, painter, widget);
}
} // namespace BlossomUI

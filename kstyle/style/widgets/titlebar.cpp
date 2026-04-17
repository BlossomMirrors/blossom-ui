// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionTitleBar>

namespace BlossomUI {

bool Style::drawTitleBarComplexControl(const QStyleOptionComplex *option,
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

  const State &flags(option->state);
  const bool enabled(flags & State_Enabled);
  const bool active(enabled &&
                    (titleBarOption->titleBarState & Qt::WindowActive));

  if (titleBarOption->subControls & SC_TitleBarLabel) {
    // render background
    painter->setClipRect(rect);
    const auto background(_helper->titleBarColor(active));
    _helper->renderTabWidgetFrame(painter, rect.adjusted(-1, -1, 1, 3),
                                  background, CornersTop, windowActive);

    const bool useSeparator(
        active &&
        _helper->titleBarColor(active) != palette.color(QPalette::Window) &&
        !(titleBarOption->titleBarState & Qt::WindowMinimized));

    if (useSeparator) {
      painter->setRenderHint(QPainter::Antialiasing, false);
      painter->setBrush(Qt::NoBrush);
      painter->setPen(palette.color(QPalette::Highlight));
      painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    }

    // render text
    palette.setColor(QPalette::WindowText, _helper->titleBarTextColor(active));
    const auto textRect(
        subControlRect(CC_TitleBar, option, SC_TitleBarLabel, widget));
    ParentStyleClass::drawItemText(painter, textRect, Qt::AlignCenter, palette,
                                   active, titleBarOption->text,
                                   QPalette::WindowText);
  }

  // buttons
  static const QList<SubControl> subControls = {
      SC_TitleBarMinButton, SC_TitleBarMaxButton, SC_TitleBarCloseButton,
      SC_TitleBarNormalButton, SC_TitleBarSysMenu};

  // loop over supported buttons
  foreach (const SubControl &subControl, subControls) {
    // skip if not requested
    if (!(titleBarOption->subControls & subControl))
      continue;

    // find matching icon
    QIcon icon;
    switch (subControl) {
    case SC_TitleBarMinButton:
      icon = standardIcon(SP_TitleBarMinButton, option, widget);
      break;
    case SC_TitleBarMaxButton:
      icon = standardIcon(SP_TitleBarMaxButton, option, widget);
      break;
    case SC_TitleBarCloseButton:
      icon = standardIcon(SP_TitleBarCloseButton, option, widget);
      break;
    case SC_TitleBarNormalButton:
      icon = standardIcon(SP_TitleBarNormalButton, option, widget);
      break;
    case SC_TitleBarSysMenu:
      icon = titleBarOption->icon;
      break;
    default:
      break;
    }

    // check icon
    if (icon.isNull())
      continue;

    // define icon rect
    auto iconRect(subControlRect(CC_TitleBar, option, subControl, widget));
    if (iconRect.isEmpty())
      continue;

    // active state
    const bool subControlActive(titleBarOption->activeSubControls & subControl);

    // mouse over state
    const bool mouseOver(!subControlActive && widget &&
                         iconRect.translated(widget->mapToGlobal(QPoint(0, 0)))
                             .contains(QCursor::pos()));

    // adjust iconRect
    const int iconWidth(pixelMetric(PM_SmallIconSize, option, widget));
    const QSize iconSize(iconWidth, iconWidth);
    iconRect = centerRect(iconRect, iconSize);

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
    const QPixmap pixmap = _helper->coloredIcon(icon, option->palette, iconSize,
                                                dpr, iconMode, iconState);
    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  return true;
}

} // namespace BlossomUI

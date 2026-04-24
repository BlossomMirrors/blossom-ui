// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"

#include <KColorUtils>
#include <QAbstractButton>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QStyleOptionToolButton>
#include <QToolBar>
#include <QToolButton>

namespace BlossomUI {

bool Style::drawPushButtonLabelControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto buttonOption(
      qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return true;

  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool sunken(state & (State_On | State_Sunken));
  const bool mouseOver(enabled && (option->state & State_MouseOver));
  const bool hasFocus(enabled && (option->state & State_HasFocus));
  const bool flat(buttonOption->features & QStyleOptionButton::Flat);

  // content
  const bool hasText(!buttonOption->text.isEmpty());
  const bool hasIcon((showIconsOnPushButtons() || flat || !hasText) &&
                     !buttonOption->icon.isNull());

  // contents
  auto contentsRect(rect);
  if (sunken && !flat)
    contentsRect.translate(0, 1);
  // else if ( mouseOver && !flat ) contentsRect.translate(0, -1);

  // color role
  QPalette::ColorRole textRole;
  if (flat) {
    if (hasFocus && sunken)
      textRole = QPalette::HighlightedText;
    else
      textRole = QPalette::WindowText;

  } else
    textRole = QPalette::ButtonText;

  // menu arrow
  if (buttonOption->features & QStyleOptionButton::HasMenu) {
    // define rect
    auto arrowRect(contentsRect);
    arrowRect.setLeft(contentsRect.right() -
                      Metrics::MenuButton_IndicatorWidth + 1);
    arrowRect = centerRect(arrowRect, Metrics::MenuButton_IndicatorWidth,
                           Metrics::MenuButton_IndicatorWidth);

    contentsRect.setRight(arrowRect.left() - Metrics::Button_ItemSpacing - 1);
    contentsRect.adjust(Metrics::Button_MarginWidth, 0, 0, 0);

    arrowRect = visualRect(option, arrowRect);

    // define color
    const auto arrowColor(_helper->arrowColor(palette, textRole));
    _helper->renderArrow(painter, arrowRect, arrowColor, ArrowDown);
  }

  // icon size
  QSize iconSize;
  if (hasIcon) {
    iconSize = buttonOption->iconSize;
    if (!iconSize.isValid()) {
      const int metric(pixelMetric(PM_SmallIconSize, option, widget));
      iconSize = QSize(metric, metric);
    }
  }

  // text size
  const int textFlags(_mnemonics->textFlags() | Qt::AlignCenter);
  const QSize textSize(option->fontMetrics.size(textFlags, buttonOption->text));

  // adjust text and icon rect based on options
  QRect iconRect;
  QRect textRect;

  if (hasText && !hasIcon)
    textRect = contentsRect;
  else if (hasIcon && !hasText)
    iconRect = contentsRect;
  else {
    const int contentsWidth(iconSize.width() + textSize.width() +
                            Metrics::Button_ItemSpacing);
    iconRect = QRect(
        QPoint(contentsRect.left() + (contentsRect.width() - contentsWidth) / 2,
               contentsRect.top() +
                   qRound((contentsRect.height() - iconSize.height()) / 2.0)),
        iconSize);
    textRect = QRect(
        QPoint(iconRect.right() + Metrics::ToolButton_ItemSpacing + 1,
               contentsRect.top() +
                   qRound((contentsRect.height() - textSize.height()) / 2.0)),
        textSize);
  }

  // handle right to left
  if (iconRect.isValid())
    iconRect = visualRect(option, iconRect);
  if (textRect.isValid())
    textRect = visualRect(option, textRect);

  // make sure there is enough room for icon
  if (iconRect.isValid())
    iconRect = centerRect(iconRect, iconSize);

  // render icon
  if (hasIcon && iconRect.isValid()) {
    // icon state and mode
    const QIcon::State iconState(sunken ? QIcon::On : QIcon::Off);
    QIcon::Mode iconMode;
    if (!enabled)
      iconMode = QIcon::Disabled;
    else if (!flat && hasFocus && !sunken)
      iconMode = QIcon::Selected;
    else if (mouseOver && flat)
      iconMode = QIcon::Active;
    else
      iconMode = QIcon::Normal;

    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const auto pixmap =
        _helper->coloredIcon(buttonOption->icon, buttonOption->palette,
                             iconSize, dpr, iconMode, iconState);
    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  // render text
  if (hasText && textRect.isValid()) {
    drawItemText(painter, textRect, textFlags, palette, enabled,
                 buttonOption->text, textRole);
  }

  return true;
}

bool Style::drawToolButtonLabelControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));

  // copy rect and palette
  const auto &rect = option->rect;
  const auto &palette = option->palette;

  // state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool sunken(state & (State_On | State_Sunken));
  const bool mouseOver(enabled && (option->state & State_MouseOver));
  const bool flat(state & State_AutoRaise);

  // focus flag is set to match the background color in either renderButtonFrame
  // or renderToolButtonFrame
  bool hasFocus(false);
  if (flat)
    hasFocus = enabled && !mouseOver && (option->state & State_HasFocus);
  else
    hasFocus = enabled && !mouseOver &&
               (option->state & (State_HasFocus | State_Sunken));

  const bool hasArrow(toolButtonOption->features &
                      QStyleOptionToolButton::Arrow);
  const bool hasIcon(!(hasArrow || toolButtonOption->icon.isNull()));
  const bool hasText(!toolButtonOption->text.isEmpty());

  // contents
  auto contentsRect(rect);
  if (sunken && !flat)
    contentsRect.translate(1, 1);

  // icon size
  const QSize iconSize(toolButtonOption->iconSize);

  // text size
  int textFlags(_mnemonics->textFlags());
  const QSize textSize(
      option->fontMetrics.size(textFlags, toolButtonOption->text));

  // adjust text and icon rect based on options
  QRect iconRect;
  QRect textRect;

  if (hasText && (!(hasArrow || hasIcon) || toolButtonOption->toolButtonStyle ==
                                                Qt::ToolButtonTextOnly)) {
    // text only
    textRect = contentsRect;
    textFlags |= Qt::AlignCenter;

  } else if ((hasArrow || hasIcon) &&
             (!hasText ||
              toolButtonOption->toolButtonStyle == Qt::ToolButtonIconOnly)) {
    // icon only
    iconRect = contentsRect;

  } else if (toolButtonOption->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
    const int contentsHeight(iconSize.height() + textSize.height() +
                             Metrics::ToolButton_ItemSpacing);
    iconRect = QRect(
        QPoint(contentsRect.left() +
                   qRound((contentsRect.width() - iconSize.width()) / 2.0),
               contentsRect.top() +
                   qRound((contentsRect.height() - contentsHeight) / 2.0)),
        iconSize);
    textRect = QRect(
        QPoint(contentsRect.left() +
                   qRound((contentsRect.width() - textSize.width()) / 2.0),
               iconRect.bottom() + Metrics::ToolButton_ItemSpacing + 1),
        textSize);
    textFlags |= Qt::AlignCenter;

  } else {
    const bool leftAlign(
        widget &&
        widget->property(PropertyNames::toolButtonAlignment).toInt() ==
            Qt::AlignLeft);
    if (leftAlign) {
      const int marginWidth(Metrics::Button_MarginWidth +
                            Metrics::Frame_FrameWidth + 1);
      iconRect =
          QRect(QPoint(contentsRect.left() + marginWidth,
                       contentsRect.top() +
                           (contentsRect.height() - iconSize.height()) / 2),
                iconSize);
    } else {
      const int contentsWidth(iconSize.width() + textSize.width() +
                              Metrics::ToolButton_ItemSpacing);
      iconRect = QRect(
          QPoint(contentsRect.left() +
                     qRound((contentsRect.width() - contentsWidth) / 2.0),
                 contentsRect.top() +
                     qRound((contentsRect.height() - iconSize.height()) / 2.0)),
          iconSize);
    }

    textRect =
        QRect(QPoint(iconRect.right() + Metrics::ToolButton_ItemSpacing + 1,
                     contentsRect.top() +
                         (contentsRect.height() - textSize.height()) / 2),
              textSize);

    // handle right to left layouts
    iconRect = visualRect(option, iconRect);
    textRect = visualRect(option, textRect);

    textFlags |= Qt::AlignLeft | Qt::AlignVCenter;
  }

  // make sure there is enough room for icon
  if (iconRect.isValid())
    iconRect = centerRect(iconRect, iconSize);

  // render arrow or icon
  if (hasArrow && iconRect.isValid()) {
    QStyleOptionToolButton copy(*toolButtonOption);
    copy.rect = iconRect;
    switch (toolButtonOption->arrowType) {
    case Qt::LeftArrow:
      drawPrimitive(PE_IndicatorArrowLeft, &copy, painter, widget);
      break;
    case Qt::RightArrow:
      drawPrimitive(PE_IndicatorArrowRight, &copy, painter, widget);
      break;
    case Qt::UpArrow:
      drawPrimitive(PE_IndicatorArrowUp, &copy, painter, widget);
      break;
    case Qt::DownArrow:
      drawPrimitive(PE_IndicatorArrowDown, &copy, painter, widget);
      break;
    default:
      break;
    }

  } else if (hasIcon && iconRect.isValid()) {
    // icon state and mode
    const QIcon::State iconState(sunken ? QIcon::On : QIcon::Off);
    QIcon::Mode iconMode;
    if (!enabled)
      iconMode = QIcon::Disabled;
    else if ((!flat && (hasFocus || sunken)) ||
             (flat && (state & State_Sunken) && !mouseOver))
      iconMode = QIcon::Selected;
    else
      iconMode = QIcon::Normal;

    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap =
        _helper->coloredIcon(toolButtonOption->icon, toolButtonOption->palette,
                             iconSize, dpr, iconMode, iconState);
    drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
  }

  // render text
  if (hasText && textRect.isValid()) {
    QPalette::ColorRole textRole(QPalette::ButtonText);
    if (flat)
      textRole =
          (((hasFocus && sunken) || (state & State_Sunken)) && !mouseOver)
              ? QPalette::HighlightedText
              : QPalette::WindowText;
    else if (hasFocus || sunken)
      textRole = QPalette::HighlightedText;

    painter->setFont(toolButtonOption->font);
    drawItemText(painter, textRect, textFlags, palette, enabled,
                 toolButtonOption->text, textRole);
  }

  return true;
}
bool Style::drawToolButtonComplexControl(const QStyleOptionComplex *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  // cast option and check
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));
  if (!toolButtonOption)
    return true;

  // need to alter palette for focused buttons
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (option->state & State_MouseOver));
  const bool hasFocus(enabled && (option->state & State_HasFocus));
  const bool sunken(state & (State_On | State_Sunken));
  const bool flat(state & State_AutoRaise);

  // update animation state
  // mouse over takes precedence over focus
  _animations->widgetStateEngine().updateState(widget, AnimationHover,
                                               mouseOver);
  _animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                               hasFocus && !mouseOver);

  // detect buttons in tabbar, for which special rendering is needed
  const bool inTabBar(widget &&
                      qobject_cast<const QTabBar *>(widget->parentWidget()));
  const bool isMenuTitle(this->isMenuTitle(widget));
  if (isMenuTitle) {
    // copy option to adjust state, and set font as not-bold
    QStyleOptionToolButton copy(*toolButtonOption);
    copy.font.setBold(false);
    copy.state = State_Enabled;

    // render
    renderMenuTitle(&copy, painter, widget);
    return true;
  }

  // copy option and alter palette
  QStyleOptionToolButton copy(*toolButtonOption);

  const bool hasPopupMenu(toolButtonOption->features &
                          QStyleOptionToolButton::MenuButtonPopup);
  const bool hasInlineIndicator(
      toolButtonOption->features & QStyleOptionToolButton::HasMenu &&
      toolButtonOption->features & QStyleOptionToolButton::PopupDelay &&
      !hasPopupMenu);

  const auto buttonRect(
      subControlRect(CC_ToolButton, option, SC_ToolButton, widget));
  const auto menuRect(
      subControlRect(CC_ToolButton, option, SC_ToolButtonMenu, widget));

  // Dolphin back/forward buttons: skip drawing the dropdown chevron (history
  // menu indicator). Dolphin uses icons for the main chevron; detect by
  // objectName (widget or its action, from KXmlGui).
  bool isDolphinNavButton = false;
  if (_app.isDolphin && (hasPopupMenu || hasInlineIndicator) && widget) {
    const QString name(widget->objectName());
    if (name == QLatin1String("go_back") || name == QLatin1String("go_forward"))
      isDolphinNavButton = true;
    else if (const auto *toolButton =
                 qobject_cast<const QToolButton *>(widget)) {
      if (const QAction *action = toolButton->defaultAction()) {
        const QString actionName(action->objectName());
        isDolphinNavButton = (actionName == QLatin1String("go_back") ||
                              actionName == QLatin1String("go_forward"));
      }
    }
  }

  // frame
  if (toolButtonOption->subControls & SC_ToolButton) {
    if (!flat)
      copy.rect = buttonRect;
    if (inTabBar)
      drawTabBarPanelButtonToolPrimitive(&copy, painter, widget);
    else
      drawPrimitive(PE_PanelButtonTool, &copy, painter, widget);
  }

  // arrow (skip dropdown chevron for Dolphin back/forward buttons)
  if (!isDolphinNavButton) {
    if (hasPopupMenu) {
      copy.rect = menuRect;
      if (!flat)
        drawPrimitive(PE_IndicatorButtonDropDown, &copy, painter, widget);

      if (sunken && !flat)
        copy.rect.translate(1, 1);
      if (flat)
        copy.rect.translate(-Metrics::MenuButton_IndicatorWidth / 4, 0);
      drawPrimitive(PE_IndicatorArrowDown, &copy, painter, widget);

    } else if (hasInlineIndicator) {
      copy.rect = menuRect;

      if (sunken && !flat)
        copy.rect.translate(1, 1);
      drawIndicatorArrowPrimitive(ArrowDownSmall, &copy, painter, widget);
    }
  }

  // contents
  {
    // restore state
    copy.state = state;

    // define contents rect
    auto contentsRect(buttonRect);

    // detect dock widget title button
    // for dockwidget title buttons, do not take out margins, so that icon do
    // not get scaled down
    const bool isDockWidgetTitleButton(
        widget && widget->inherits("QDockWidgetTitleButton"));
    if (isDockWidgetTitleButton) {
      // cast to abstract button
      // adjust state to have correct icon rendered
      const auto button(qobject_cast<const QAbstractButton *>(widget));
      if (button->isChecked() || button->isDown())
        copy.state |= State_On;

    } else if (!inTabBar && hasInlineIndicator) {
      const int marginWidth(flat ? Metrics::ToolButton_MarginWidth
                                 : Metrics::Button_MarginWidth +
                                       Metrics::Frame_FrameWidth);
      contentsRect = insideMargin(contentsRect, marginWidth, 0);
      contentsRect = visualRect(option, contentsRect);
    }

    copy.rect = contentsRect;

    // render
    drawControl(CE_ToolButtonLabel, &copy, painter, widget);
  }

  return true;
}

} // namespace BlossomUI

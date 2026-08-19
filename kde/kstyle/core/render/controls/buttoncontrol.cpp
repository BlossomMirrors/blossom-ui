// SPDX-License-Identifier: GPL-2.0-or-later
#include "buttoncontrol.h"

#include "blossomuianimations.h"
#include "blossomuihelper.h"
#include "blossomuimnemonics.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "button.h"
#include "compositerenderer.h"
#include "tabbar.h"
#include "stateextractor.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QCommandLinkButton>
#include <QPainter>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#if BLOSSOMUI_HAVE_QTQUICK
#include <QQuickItem>
#endif

namespace BlossomUI {
namespace Render {

static void applyCursor(const QObject *styleObject, bool isQtQuick, bool enabled,
                        const Cursor &cursor) {
#if BLOSSOMUI_HAVE_QTQUICK
  if (!isQtQuick || !styleObject)
    return;
  auto *item = const_cast<QQuickItem *>(static_cast<const QQuickItem *>(styleObject));
  if (enabled && cursor.hasEnabled)
    item->setCursor(cursor.enabledShape);
  else if (!enabled && cursor.hasDisabled)
    item->setCursor(cursor.disabledShape);
  else
    item->unsetCursor();
#else
  Q_UNUSED(styleObject)
  Q_UNUSED(isQtQuick)
  Q_UNUSED(enabled)
  Q_UNUSED(cursor)
#endif
}

bool ButtonControl::drawPanelCommand(const QStyleOption *option, QPainter *painter,
                                     const QWidget *widget) const {
  const auto buttonOption(qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return true;

  const bool isQtQuick = _style->isQtQuickControl(option, widget);
  const bool flat(buttonOption->features & QStyleOptionButton::Flat);
  const auto state = extractState(option, widget, isQtQuick,
                                  _style->_animations->widgetStateEngine(),
                                  AnimationHover | AnimationPressed);

  const WidgetSpec spec =
      flat ? toolButtonFrame(_style->_helper, state)
           : buttonFrame(_style->_helper, state,
                         buttonOption->features & QStyleOptionButton::DefaultButton);

  applyCursor(widget ? widget : option->styleObject, isQtQuick, state.enabled,
              spec.cursorStyle);
  WidgetRenderer(_style->_helper).render(painter, option->rect, spec, state);
  return true;
}

bool ButtonControl::drawPanelTool(const QStyleOption *option, QPainter *painter,
                                  const QWidget *widget) const {
  auto rect(option->rect);
  const bool isQtQuick = _style->isQtQuickControl(option, widget);
  const bool autoRaise(option->state & QStyle::State_AutoRaise);

  // hover/focus already updated by drawToolComplex
  const auto state = extractState(option, widget, isQtQuick,
                                  _style->_animations->widgetStateEngine(),
                                  AnimationPressed, {FocusRule::IncludesSunken});

  if (!autoRaise) {
    const auto toolButton(qobject_cast<const QToolButton *>(widget));
    const bool hasPopupMenu(toolButton &&
                            toolButton->popupMode() == QToolButton::MenuButtonPopup);
    if (hasPopupMenu) {
      painter->setClipRect(rect);
      rect.adjust(0, 0, StyleConfigData::cornerRadius() + 2, 0);
      rect = _style->visualRect(option, rect);
    }

    const WidgetSpec spec = buttonFrame(_style->_helper, state);
    applyCursor(widget ? widget : option->styleObject, isQtQuick, state.enabled,
                spec.cursorStyle);
    WidgetRenderer(_style->_helper).render(painter, rect, spec, state);
    return true;
  }

  if (widget && widget->parentWidget()) {
    const QWidget *p = widget->parentWidget();
    if (qobject_cast<const QAbstractItemView *>(p) ||
        qobject_cast<const QAbstractItemView *>(p->parentWidget()))
      return true;
  }

  QRect hoverRect = rect;
  const auto tbOption = qstyleoption_cast<const QStyleOptionToolButton *>(option);
  if (tbOption && (tbOption->features & QStyleOptionToolButton::MenuButtonPopup)) {
    hoverRect.adjust(0, 0, 1, 0);
  } else if (tbOption && tbOption->toolButtonStyle == Qt::ToolButtonIconOnly &&
             rect.width() != rect.height()) {
    const int size = qMin(rect.width(), rect.height());
    hoverRect = QRect(rect.x() + (rect.width() - size) / 2,
                      rect.y() + (rect.height() - size) / 2, size, size);
  }

  const WidgetSpec spec = toolButtonFrame(_style->_helper, state);
  applyCursor(widget ? widget : option->styleObject, isQtQuick, state.enabled,
              spec.cursorStyle);
  WidgetRenderer(_style->_helper).render(painter, hoverRect, spec, state);
  return true;
}

bool ButtonControl::drawTabBarPanelTool(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  auto rect(option->rect);
  const QTabBar *tabBar(static_cast<QTabBar *>(widget->parentWidget()));
  const int overlap(Render::TabBar_BaseOverlap - 1);

  switch (tabBar->shape()) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:
    rect.adjust(0, 0, 0, -overlap);
    break;
  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    rect.adjust(0, overlap, 0, 0);
    break;
  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    rect.adjust(0, 0, -overlap, 0);
    break;
  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    rect.adjust(overlap, 0, 0, 0);
    break;
  default:
    break;
  }

  const QWidget *parent(tabBar->parentWidget());
  if (qobject_cast<const QTabWidget *>(parent))
    parent = parent->parentWidget();
  const auto &palette(parent ? parent->palette() : QApplication::palette());
  QColor color = _style->hasAlteredBackground(parent)
                     ? _style->_helper->frameBackgroundColor(palette)
                     : palette.color(QPalette::Window);

  if ((_style->_app.isDolphin || _style->_app.isKonsole) &&
      StyleConfigData::tabBarOpacity() < 100 && !_style->_app.isOpaque) {
    color = _style->_helper->transparentBarBgColor(
        widget->palette().color(QPalette::Window), painter, rect, BarType::TabBar);
  }

  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawRect(rect);
  return true;
}

bool ButtonControl::drawDropDownIndicator(const QStyleOption *option,
                                          QPainter *painter,
                                          const QWidget *widget) const {
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));
  if (!toolButtonOption)
    return true;
  if ((option->state & QStyle::State_AutoRaise) ||
      !(toolButtonOption->subControls & QStyle::SC_ToolButtonMenu))
    return true;

  const auto state = extractState(option, widget, false,
                                  _style->_animations->widgetStateEngine(),
                                  AnimationHover | AnimationFocus,
                                  {FocusRule::IncludesSunken});

  auto frameRect(option->rect);
  painter->setClipRect(option->rect);
  frameRect.adjust(-StyleConfigData::cornerRadius() - 1, 0, 0, 0);
  frameRect = _style->visualRect(option, frameRect);

  WidgetRenderer(_style->_helper)
      .render(painter, frameRect, buttonFrame(_style->_helper, state), state);
  return true;
}

bool ButtonControl::drawPushLabel(const QStyleOption *option, QPainter *painter,
                                  const QWidget *widget) const {
  const auto buttonOption(qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return true;

  const QStyle::State &qstate(option->state);
  const bool enabled(qstate & QStyle::State_Enabled);
  const bool sunken(qstate & (QStyle::State_On | QStyle::State_Sunken));
  const bool mouseOver(enabled && (qstate & QStyle::State_MouseOver));
  const bool hasFocus(enabled && (qstate & QStyle::State_HasFocus));
  const bool flat(buttonOption->features & QStyleOptionButton::Flat);
  const bool hasText(!buttonOption->text.isEmpty());
  const bool hasIcon((_style->showIconsOnPushButtons() || flat || !hasText) &&
                     !buttonOption->icon.isNull());

  WidgetInteractionState state;
  state.enabled = enabled;
  state.pressed = sunken;
  state.hovered = mouseOver;
  state.focused = hasFocus;
  state.palette = option->palette;
  state.pressOpacity = _style->_animations->widgetStateEngine().opacity(
      widget ? widget : option->styleObject, AnimationPressed);

  Content content;
  content.text = hasText ? buttonOption->text : QString();
  content.icon = hasIcon ? buttonOption->icon : QIcon();
  content.font = painter->font();
  content.iconState = sunken ? QIcon::On : QIcon::Off;
  content.iconMode = buttonIconMode(enabled, flat, hasFocus, sunken, mouseOver);
  content.textRole = buttonTextRole(flat, hasFocus, sunken);
  content.textFlags = _style->_mnemonics->textFlags() | Qt::AlignCenter;

  if (hasIcon) {
    content.iconState = sunken ? QIcon::On : QIcon::Off;
    QSize iconSize = buttonOption->iconSize;
    if (!iconSize.isValid()) {
      const int metric(_style->pixelMetric(QStyle::PM_SmallIconSize, option, widget));
      iconSize = QSize(metric, metric);
    }
    content.icon = buttonOption->icon;
    ContentLayout layout = buttonContent(Render::Button_ItemSpacing);
    layout.iconSize = iconSize;

    QRect bounds(option->rect);
    if (buttonOption->features & QStyleOptionButton::HasMenu)
      bounds = menuArrowSplit(option, painter, bounds, content.textRole);

    QVector<Part> parts{Part::fromContent(
        layout, content, Placement::fill(),
        flat ? Motion() : ButtonSpec.motionStyle)};
    CompositeRenderer(_style->_helper).render(painter, bounds, parts, state);
    return true;
  }

  ContentLayout layout = buttonContent(Render::Button_ItemSpacing);
  layout.arrangement = ContentArrangement::TextOnly;

  QRect bounds(option->rect);
  if (buttonOption->features & QStyleOptionButton::HasMenu)
    bounds = menuArrowSplit(option, painter, bounds, content.textRole);

  QVector<Part> parts{Part::fromContent(
      layout, content, Placement::fill(),
      flat ? Motion() : ButtonSpec.motionStyle)};
  CompositeRenderer(_style->_helper).render(painter, bounds, parts, state);
  return true;
}

QRect ButtonControl::menuArrowSplit(const QStyleOption *option, QPainter *painter,
                                    const QRect &bounds,
                                    QPalette::ColorRole textRole) const {
  auto arrowRect(bounds);
  arrowRect.setLeft(bounds.right() - Render::MenuButton_IndicatorWidth + 1);
  arrowRect = _style->centerRect(arrowRect, Render::MenuButton_IndicatorWidth,
                                 Render::MenuButton_IndicatorWidth);
  arrowRect = _style->visualRect(option, arrowRect);
  _style->_helper->renderArrow(
      painter, arrowRect, _style->_helper->arrowColor(option->palette, textRole),
      ArrowDown);

  QRect contents(bounds);
  contents.setRight(bounds.right() - Render::MenuButton_IndicatorWidth -
                    Render::Button_ItemSpacing);
  contents.adjust(Render::Button_MarginWidth, 0, 0, 0);
  return contents;
}

bool ButtonControl::drawToolLabel(const QStyleOption *option, QPainter *painter,
                                  const QWidget *widget) const {
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));
  if (!toolButtonOption)
    return true;

  const QStyle::State &qstate(option->state);
  const bool enabled(qstate & QStyle::State_Enabled);
  const bool sunken(qstate & (QStyle::State_On | QStyle::State_Sunken));
  const bool mouseOver(enabled && (qstate & QStyle::State_MouseOver));
  const bool flat(qstate & QStyle::State_AutoRaise);
  const bool hasArrow(toolButtonOption->features & QStyleOptionToolButton::Arrow);

  bool hasFocus = enabled && !mouseOver &&
                  (qstate & (flat ? QStyle::State_HasFocus
                                  : (QStyle::State_HasFocus | QStyle::State_Sunken)));

  WidgetInteractionState state;
  state.enabled = enabled;
  state.pressed = sunken;
  state.hovered = mouseOver;
  state.focused = hasFocus;
  state.palette = option->palette;
  state.pressOpacity = _style->_animations->widgetStateEngine().opacity(
      widget ? widget : option->styleObject, AnimationPressed);

  const bool leftAlign(
      widget &&
      widget->property(PropertyNames::toolButtonAlignment).toInt() == Qt::AlignLeft);
  ContentLayout layout = toolButtonContent(toolButtonOption->toolButtonStyle,
                                           toolButtonOption->iconSize, leftAlign);
  if (hasArrow)
    layout.arrangement = ContentArrangement::TextOnly;

  Content content;
  content.text = toolButtonOption->text;
  content.icon = hasArrow ? QIcon() : toolButtonOption->icon;
  content.font = toolButtonOption->font;
  content.iconState = sunken ? QIcon::On : QIcon::Off;
  content.iconMode = toolButtonIconMode(enabled, flat, hasFocus, sunken, mouseOver);
  content.textRole = toolButtonTextRole(flat, hasFocus, sunken, mouseOver);
  content.textFlags = _style->_mnemonics->textFlags();
  if (layout.arrangement == ContentArrangement::TextOnly ||
      layout.arrangement == ContentArrangement::TextUnderIcon)
    content.textFlags |= Qt::AlignCenter;
  else
    content.textFlags |= Qt::AlignLeft | Qt::AlignVCenter;

  QVector<Part> parts{Part::fromContent(
      layout, content, Placement::fill(), ToolButtonSpec.motionStyle)};
  CompositeRenderer(_style->_helper).render(painter, option->rect, parts, state);

  if (hasArrow) {
    QStyleOptionToolButton copy(*toolButtonOption);
    copy.rect = option->rect;
    switch (toolButtonOption->arrowType) {
    case Qt::LeftArrow:
      _style->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &copy, painter, widget);
      break;
    case Qt::RightArrow:
      _style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &copy, painter, widget);
      break;
    case Qt::UpArrow:
      _style->drawPrimitive(QStyle::PE_IndicatorArrowUp, &copy, painter, widget);
      break;
    case Qt::DownArrow:
      _style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &copy, painter, widget);
      break;
    default:
      break;
    }
  }
  return true;
}

bool ButtonControl::drawToolComplex(const QStyleOptionComplex *option,
                                    QPainter *painter, const QWidget *widget) const {
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));
  if (!toolButtonOption)
    return true;

  const QStyle::State &qstate(option->state);
  const bool enabled(qstate & QStyle::State_Enabled);
  const bool mouseOver(enabled && (qstate & QStyle::State_MouseOver));
  const bool hasFocus(enabled && (qstate & QStyle::State_HasFocus));
  const bool sunken(qstate & (QStyle::State_On | QStyle::State_Sunken));
  const bool flat(qstate & QStyle::State_AutoRaise);

  auto &engine = _style->_animations->widgetStateEngine();
  engine.updateState(widget, AnimationHover, mouseOver);
  engine.updateState(widget, AnimationFocus, hasFocus && !mouseOver);

  const bool inTabBar(widget &&
                      qobject_cast<const QTabBar *>(widget->parentWidget()));
  if (_style->isMenuTitle(widget)) {
    QStyleOptionToolButton copy(*toolButtonOption);
    copy.font.setBold(false);
    copy.state = QStyle::State_Enabled;
    _style->renderMenuTitle(&copy, painter, widget);
    return true;
  }

  QStyleOptionToolButton copy(*toolButtonOption);
  const bool hasPopupMenu(toolButtonOption->features &
                          QStyleOptionToolButton::MenuButtonPopup);
  const bool hasInlineIndicator(
      toolButtonOption->features & QStyleOptionToolButton::HasMenu &&
      toolButtonOption->features & QStyleOptionToolButton::PopupDelay && !hasPopupMenu);

  const auto buttonRect(
      _style->subControlRect(QStyle::CC_ToolButton, option, QStyle::SC_ToolButton, widget));
  const auto menuRect(_style->subControlRect(QStyle::CC_ToolButton, option,
                                             QStyle::SC_ToolButtonMenu, widget));

  if (toolButtonOption->subControls & QStyle::SC_ToolButton) {
    if (!flat)
      copy.rect = buttonRect;
    if (inTabBar)
      drawTabBarPanelTool(&copy, painter, widget);
    else
      _style->drawPrimitive(QStyle::PE_PanelButtonTool, &copy, painter, widget);
  }

  if (!isDolphinNavButton(widget)) {
    const int shrink = Render::Button_PressedShrink;
    if (hasPopupMenu) {
      copy.rect = menuRect;
      if (!flat)
        _style->drawPrimitive(QStyle::PE_IndicatorButtonDropDown, &copy, painter, widget);
      if (sunken && !flat)
        copy.rect.adjust(shrink, shrink, -shrink, -shrink);
      if (flat)
        copy.rect.translate(-Render::MenuButton_IndicatorWidth / 4, 0);
      _style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &copy, painter, widget);

    } else if (hasInlineIndicator) {
      copy.rect = menuRect;
      if (sunken && !flat)
        copy.rect.adjust(shrink, shrink, -shrink, -shrink);
      _style->drawIndicatorArrowPrimitive(ArrowDownSmall, &copy, painter, widget);
    }
  }

  copy.state = qstate;
  auto contentsRect(buttonRect);
  if (widget && widget->inherits("QDockWidgetTitleButton")) {
    const auto button(qobject_cast<const QAbstractButton *>(widget));
    if (button->isChecked() || button->isDown())
      copy.state |= QStyle::State_On;

  } else if (!inTabBar && hasInlineIndicator) {
    const int marginWidth(flat ? Render::ToolButton_MarginWidth
                               : Render::Button_MarginWidth + Render::Frame_FrameWidth);
    contentsRect = _style->insideMargin(contentsRect, marginWidth, 0);
    contentsRect = _style->visualRect(option, contentsRect);
  }
  copy.rect = contentsRect;
  _style->drawControl(QStyle::CE_ToolButtonLabel, &copy, painter, widget);
  return true;
}

//* Dolphin draws its own back/forward chevrons as icons; skip the style's
bool ButtonControl::isDolphinNavButton(const QWidget *widget) const {
  if (!_style->_app.isDolphin || !widget)
    return false;
  const QString name(widget->objectName());
  if (name == QLatin1String("go_back") || name == QLatin1String("go_forward"))
    return true;
  if (const auto *toolButton = qobject_cast<const QToolButton *>(widget)) {
    if (const QAction *action = toolButton->defaultAction()) {
      const QString actionName(action->objectName());
      return actionName == QLatin1String("go_back") ||
             actionName == QLatin1String("go_forward");
    }
  }
  return false;
}

void ButtonControl::polish(QWidget *widget) {
  widget->setAttribute(Qt::WA_Hover);
  QObject::connect(
      static_cast<QAbstractButton *>(widget), &QAbstractButton::pressed, widget,
      [widget]() { widget->repaint(); }, Qt::DirectConnection);
}

bool ButtonControl::polishAutoRaise(QWidget *widget) {
  auto toolButton = qobject_cast<QToolButton *>(widget);
  if (!toolButton)
    return false;

  if (toolButton->autoRaise()) {
    widget->setBackgroundRole(QPalette::NoRole);
    widget->setForegroundRole(QPalette::WindowText);
  }
  if (widget->parentWidget() && widget->parentWidget()->parentWidget() &&
      widget->parentWidget()->parentWidget()->inherits("Gwenview::SideBarGroup"))
    widget->setProperty(PropertyNames::toolButtonAlignment, Qt::AlignLeft);

  return true;
}

} // namespace Render

bool Style::drawPanelButtonCommandPrimitive(const QStyleOption *option,
                                            QPainter *painter,
                                            const QWidget *widget) const {
  return Render::ButtonControl(this).drawPanelCommand(option, painter, widget);
}

bool Style::drawPanelButtonToolPrimitive(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  return Render::ButtonControl(this).drawPanelTool(option, painter, widget);
}

bool Style::drawTabBarPanelButtonToolPrimitive(const QStyleOption *option,
                                               QPainter *painter,
                                               const QWidget *widget) const {
  return Render::ButtonControl(this).drawTabBarPanelTool(option, painter, widget);
}

bool Style::drawIndicatorButtonDropDownPrimitive(const QStyleOption *option,
                                                 QPainter *painter,
                                                 const QWidget *widget) const {
  return Render::ButtonControl(this).drawDropDownIndicator(option, painter, widget);
}

bool Style::drawPushButtonLabelControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  return Render::ButtonControl(this).drawPushLabel(option, painter, widget);
}

bool Style::drawToolButtonLabelControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  return Render::ButtonControl(this).drawToolLabel(option, painter, widget);
}

bool Style::drawToolButtonComplexControl(const QStyleOptionComplex *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  return Render::ButtonControl(this).drawToolComplex(option, painter, widget);
}

void Style::polishButton(QWidget *widget) {
  if (!(qobject_cast<QPushButton *>(widget) || qobject_cast<QToolButton *>(widget)))
    return;
  addEventFilter(widget);
  Render::ButtonControl::polish(widget);
}

bool Style::polishAutoRaiseToolButton(QWidget *widget) {
  return Render::ButtonControl::polishAutoRaise(widget);
}

bool Style::polishCommandLinkButton(QWidget *widget) {
  if (!qobject_cast<QCommandLinkButton *>(widget))
    return false;
  addEventFilter(widget);
  return true;
}

} // namespace BlossomUI

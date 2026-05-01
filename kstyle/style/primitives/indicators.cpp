// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "widgets/switch.h"

#include <KColorUtils>
#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QStyleOptionHeader>
#include <QStyleOptionToolButton>
#include <QTreeView>

namespace BlossomUI {

bool Style::drawIndicatorArrowPrimitive(ArrowOrientation orientation,
                                        const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // store rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // store state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  bool mouseOver(enabled && (state & State_MouseOver));
  bool hasFocus(enabled && (state & State_HasFocus));

  // detect special buttons
  const bool inTabBar(widget &&
                      qobject_cast<const QTabBar *>(widget->parentWidget()));
  const bool inToolButton(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));

  // color
  QColor color;
  if (inTabBar) {
    // for tabbar arrows one uses animations to get the arrow color
    /*
     * get animation state
     * there is no need to update the engine since this was already done when
     * rendering the frame
     */
    const AnimationMode mode(
        _animations->widgetStateEngine().buttonAnimationMode(widget));
    const qreal opacity(_animations->widgetStateEngine().buttonOpacity(widget));
    color = _helper->arrowColor(palette, mouseOver, hasFocus, opacity, mode);

  } else if (mouseOver && !inToolButton) {
    color = _helper->arrowColor(palette, QPalette::WindowText);

  } else if (inToolButton) {
    const bool flat(state & State_AutoRaise);
    color = flat ? _helper->arrowColor(palette, QPalette::WindowText)
                 : _helper->arrowColor(palette, QPalette::ButtonText);

  } else
    color = _helper->arrowColor(palette, QPalette::WindowText);

  // render
  _helper->renderArrow(painter, rect, color, orientation);

  return true;
}

bool Style::drawIndicatorHeaderArrowPrimitive(const QStyleOption *option,
                                              QPainter *painter,
                                              const QWidget *) const {
  const auto headerOption(
      qstyleoption_cast<const QStyleOptionHeader *>(option));
  const State &state(option->state);

  // arrow orientation
  ArrowOrientation orientation(ArrowNone);
  if (state & State_UpArrow || (headerOption && headerOption->sortIndicator ==
                                                    QStyleOptionHeader::SortUp))
    orientation = ArrowUp;
  else if (state & State_DownArrow ||
           (headerOption &&
            headerOption->sortIndicator == QStyleOptionHeader::SortDown))
    orientation = ArrowDown;
  if (orientation == ArrowNone)
    return true;

  // invert arrows if requested by (hidden) options
  if (StyleConfigData::viewInvertSortIndicator())
    orientation = (orientation == ArrowUp) ? ArrowDown : ArrowUp;

  // state, rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // define color and polygon for drawing arrow
  const auto color = _helper->arrowColor(palette, QPalette::ButtonText);

  // render
  _helper->renderArrow(painter, rect, color, orientation);

  return true;
}

bool Style::drawIndicatorCheckBoxPrimitive(const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *widget) const {
  // when a switch overlay widget is present, it draws the indicator
  if (widget) {
    QObject *ov =
        widget->property("blossomui-switch-overlay").value<QObject *>();
    if (ov && qobject_cast<BlossomUISwitchWidget *>(ov) &&
        static_cast<BlossomUISwitchWidget *>(ov)->isVisible())
      return true;
  }

  // Slint (and similar) may pass widget=null or the window instead of the
  // checkbox; ensure overlay exists
  if (isSwitchCheckBox(option, widget)) {
    QWidget *window =
        widget ? widget->window() : dynamic_cast<QWidget *>(painter->device());
    if (window)
      window = window->window();
    if (window)
      for (QCheckBox *cb : window->findChildren<QCheckBox *>())
        if (cb->text().contains(QLatin1String("Switch"), Qt::CaseInsensitive) &&
            !cb->property("blossomui-switch-overlay").value<QObject *>()) {
          const_cast<Style *>(this)->polish(cb);
          break;
        }
  }

  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // store flags
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool sunken(enabled && (state & State_Sunken));
  // const bool active( ( state & (State_On|State_NoChange) ) );

  // checkbox state
  CheckBoxState checkBoxState(CheckOff);
  if (state & State_NoChange)
    checkBoxState = CheckPartial;
  else if (state & State_On)
    checkBoxState = CheckOn;

  // animation state
  const QObject *styleObject(widget ? widget : option->styleObject);
  isQtQuickControl(option, widget);
  _animations->widgetStateEngine().updateState(styleObject, AnimationHover,
                                               mouseOver);
  _animations->widgetStateEngine().updateState(styleObject, AnimationPressed,
                                               checkBoxState != CheckOff);
  if (_animations->widgetStateEngine().isAnimated(styleObject, AnimationPressed))
    checkBoxState = CheckAnimated;
  const qreal animation(
      _animations->widgetStateEngine().opacity(styleObject, AnimationPressed));

  if (isSwitchCheckBox(option, widget))
    _helper->renderSwitch(painter, rect, palette, sunken, mouseOver,
                          checkBoxState, animation);
  else
    _helper->renderCheckBox(painter, rect, palette, false, sunken, mouseOver,
                            checkBoxState, false, animation);
  return true;
}

bool Style::drawIndicatorRadioButtonPrimitive(const QStyleOption *option,
                                              QPainter *painter,
                                              const QWidget *widget) const {
  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // store flags
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool sunken(state & State_Sunken);
  // const bool checked( state & State_On );

  // radio button state
  RadioButtonState radioButtonState(state & State_On ? RadioOn : RadioOff);

  // animation state
  const QObject *styleObject(widget ? widget : option->styleObject);
  isQtQuickControl(option, widget);
  _animations->widgetStateEngine().updateState(styleObject, AnimationHover,
                                               mouseOver);
  _animations->widgetStateEngine().updateState(
      styleObject, AnimationPressed, radioButtonState != RadioOff, AnimationOutBack);
  if (_animations->widgetStateEngine().isAnimated(styleObject, AnimationPressed))
    radioButtonState = RadioAnimated;
  const qreal animation(
      _animations->widgetStateEngine().opacity(styleObject, AnimationPressed));

  _helper->renderRadioButton(painter, rect, palette, mouseOver, sunken,
                             radioButtonState, false, animation);

  return true;
}

bool Style::drawIndicatorButtonDropDownPrimitive(const QStyleOption *option,
                                                 QPainter *painter,
                                                 const QWidget *widget) const {
  // cast option and check
  const auto toolButtonOption(
      qstyleoption_cast<const QStyleOptionToolButton *>(option));
  if (!toolButtonOption)
    return true;

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  // store state
  const State &state(option->state);
  const bool autoRaise(state & State_AutoRaise);

  // do nothing for autoraise buttons
  if (autoRaise || !(toolButtonOption->subControls & SC_ToolButtonMenu))
    return true;

  // store palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  // store state
  const bool enabled(state & State_Enabled);
  const bool hasFocus(enabled && (state & (State_HasFocus | State_Sunken)));
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool sunken(enabled && (state & State_Sunken));

  // update animation state
  // mouse over takes precedence over focus
  _animations->widgetStateEngine().updateState(widget, AnimationHover,
                                               mouseOver);
  _animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                               hasFocus && !mouseOver);

  const AnimationMode mode(
      _animations->widgetStateEngine().buttonAnimationMode(widget));
  const qreal opacity(_animations->widgetStateEngine().buttonOpacity(widget));

  // render as push button
  // const auto shadow( _helper->shadowColor( palette ) );
  const auto outline(
      _helper->buttonOutlineColor(palette, mouseOver, hasFocus, opacity, mode));
  const auto background(_helper->buttonBackgroundColor(
      palette, mouseOver, hasFocus, false, opacity, mode));

  auto frameRect(rect);
  painter->setClipRect(rect);
  frameRect.adjust(-StyleConfigData::cornerRadius() - 1, 0, 0, 0);
  frameRect = visualRect(option, frameRect);

  // render
  _helper->renderButtonFrame(painter, frameRect, background, palette, hasFocus,
                             sunken, mouseOver, enabled,
                             windowActive); // TODO: use sides?

  // also render separator
  auto separatorRect(rect.adjusted(0, 9, -2, -9));
  separatorRect.setWidth(1);
  separatorRect = visualRect(option, separatorRect);
  if (sunken)
    separatorRect.translate(1, 1);
  _helper->renderSeparator(painter, separatorRect, outline, true);

  return true;
}

bool Style::drawIndicatorTabClosePrimitive(const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *widget) const {
  // get icon and check
  QIcon icon(standardIcon(SP_TitleBarCloseButton, option, widget));
  if (icon.isNull())
    return false;

  // store state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool active(state & State_Raised);
  const bool sunken(state & State_Sunken);

  // decide icon mode and state
  QIcon::Mode iconMode;
  QIcon::State iconState;
  if (!enabled) {
    iconMode = QIcon::Disabled;
    iconState = QIcon::Off;

  } else {
    if (active)
      iconMode = QIcon::Active;
    else
      iconMode = QIcon::Normal;

    iconState = sunken ? QIcon::On : QIcon::Off;
  }

  // icon size
  const int iconWidth(pixelMetric(QStyle::PM_SmallIconSize, option, widget));
  const QSize iconSize(iconWidth, iconWidth);

  // get pixmap
  const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                      : qApp->devicePixelRatio();
  const QPixmap pixmap(_helper->coloredIcon(icon, option->palette, iconSize,
                                            dpr, iconMode, iconState));

  // render
  drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
  return true;
}

bool Style::drawIndicatorTabTearPrimitive(const QStyleOption *option,
                                          QPainter *painter,
                                          const QWidget *) const {
  // cast option and check
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  if (!tabOption)
    return true;

  // store palette and rect
  const auto &palette(option->palette);
  auto rect(option->rect);

  const bool reverseLayout(option->direction == Qt::RightToLeft);

  const auto color(
      _helper->alphaColor(palette.color(QPalette::WindowText), 0.2));
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setPen(color);
  painter->setBrush(Qt::NoBrush);
  switch (tabOption->shape) {
  case QTabBar::TriangularNorth:
  case QTabBar::RoundedNorth:
    rect.adjust(0, 1, 0, 0);
    if (reverseLayout)
      painter->drawLine(rect.topRight(), rect.bottomRight());
    else
      painter->drawLine(rect.topLeft(), rect.bottomLeft());
    break;

  case QTabBar::TriangularSouth:
  case QTabBar::RoundedSouth:
    rect.adjust(0, 0, 0, -1);
    if (reverseLayout)
      painter->drawLine(rect.topRight(), rect.bottomRight());
    else
      painter->drawLine(rect.topLeft(), rect.bottomLeft());
    break;

  case QTabBar::TriangularWest:
  case QTabBar::RoundedWest:
    rect.adjust(1, 0, 0, 0);
    painter->drawLine(rect.topLeft(), rect.topRight());
    break;

  case QTabBar::TriangularEast:
  case QTabBar::RoundedEast:
    rect.adjust(0, 0, -1, 0);
    painter->drawLine(rect.topLeft(), rect.topRight());
    break;

  default:
    break;
  }

  return true;
}

bool Style::drawIndicatorToolBarHandlePrimitive(const QStyleOption *option,
                                                QPainter *painter,
                                                const QWidget *) const {
  // do nothing if disabled from options
  if (!StyleConfigData::toolBarDrawItemSeparator())
    return true;

  // store rect and palette
  auto rect(option->rect);
  const auto &palette(option->palette);

  // store state
  const State &state(option->state);
  const bool separatorIsVertical(state & State_Horizontal);

  // define color and render
  const auto color(_helper->separatorColor(palette));
  if (separatorIsVertical) {
    rect.setWidth(Metrics::ToolBar_HandleWidth);
    rect = centerRect(option->rect, rect.size());
    rect.setWidth(3);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);

    rect.translate(2, 0);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);

  } else {
    rect.setHeight(Metrics::ToolBar_HandleWidth);
    rect = centerRect(option->rect, rect.size());
    rect.setHeight(3);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);

    rect.translate(0, 2);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);
  }

  return true;
}

bool Style::drawIndicatorToolBarSeparatorPrimitive(
    const QStyleOption *option, QPainter *painter,
    const QWidget *widget) const {
  /*
   * do nothing if disabled from options
   * also need to check if widget is a combobox, because of Qt hack using
   * 'toolbar' separator primitive for rendering separators in comboboxes
   */
  if (!(StyleConfigData::toolBarDrawItemSeparator() ||
        qobject_cast<const QComboBox *>(widget))) {
    return true;
  }

  // store rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // store state
  const State &state(option->state);
  const bool separatorIsVertical(state & State_Horizontal);

  // define color and render
  const auto color(_helper->separatorColor(palette));
  _helper->renderSeparator(painter, rect, color, separatorIsVertical);

  return true;
}

bool Style::drawIndicatorBranchPrimitive(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *) const {
  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // state
  const State &state(option->state);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // draw expander
  int expanderAdjust = 0;
  if (state & State_Children) {
    // state
    const bool expanderOpen(state & State_Open);
    const bool enabled(state & State_Enabled);
    Q_UNUSED(enabled)

    // expander rect
    int expanderSize = qMin(rect.width(), rect.height());
    expanderSize = qMin(expanderSize, int(Metrics::ItemView_ArrowSize));
    expanderAdjust = expanderSize / 2 + 1;
    const auto arrowRect = centerRect(rect, expanderSize, expanderSize);

    // get orientation from option
    ArrowOrientation orientation;
    if (expanderOpen)
      orientation = ArrowDown;
    else if (reverseLayout)
      orientation = ArrowLeft;
    else
      orientation = ArrowRight;

    const auto arrowColor(_helper->arrowColor(palette, QPalette::Text));

    // render
    _helper->renderArrow(painter, arrowRect, arrowColor, orientation);
  }

  // tree branches
  if (!StyleConfigData::viewDrawTreeBranchLines())
    return true;

  const auto center(rect.center());
  const auto lineColor(KColorUtils::mix(palette.color(QPalette::Base),
                                        palette.color(QPalette::Text), 0.25));
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->translate(0.5, 0.5);
  painter->setPen(QPen(lineColor, 1));
  if (state & (State_Item | State_Children | State_Sibling)) {
    const QLineF line(QPointF(center.x(), rect.top()),
                      QPointF(center.x(), center.y() - expanderAdjust - 1));
    painter->drawLine(line);
  }

  // The right/left (depending on direction) line gets drawn if we have an item
  if (state & State_Item) {
    const QLineF line =
        reverseLayout ? QLineF(QPointF(rect.left(), center.y()),
                               QPointF(center.x() - expanderAdjust, center.y()))
                      : QLineF(QPointF(center.x() + expanderAdjust, center.y()),
                               QPointF(rect.right(), center.y()));
    painter->drawLine(line);
  }

  // The bottom if we have a sibling
  if (state & State_Sibling) {
    const QLineF line(QPointF(center.x(), center.y() + expanderAdjust),
                      QPointF(center.x(), rect.bottom()));
    painter->drawLine(line);
  }

  return true;
}

} // namespace BlossomUI

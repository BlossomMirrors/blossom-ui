// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "itemview.h"
#include "switchwidget.h"
#include "toolbarcontrol.h"

#include <KColorUtils>
#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QEasingCurve>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QStyleOptionButton>
#include <QStyleOptionHeader>
#include <QStyleOptionToolButton>
#include <QTreeView>

namespace BlossomUI {

//* contrast for arrow and treeline rendering
static const qreal arrowShade = 0.15;

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

  if (isSwitchCheckBox(option, widget)) {
    // overshooting thumb: ease toward the target so turning on overshoots
    // past the end position and turning off overshoots past the start
    qreal t = animation;
    if (checkBoxState == CheckAnimated && animation >= 0) {
      const QEasingCurve ease(QEasingCurve::OutBack);
      const bool switchOn = (state & State_On);
      t = switchOn ? ease.valueForProgress(animation)
                   : 1.0 - ease.valueForProgress(1.0 - animation);
    }
    _helper->renderSwitch(painter, rect, palette, sunken, mouseOver,
                          checkBoxState, t);
  } else
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
    rect.setWidth(Render::ToolBar_HandleWidth);
    rect = centerRect(option->rect, rect.size());
    rect.setWidth(3);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);

    rect.translate(2, 0);
    _helper->renderSeparator(painter, rect, color, separatorIsVertical);

  } else {
    rect.setHeight(Render::ToolBar_HandleWidth);
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
    expanderSize = qMin(expanderSize, int(Render::ItemView_ArrowSize));
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

QColor Helper::arrowColor(const QPalette &palette, QPalette::ColorGroup group,
                          QPalette::ColorRole role) const {
  switch (role) {
  case QPalette::Text:
    return KColorUtils::mix(palette.color(group, QPalette::Text),
                            palette.color(group, QPalette::Base), arrowShade);
  case QPalette::WindowText:
    return KColorUtils::mix(palette.color(group, QPalette::WindowText),
                            palette.color(group, QPalette::Window), arrowShade);
  case QPalette::ButtonText:
    return KColorUtils::mix(palette.color(group, QPalette::ButtonText),
                            palette.color(group, QPalette::Button), arrowShade);
  default:
    return palette.color(group, role);
  }
}

QColor Helper::arrowColor(const QPalette &palette, bool mouseOver,
                          bool hasFocus, qreal opacity,
                          AnimationMode mode) const {
  QColor outline(arrowColor(palette, QPalette::WindowText));
  if (mode == AnimationHover) {
    const QColor focus(focusColor(palette));
    const QColor hover(hoverColor(palette));
    if (hasFocus)
      outline = KColorUtils::mix(focus, hover, opacity);
    else
      outline = KColorUtils::mix(outline, hover, opacity);

  } else if (mouseOver) {
    // fix skanlite arrow color bug (mouseOver shows dark color (focusColor),
    // not light color (hoverColor))
    outline = focusColor(palette);

  } else if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = focusColor(palette);
  }

  return outline;
}

void Helper::renderArrow(QPainter *painter, const QRect &rect,
                         const QColor &color,
                         ArrowOrientation orientation) const {
  // define polygon
  QPolygonF arrow;
  switch (orientation) {
  /* The inner points of the normal arrows are not on half pixels because
   * they need to have an even width (up/down) or height (left/right).
   * An even width/height makes them easier to align with other UI elements.
   */
  case ArrowUp:
    arrow =
        QVector<QPointF>{QPointF(-4.5, 1.5), QPointF(0, -3), QPointF(4.5, 1.5)};
    break;
  case ArrowDown:
    arrow = QVector<QPointF>{QPointF(-4.5, -1.5), QPointF(0, 3),
                             QPointF(4.5, -1.5)};
    break;
  case ArrowLeft:
    arrow =
        QVector<QPointF>{QPointF(1.5, -4.5), QPointF(-3, 0), QPointF(1.5, 4.5)};
    break;
  case ArrowRight:
    arrow = QVector<QPointF>{QPointF(-1.5, -4.5), QPointF(3, 0),
                             QPointF(-1.5, 4.5)};
    break;
  case ArrowDownSmall:
    arrow = QVector<QPointF>{QPointF(1.5, 3.5), QPointF(3.5, 5.5),
                             QPointF(5.5, 3.5)};
    break;
  default:
    break;
  }

  painter->save();
  painter->setRenderHints(QPainter::Antialiasing);
  painter->translate(QRectF(rect).center());
  painter->setBrush(Qt::NoBrush);
  QPen pen(color, PenWidth::Symbol);
  pen.setCapStyle(Qt::SquareCap);
  pen.setJoinStyle(Qt::MiterJoin);
  painter->setPen(pen);
  painter->drawPolyline(arrow);
  painter->restore();
}

} // namespace BlossomUI

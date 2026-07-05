// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <KColorUtils>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionSlider>

namespace BlossomUI {

bool Style::drawScrollBarSliderControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  // copy rect and palette
  // const auto& rect( option->rect );
  const auto &palette(option->palette);

  // need to make it center due to the thin line separator
  QRect rect = option->rect;

  if (option->state & State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  // try to understand if anywhere the widget is under mouse, not just the
  // handle, use _animations in case of QWidget, option->styleObject in case of
  // QML
  bool widgetMouseOver((option->state & State_MouseOver));
  if (widget)
    widgetMouseOver = _animations->scrollBarEngine().isHovered(
        widget, QStyle::SC_ScrollBarGroove);
  else if (option->styleObject)
    widgetMouseOver = option->styleObject->property("hover").toBool();

  qreal grooveAnimationOpacity(_animations->scrollBarEngine().opacity(
      widget, QStyle::SC_ScrollBarGroove));
  if (grooveAnimationOpacity == AnimationData::OpacityInvalid)
    grooveAnimationOpacity = (widgetMouseOver ? 1 : 0);

  // define state
  const State &state(option->state);
  const bool horizontal(state & State_Horizontal);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));

  // check focus from relevant parent
  const QWidget *parent(scrollBarParent(widget));
  const bool hasFocus(enabled && ((widget && widget->hasFocus()) ||
                                  (parent && parent->hasFocus())));

  // enable animation state
  const bool handleActive(sliderOption->activeSubControls & SC_ScrollBarSlider);
  _animations->scrollBarEngine().updateState(widget, AnimationFocus, hasFocus);

  _animations->scrollBarEngine().updateState(widget, AnimationHover,
                                             mouseOver && handleActive);

  const auto mode(
      _animations->scrollBarEngine().animationMode(widget, SC_ScrollBarSlider));
  const qreal opacity(
      _animations->scrollBarEngine().opacity(widget, SC_ScrollBarSlider));
  Q_UNUSED(mode)
  Q_UNUSED(opacity)

  QColor color;
  if (state & State_Sunken) {
    color = palette.color(QPalette::Highlight);
  } else {
    color = _helper->alphaColor(palette.color(QPalette::WindowText), 0.35);
  }
  if (StyleConfigData::animationsEnabled()) {
    color.setAlphaF(color.alphaF() * (0.7 + 0.3 * grooveAnimationOpacity));
  }

  // define handle rect
  QRectF handleRect;
  const qreal sliderWidth = static_cast<qreal>(Metrics::ScrollBar_SliderWidth) /
                            (2 - grooveAnimationOpacity);
  if (horizontal)
    handleRect = centerRectF(rect, rect.width(), sliderWidth);
  else
    handleRect = centerRectF(rect, sliderWidth, rect.height());

  _helper->renderScrollBarHandle(painter, handleRect, color);
  return true;
}

bool Style::drawScrollBarAddLineControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // do nothing if no buttons are defined
  if (_addLineButtons == NoButton)
    return true;

  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const State &state(option->state);
  const bool horizontal(state & State_Horizontal);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // adjust rect, based on number of buttons to be drawn
  auto rect(scrollBarInternalSubControlRect(sliderOption, SC_ScrollBarAddLine));

  // need to make it center due to the thin line separator
  if (option->state & State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  QColor color;
  QStyleOptionSlider copy(*sliderOption);
  if (_addLineButtons == DoubleButton) {
    if (horizontal) {
      // Draw the arrows
      const QSize halfSize(rect.width() / 2, rect.height());
      const QRect leftSubButton(rect.topLeft(), halfSize);
      const QRect rightSubButton(leftSubButton.topRight() + QPoint(1, 0),
                                 halfSize);

      copy.rect = leftSubButton;
      color = scrollBarArrowColor(
          &copy, reverseLayout ? SC_ScrollBarAddLine : SC_ScrollBarSubLine,
          widget);
      _helper->renderArrow(painter, leftSubButton, color, ArrowLeft);

      copy.rect = rightSubButton;
      color = scrollBarArrowColor(
          &copy, reverseLayout ? SC_ScrollBarSubLine : SC_ScrollBarAddLine,
          widget);
      _helper->renderArrow(painter, rightSubButton, color, ArrowRight);

    } else {
      const QSize halfSize(rect.width(), rect.height() / 2);
      const QRect topSubButton(rect.topLeft(), halfSize);
      const QRect botSubButton(topSubButton.bottomLeft() + QPoint(0, 1),
                               halfSize);

      copy.rect = topSubButton;
      color = scrollBarArrowColor(&copy, SC_ScrollBarSubLine, widget);
      _helper->renderArrow(painter, topSubButton, color, ArrowUp);

      copy.rect = botSubButton;
      color = scrollBarArrowColor(&copy, SC_ScrollBarAddLine, widget);
      _helper->renderArrow(painter, botSubButton, color, ArrowDown);
    }

  } else if (_addLineButtons == SingleButton) {
    copy.rect = rect;
    color = scrollBarArrowColor(&copy, SC_ScrollBarAddLine, widget);
    if (horizontal) {
      if (reverseLayout)
        _helper->renderArrow(painter, rect, color, ArrowLeft);
      else
        _helper->renderArrow(painter, rect.translated(1, 0), color, ArrowRight);

    } else
      _helper->renderArrow(painter, rect.translated(0, 1), color, ArrowDown);
  }

  return true;
}

bool Style::drawScrollBarSubLineControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // do nothing if no buttons are set
  if (_subLineButtons == NoButton)
    return true;

  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const State &state(option->state);
  const bool horizontal(state & State_Horizontal);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // adjust rect, based on number of buttons to be drawn
  auto rect(scrollBarInternalSubControlRect(sliderOption, SC_ScrollBarSubLine));

  // need to make it center due to the thin line separator
  if (option->state & State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  QColor color;
  QStyleOptionSlider copy(*sliderOption);
  if (_subLineButtons == DoubleButton) {
    if (horizontal) {
      // Draw the arrows
      const QSize halfSize(rect.width() / 2, rect.height());
      const QRect leftSubButton(rect.topLeft(), halfSize);
      const QRect rightSubButton(leftSubButton.topRight() + QPoint(1, 0),
                                 halfSize);

      copy.rect = leftSubButton;
      color = scrollBarArrowColor(
          &copy, reverseLayout ? SC_ScrollBarAddLine : SC_ScrollBarSubLine,
          widget);
      _helper->renderArrow(painter, leftSubButton, color, ArrowLeft);

      copy.rect = rightSubButton;
      color = scrollBarArrowColor(
          &copy, reverseLayout ? SC_ScrollBarSubLine : SC_ScrollBarAddLine,
          widget);
      _helper->renderArrow(painter, rightSubButton, color, ArrowRight);

    } else {
      const QSize halfSize(rect.width(), rect.height() / 2);
      const QRect topSubButton(rect.topLeft(), halfSize);
      const QRect botSubButton(topSubButton.bottomLeft() + QPoint(0, 1),
                               halfSize);

      copy.rect = topSubButton;
      color = scrollBarArrowColor(&copy, SC_ScrollBarSubLine, widget);
      _helper->renderArrow(painter, topSubButton, color, ArrowUp);

      copy.rect = botSubButton;
      color = scrollBarArrowColor(&copy, SC_ScrollBarAddLine, widget);
      _helper->renderArrow(painter, botSubButton, color, ArrowDown);
    }

  } else if (_subLineButtons == SingleButton) {
    copy.rect = rect;
    color = scrollBarArrowColor(&copy, SC_ScrollBarSubLine, widget);
    if (horizontal) {
      if (reverseLayout)
        _helper->renderArrow(painter, rect.translated(1, 0), color, ArrowRight);
      else
        _helper->renderArrow(painter, rect, color, ArrowLeft);

    } else
      _helper->renderArrow(painter, rect, color, ArrowUp);
  }

  return true;
}
bool Style::drawScrollBarComplexControl(const QStyleOptionComplex *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // the animation for QStyle::SC_ScrollBarGroove is special: it will animate
  // the opacity of everything else as well, included slider and arrows
  qreal opacity(_animations->scrollBarEngine().opacity(
      widget, QStyle::SC_ScrollBarGroove));
  const bool animated(StyleConfigData::animationsEnabled() &&
                      _animations->scrollBarEngine().isAnimated(
                          widget, AnimationHover, QStyle::SC_ScrollBarGroove));
  const bool mouseOver(option->state & State_MouseOver);

  if (opacity == AnimationData::OpacityInvalid)
    opacity = 1;

  QRect separatorRect;
  if (option->state & State_Horizontal) {
    separatorRect = QRect(0, 0, option->rect.width(), 1);
  } else {
    separatorRect = alignedRect(option->direction, Qt::AlignLeft,
                                QSize(PenWidth::Frame, option->rect.height()),
                                option->rect);
  }

  if (StyleConfigData::renderThinSeperatorBetweenTheScrollBar()) {
    _helper->renderScrollBarBorder(
        painter, separatorRect,
        _helper->alphaColor(option->palette.color(QPalette::Text), 0.1));
  }

  // render full groove directly, rather than using the addPage and subPage
  // control element methods
  if ((!StyleConfigData::animationsEnabled() || mouseOver || animated) &&
      option->subControls & SC_ScrollBarGroove) {
    // retrieve groove rectangle
    auto grooveRect(
        subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

    // need to make it center due to the thin line separator
    if (option->state & State_Horizontal) {
      grooveRect.setTop(PenWidth::Frame);
    } else if (option->direction == Qt::RightToLeft) {
      grooveRect.setRight(grooveRect.right() - PenWidth::Frame);
    } else {
      grooveRect.setLeft(PenWidth::Frame);
    }

    const auto &palette(option->palette);
    const auto color(_helper->alphaColor(palette.color(QPalette::WindowText),
                                         0.3 * (animated ? opacity : 1)));
    const auto &state(option->state);
    const bool horizontal(state & State_Horizontal);

    if (horizontal)
      grooveRect = centerRect(grooveRect, grooveRect.width(),
                              Metrics::ScrollBar_SliderWidth);
    else
      grooveRect = centerRect(grooveRect, Metrics::ScrollBar_SliderWidth,
                              grooveRect.height());

    // render
    _helper->renderScrollBarGroove(painter, grooveRect, color);
  }

  // call base class primitive
  ParentStyleClass::drawComplexControl(CC_ScrollBar, option, painter, widget);

  return true;
}

} // namespace BlossomUI

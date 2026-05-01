// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"

#include <KColorUtils>
#include <QApplication>
#include <QDial>
#include <QPainter>
#include <QPainterPath>
#include <QSlider>
#include <QStyleOptionSlider>

namespace BlossomUI {

bool Style::drawSliderComplexControl(const QStyleOptionComplex *option,
                                     QPainter *painter,
                                     const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // copy state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));

  const QObject *styleObject(widget ? widget : option->styleObject);
  isQtQuickControl(option, widget);

  // direction
  const bool horizontal(sliderOption->orientation == Qt::Horizontal);

  // tickmarks
  if (sliderOption->subControls & SC_SliderTickmarks) {
    const bool upsideDown(sliderOption->upsideDown);
    const int tickPosition(sliderOption->tickPosition);
    const int available(pixelMetric(PM_SliderSpaceAvailable, option, widget));
    int interval = sliderOption->tickInterval;
    if (interval < 1)
      interval = sliderOption->pageStep;
    if (interval >= 1) {
      const int fudge(pixelMetric(PM_SliderLength, option, widget) / 2);
      int current(sliderOption->minimum);

      // store tick lines
      const auto grooveRect(
          subControlRect(CC_Slider, sliderOption, SC_SliderGroove, widget));
      QList<QLine> tickLines;
      if (horizontal) {
        if (tickPosition & QSlider::TicksAbove)
          tickLines.append(QLine(
              rect.left(), grooveRect.top() - Metrics::Slider_TickMarginWidth,
              rect.left(),
              grooveRect.top() - Metrics::Slider_TickMarginWidth -
                  Metrics::Slider_TickLength));
        if (tickPosition & QSlider::TicksBelow)
          tickLines.append(
              QLine(rect.left(),
                    grooveRect.bottom() + Metrics::Slider_TickMarginWidth,
                    rect.left(),
                    grooveRect.bottom() + Metrics::Slider_TickMarginWidth +
                        Metrics::Slider_TickLength));

      } else {
        if (tickPosition & QSlider::TicksAbove)
          tickLines.append(QLine(
              grooveRect.left() - Metrics::Slider_TickMarginWidth, rect.top(),
              grooveRect.left() - Metrics::Slider_TickMarginWidth -
                  Metrics::Slider_TickLength,
              rect.top()));
        if (tickPosition & QSlider::TicksBelow)
          tickLines.append(QLine(
              grooveRect.right() + Metrics::Slider_TickMarginWidth, rect.top(),
              grooveRect.right() + Metrics::Slider_TickMarginWidth +
                  Metrics::Slider_TickLength,
              rect.top()));
      }

      // colors
      const auto base(_helper->separatorColor(palette));
      const auto &highlight = palette.color(QPalette::Highlight);

      while (current <= sliderOption->maximum) {
        // adjust color
        const auto color((enabled && current <= sliderOption->sliderPosition)
                             ? highlight
                             : base);
        painter->setPen(color);

        // calculate positions and draw lines
        const int position(sliderPositionFromValue(sliderOption->minimum,
                                                   sliderOption->maximum,
                                                   current, available) +
                           fudge);
        foreach (const QLine &tickLine, tickLines) {
          if (horizontal)
            painter->drawLine(tickLine.translated(
                upsideDown ? (rect.width() - position) : position, 0));
          else
            painter->drawLine(tickLine.translated(
                0, upsideDown ? (rect.height() - position) : position));
        }

        // go to next position
        current += interval;
      }
    }
  }

  // groove
  if (sliderOption->subControls & SC_SliderGroove) {
    // retrieve groove rect
    auto grooveRect(
        subControlRect(CC_Slider, sliderOption, SC_SliderGroove, widget));

    // base color
    const auto grooveColor(
        _helper->alphaColor(palette.color(QPalette::WindowText), 0.16));

    if (!enabled)
      _helper->renderSliderGroove(painter, grooveRect, grooveColor);
    else {
      const bool upsideDown(sliderOption->upsideDown);

      // handle rect
      auto handleRect(
          subControlRect(CC_Slider, sliderOption, SC_SliderHandle, widget));

      // highlight color
      const auto highlight(KColorUtils::mix(palette.color(QPalette::Highlight),
                                            palette.color(QPalette::Window),
                                            0.25));

      if (sliderOption->orientation == Qt::Horizontal) {
        auto leftRect(grooveRect);
        leftRect.setRight(handleRect.center().x());
        _helper->renderSliderGroove(
            painter, upsideDown ? leftRect.adjusted(0, 1, 0, -1) : leftRect,
            upsideDown ? grooveColor : highlight);

        auto rightRect(grooveRect);
        rightRect.setLeft(handleRect.center().x());
        _helper->renderSliderGroove(
            painter, upsideDown ? rightRect : rightRect.adjusted(0, 1, 0, -1),
            upsideDown ? highlight : grooveColor);

      } else {
        auto topRect(grooveRect);
        topRect.setBottom(handleRect.center().y());
        _helper->renderSliderGroove(
            painter, upsideDown ? topRect.adjusted(1, 0, -1, 0) : topRect,
            upsideDown ? grooveColor : highlight);

        auto bottomRect(grooveRect);
        bottomRect.setTop(handleRect.center().y());
        _helper->renderSliderGroove(
            painter, upsideDown ? bottomRect : bottomRect.adjusted(1, 0, -1, 0),
            upsideDown ? highlight : grooveColor);
      }
    }
  }

  // handle
  if (sliderOption->subControls & SC_SliderHandle) {
    // get rect and center
    auto handleRect(
        subControlRect(CC_Slider, sliderOption, SC_SliderHandle, widget));

    // handle state
    const bool handleActive(sliderOption->activeSubControls & SC_SliderHandle);
    const bool sunken(state & (State_On | State_Sunken));

    const bool hovered(handleActive && mouseOver);
    _animations->widgetStateEngine().updateState(styleObject, AnimationHover,
                                                 hovered);

    const bool isHoverAnimated(
        _animations->widgetStateEngine().isAnimated(styleObject, AnimationHover));
    const qreal hoverOpacity =
        isHoverAnimated
            ? _animations->widgetStateEngine().opacity(styleObject, AnimationHover)
            : (hovered ? 1.0 : 0.0);

    QColor background(palette.color(QPalette::Text));
    QColor outline(palette.color(QPalette::Highlight));

    _helper->renderSliderHandle(painter, handleRect, background, outline,
                                hoverOpacity, sunken);
  }

  return true;
}

bool Style::drawDialComplexControl(const QStyleOptionComplex *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const auto &palette(option->palette);
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));

  // do not render tickmarks
  if (sliderOption->subControls & SC_DialTickmarks) {
  }

  // groove
  if (sliderOption->subControls & SC_DialGroove) {
    // groove rect
    auto grooveRect(
        subControlRect(CC_Dial, sliderOption, SC_SliderGroove, widget));

    // groove
    const auto grooveColor(KColorUtils::mix(palette.color(QPalette::Window),
                                            palette.color(QPalette::WindowText),
                                            0.16));

    // angles
    const qreal first(dialAngle(sliderOption, sliderOption->minimum));
    const qreal last(dialAngle(sliderOption, sliderOption->maximum));

    // render groove
    _helper->renderDialGroove(painter, grooveRect, grooveColor, first, last);

    if (enabled) {
      // highlight
      const auto highlight(KColorUtils::mix(palette.color(QPalette::Highlight),
                                            palette.color(QPalette::Window),
                                            0.25));

      // angles
      const qreal second(dialAngle(sliderOption, sliderOption->sliderPosition));

      // render contents
      _helper->renderDialContents(painter, grooveRect, highlight, first,
                                  second);
    }
  }

  // handle
  if (sliderOption->subControls & SC_DialHandle) {
    // get handle rect
    auto handleRect(
        subControlRect(CC_Dial, sliderOption, SC_DialHandle, widget));
    handleRect = centerRect(handleRect, Metrics::Slider_ControlThickness,
                            Metrics::Slider_ControlThickness);

    // handle state
    const bool handleActive(
        mouseOver &&
        handleRect.contains(_animations->dialEngine().position(widget)));
    const bool sunken(state & (State_On | State_Sunken));

    const bool hovered(handleActive && mouseOver);
    _animations->dialEngine().setHandleRect(widget, handleRect);
    _animations->dialEngine().updateState(widget, AnimationHover, hovered);

    const bool isHoverAnimated(
        _animations->dialEngine().isAnimated(widget, AnimationHover));
    const qreal hoverOpacity =
        isHoverAnimated
            ? _animations->dialEngine().opacity(widget, AnimationHover)
            : (hovered ? 1.0 : 0.0);

    const auto background = palette.color(QPalette::Button);
    const auto outline =
        KColorUtils::mix(background, palette.color(QPalette::WindowText), 0.2);

    _helper->renderSliderHandle(painter, handleRect, background, outline,
                                hoverOpacity, sunken);
  }

  return true;
}

} // namespace BlossomUI

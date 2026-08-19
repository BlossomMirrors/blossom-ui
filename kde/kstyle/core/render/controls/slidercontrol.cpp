// SPDX-License-Identifier: GPL-2.0-or-later
#include "slidercontrol.h"
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "slider.h"

#include <KColorUtils>
#include <QtMath>
#include <QApplication>
#include <QDial>
#include <QPainter>
#include <QPainterPath>
#include <QSlider>
#include <QStyleOptionSlider>

namespace BlossomUI {

bool Render::SliderControl::drawSliderComplexControl(const QStyleOptionComplex *option,
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
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool mouseOver(enabled && (state & QStyle::State_MouseOver));

  const QObject *styleObject(widget ? widget : option->styleObject);
  _style->isQtQuickControl(option, widget);

  // direction
  const bool horizontal(sliderOption->orientation == Qt::Horizontal);

  // tickmarks
  if (sliderOption->subControls & QStyle::SC_SliderTickmarks) {
    const bool upsideDown(sliderOption->upsideDown);
    const int tickPosition(sliderOption->tickPosition);
    const int available(_style->pixelMetric(QStyle::PM_SliderSpaceAvailable, option, widget));
    int interval = sliderOption->tickInterval;
    if (interval < 1)
      interval = sliderOption->pageStep;
    if (interval >= 1) {
      const int fudge(_style->pixelMetric(QStyle::PM_SliderLength, option, widget) / 2);
      int current(sliderOption->minimum);

      // store tick lines
      const auto grooveRect(
          _style->subControlRect(QStyle::CC_Slider, sliderOption, QStyle::SC_SliderGroove, widget));
      QList<QLine> tickLines;
      if (horizontal) {
        if (tickPosition & QSlider::TicksAbove)
          tickLines.append(QLine(
              rect.left(), grooveRect.top() - Render::Slider_TickMarginWidth,
              rect.left(),
              grooveRect.top() - Render::Slider_TickMarginWidth -
                  Render::Slider_TickLength));
        if (tickPosition & QSlider::TicksBelow)
          tickLines.append(
              QLine(rect.left(),
                    grooveRect.bottom() + Render::Slider_TickMarginWidth,
                    rect.left(),
                    grooveRect.bottom() + Render::Slider_TickMarginWidth +
                        Render::Slider_TickLength));

      } else {
        if (tickPosition & QSlider::TicksAbove)
          tickLines.append(QLine(
              grooveRect.left() - Render::Slider_TickMarginWidth, rect.top(),
              grooveRect.left() - Render::Slider_TickMarginWidth -
                  Render::Slider_TickLength,
              rect.top()));
        if (tickPosition & QSlider::TicksBelow)
          tickLines.append(QLine(
              grooveRect.right() + Render::Slider_TickMarginWidth, rect.top(),
              grooveRect.right() + Render::Slider_TickMarginWidth +
                  Render::Slider_TickLength,
              rect.top()));
      }

      // colors
      const auto base(_style->_helper->separatorColor(palette));
      const auto &highlight = palette.color(QPalette::Highlight);

      while (current <= sliderOption->maximum) {
        // adjust color
        const auto color((enabled && current <= sliderOption->sliderPosition)
                             ? highlight
                             : base);
        painter->setPen(color);

        // calculate positions and draw lines
        const int position(_style->sliderPositionFromValue(sliderOption->minimum,
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
  if (sliderOption->subControls & QStyle::SC_SliderGroove) {
    // retrieve groove rect
    auto grooveRect(
        _style->subControlRect(QStyle::CC_Slider, sliderOption, QStyle::SC_SliderGroove, widget));

    // base color
    const auto grooveColor(Render::sliderGrooveFill(_style->_helper, palette).brush.color());

    if (!enabled)
      _style->_helper->renderSliderGroove(painter, grooveRect, grooveColor);
    else {
      const bool upsideDown(sliderOption->upsideDown);

      // handle rect
      auto handleRect(
          _style->subControlRect(QStyle::CC_Slider, sliderOption, QStyle::SC_SliderHandle, widget));

      // highlight color
      const auto highlight(Render::sliderGrooveHighlight(palette).brush.color());

      if (sliderOption->orientation == Qt::Horizontal) {
        auto leftRect(grooveRect);
        leftRect.setRight(handleRect.center().x());
        _style->_helper->renderSliderGroove(
            painter, upsideDown ? leftRect.adjusted(0, 1, 0, -1) : leftRect,
            upsideDown ? grooveColor : highlight);

        auto rightRect(grooveRect);
        rightRect.setLeft(handleRect.center().x());
        _style->_helper->renderSliderGroove(
            painter, upsideDown ? rightRect : rightRect.adjusted(0, 1, 0, -1),
            upsideDown ? highlight : grooveColor);

      } else {
        auto topRect(grooveRect);
        topRect.setBottom(handleRect.center().y());
        _style->_helper->renderSliderGroove(
            painter, upsideDown ? topRect.adjusted(1, 0, -1, 0) : topRect,
            upsideDown ? grooveColor : highlight);

        auto bottomRect(grooveRect);
        bottomRect.setTop(handleRect.center().y());
        _style->_helper->renderSliderGroove(
            painter, upsideDown ? bottomRect : bottomRect.adjusted(1, 0, -1, 0),
            upsideDown ? highlight : grooveColor);
      }
    }
  }

  // handle
  if (sliderOption->subControls & QStyle::SC_SliderHandle) {
    // get rect and center
    auto handleRect(
        _style->subControlRect(QStyle::CC_Slider, sliderOption, QStyle::SC_SliderHandle, widget));

    // handle state
    const bool handleActive(sliderOption->activeSubControls & QStyle::SC_SliderHandle);
    const bool sunken(state & (QStyle::State_On | QStyle::State_Sunken));

    const bool hovered(handleActive && mouseOver);
    _style->_animations->widgetStateEngine().updateState(styleObject, AnimationHover,
                                                 hovered);

    const bool isHoverAnimated(
        _style->_animations->widgetStateEngine().isAnimated(styleObject, AnimationHover));
    const qreal hoverOpacity =
        isHoverAnimated
            ? _style->_animations->widgetStateEngine().opacity(styleObject, AnimationHover)
            : (hovered ? 1.0 : 0.0);

    QColor background(Render::sliderHandleFill(palette).brush.color());
    QColor outline(Render::sliderHandleOutline(palette).brush.color());

    _style->_helper->renderSliderHandle(painter, handleRect, background, outline,
                                hoverOpacity, sunken);
  }

  return true;
}

bool Render::SliderControl::drawDialComplexControl(const QStyleOptionComplex *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const auto &palette(option->palette);
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool mouseOver(enabled && (state & QStyle::State_MouseOver));

  // do not render tickmarks
  if (sliderOption->subControls & QStyle::SC_DialTickmarks) {
  }

  // groove
  if (sliderOption->subControls & QStyle::SC_DialGroove) {
    // groove rect
    auto grooveRect(
        _style->subControlRect(QStyle::CC_Dial, sliderOption, QStyle::SC_SliderGroove, widget));

    // groove
    const auto grooveColor(Render::dialGrooveFill(palette).brush.color());

    // angles
    const qreal first(_style->dialAngle(sliderOption, sliderOption->minimum));
    const qreal last(_style->dialAngle(sliderOption, sliderOption->maximum));

    // render groove
    _style->_helper->renderDialGroove(painter, grooveRect, grooveColor, first, last);

    if (enabled) {
      // highlight
      const auto highlight(Render::dialHighlight(palette).brush.color());

      // angles
      const qreal second(_style->dialAngle(sliderOption, sliderOption->sliderPosition));

      // render contents
      _style->_helper->renderDialContents(painter, grooveRect, highlight, first,
                                  second);
    }
  }

  // handle
  if (sliderOption->subControls & QStyle::SC_DialHandle) {
    // get handle rect
    auto handleRect(
        _style->subControlRect(QStyle::CC_Dial, sliderOption, QStyle::SC_DialHandle, widget));
    handleRect = _style->centerRect(handleRect, Render::Slider_ControlThickness,
                            Render::Slider_ControlThickness);

    // handle state
    const bool handleActive(
        mouseOver &&
        handleRect.contains(_style->_animations->dialEngine().position(widget)));
    const bool sunken(state & (QStyle::State_On | QStyle::State_Sunken));

    const bool hovered(handleActive && mouseOver);
    _style->_animations->dialEngine().setHandleRect(widget, handleRect);
    _style->_animations->dialEngine().updateState(widget, AnimationHover, hovered);

    const bool isHoverAnimated(
        _style->_animations->dialEngine().isAnimated(widget, AnimationHover));
    const qreal hoverOpacity =
        isHoverAnimated
            ? _style->_animations->dialEngine().opacity(widget, AnimationHover)
            : (hovered ? 1.0 : 0.0);

    const auto background = palette.color(QPalette::Button);
    const auto outline = Render::dialHandleOutline(palette).brush.color();

    _style->_helper->renderSliderHandle(painter, handleRect, background, outline,
                                hoverOpacity, sunken);
  }

  return true;
}

void Helper::renderSliderGroove(QPainter *painter, const QRect &rect,
                                const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * static_cast<qreal>(Render::Slider_GrooveThickness));

  // content
  if (color.isValid()) {
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

void Helper::renderDialGroove(QPainter *painter, const QRect &rect,
                              const QColor &color, qreal first,
                              qreal last) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  // content
  if (color.isValid()) {
    const qreal penWidth(Render::Slider_GrooveThickness - 2);
    const QRectF grooveRect(rect.adjusted(penWidth / 2, penWidth / 2,
                                          -penWidth / 2, -penWidth / 2));

    // setup angles
    const int angleStart(first * 180 * 16 / M_PI);
    const int angleSpan((last - first) * 180 * 16 / M_PI);

    // setup pen
    if (angleSpan != 0) {
      QPen pen(color, penWidth);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawArc(grooveRect, angleStart, angleSpan);
    }
  }
}

void Helper::renderDialContents(QPainter *painter, const QRect &rect,
                                const QColor &color, qreal first,
                                qreal second) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  // content
  if (color.isValid()) {
    // setup groove rect
    const qreal penWidth(Render::Slider_GrooveThickness);
    const QRectF grooveRect(rect.adjusted(penWidth / 2, penWidth / 2,
                                          -penWidth / 2, -penWidth / 2));

    // setup angles
    const int angleStart(first * 180 * 16 / M_PI);
    const int angleSpan((second - first) * 180 * 16 / M_PI);

    // setup pen
    if (angleSpan != 0) {
      QPen pen(color, penWidth);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawArc(grooveRect, angleStart, angleSpan);
    }
  }
}

void Helper::renderSliderHandle(QPainter *painter, const QRect &rect,
                                const QColor &color, const QColor &outline,
                                qreal hoverOpacity, bool sunken) const {
  painter->setRenderHint(QPainter::Antialiasing, true);

  // hover circle behind handle: grows from handle size outward
  if (hoverOpacity > 0.0 && outline.isValid()) {
    constexpr qreal maxExpand = Render::Slider_HoverMargin;
    const qreal expand = maxExpand * hoverOpacity;
    QRectF hoverRect = QRectF(rect).adjusted(-expand, -expand, expand, expand);
    QColor hoverColor(outline);
    hoverColor.setAlpha(qRound(50 * hoverOpacity));
    painter->setBrush(hoverColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(hoverRect);
  }

  // handle circle on top
  QRectF frameRect(rect);
  frameRect.adjust(3, 3, -3, -3);
  if (sunken)
    frameRect.translate(0, 2);

  QColor fill(color.isValid() ? color : Qt::transparent);
  if (sunken)
    fill = fill.darker(103);
  if (fill.isValid())
    fill.setAlpha(255);
  painter->setBrush(fill);
  if (outline.isValid())
    painter->setPen(QPen(outline, 1));
  else
    painter->setPen(Qt::NoPen);
  painter->drawEllipse(frameRect);
}
bool Style::drawSliderComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::SliderControl(this).drawSliderComplexControl(option, painter, widget);
}

bool Style::drawDialComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::SliderControl(this).drawDialComplexControl(option, painter, widget);
}
} // namespace BlossomUI

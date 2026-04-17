// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <KColorUtils>
#include <QApplication>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionComboBox>

namespace BlossomUI {

bool Style::drawComboBoxLabelControl(const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const {
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption)
    return false;
  if (comboBoxOption->editable)
    return false;

  // need to alter palette for focused buttons
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool sunken(state & (State_On | State_Sunken));
  const bool mouseOver(enabled && (option->state & State_MouseOver));
  const bool hasFocus(enabled && !mouseOver &&
                      (option->state & State_HasFocus));
  const bool flat(!comboBoxOption->frame);

  QPalette::ColorRole textRole;
  if (flat) {
    if (sunken)
      textRole = QPalette::HighlightedText;
    else
      textRole = QPalette::WindowText;

  } else if (sunken || mouseOver)
    textRole = QPalette::HighlightedText;
  else
    textRole = QPalette::ButtonText;

  // change pen color directly
  painter->setPen(QPen(option->palette.color(textRole), 1));

  // translate painter for pressed down comboboxes
  if (sunken && !flat) {
    painter->translate(1, 1);
  }

  if (const auto cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
    auto editRect =
        proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
    painter->save();
    painter->setClipRect(editRect);
    if (!cb->currentIcon.isNull()) {
      QIcon::Mode mode;

      if (!enabled)
        mode = QIcon::Disabled;
      else if (!flat && hasFocus)
        mode = QIcon::Selected;
      else if (mouseOver && flat)
        mode = QIcon::Active;
      else
        mode = QIcon::Normal;

      const qreal dpr = painter->device()
                            ? painter->device()->devicePixelRatioF()
                            : qApp->devicePixelRatio();
      const QPixmap pixmap = _helper->coloredIcon(cb->currentIcon, cb->palette,
                                                  cb->iconSize, dpr, mode);
      auto iconRect(editRect);
      iconRect.setWidth(cb->iconSize.width() + 4);
      iconRect = alignedRect(cb->direction, Qt::AlignLeft | Qt::AlignVCenter,
                             iconRect.size(), editRect);
      if (cb->editable)
        painter->fillRect(iconRect, option->palette.brush(QPalette::Base));
      proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

      if (cb->direction == Qt::RightToLeft)
        editRect.translate(-4 - cb->iconSize.width(), 0);
      else
        editRect.translate(cb->iconSize.width() + 4, 0);
    }
    if (!cb->currentText.isEmpty() && !cb->editable) {
      proxy()->drawItemText(
          painter, editRect.adjusted(1, 0, -1, 0),
          visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
          cb->palette, cb->state & State_Enabled, cb->currentText);
    }
    painter->restore();
  }

  return true;
}
bool Style::drawComboBoxComplexControl(const QStyleOptionComplex *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption)
    return true;

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  // rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool hasFocus(enabled && (state & (State_HasFocus | State_Sunken)));
  const bool editable(comboBoxOption->editable);
  const bool sunken(state & (State_On | State_Sunken));
  bool flat(!comboBoxOption->frame);

  // frame
  if (option->subControls & SC_ComboBoxFrame) {
    if (editable) {
      flat |= (rect.height() <= 2 * Metrics::Frame_FrameWidth +
                                    Metrics::MenuButton_IndicatorWidth);
      if (flat) {
        const auto &background = palette.color(QPalette::Base);

        painter->setBrush(background);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);

      } else {
        drawPrimitive(PE_FrameLineEdit, option, painter, widget);
      }

    } else {
      // update animation state
      // hover takes precedence over focus
      _animations->inputWidgetEngine().updateState(widget, AnimationHover,
                                                   mouseOver);
      _animations->inputWidgetEngine().updateState(widget, AnimationFocus,
                                                   hasFocus && !mouseOver);
      const AnimationMode mode(
          _animations->inputWidgetEngine().buttonAnimationMode(widget));
      const qreal opacity(
          _animations->inputWidgetEngine().buttonOpacity(widget));

      if (flat) {
        // define colors and render
        const auto color(_helper->toolButtonColor(palette, mouseOver, hasFocus,
                                                  sunken, opacity, mode));
        _helper->renderToolButtonFrame(painter, rect, color, sunken);

      } else {
        // define colors - don't use accent for focus on combobox trigger (too
        // prominent when item selected)
        const auto background(_helper->buttonBackgroundColor(
            palette, mouseOver, false, false, opacity, mode));

        // render
        _helper->renderButtonFrame(painter, rect, background, palette, false,
                                   sunken, mouseOver, enabled, windowActive);
      }
    }
  }

  // arrow
  if (option->subControls & SC_ComboBoxArrow) {
    // detect empty comboboxes
    const auto comboBox = qobject_cast<const QComboBox *>(widget);
    const bool empty(comboBox && !comboBox->count());

    // arrow color
    QColor arrowColor;
    if (editable) {
      if (empty || !enabled)
        arrowColor = palette.color(QPalette::Disabled, QPalette::Text);
      else {
        // check animation state
        const bool subControlHover(enabled && mouseOver &&
                                   comboBoxOption->activeSubControls &
                                       SC_ComboBoxArrow);
        _animations->comboBoxEngine().updateState(widget, AnimationHover,
                                                  subControlHover);

        arrowColor = _helper->arrowColor(palette, QPalette::WindowText);
      }

    } else if (flat) {
      if (empty || !enabled)
        arrowColor = _helper->arrowColor(palette, QPalette::Disabled,
                                         QPalette::WindowText);
      else if (hasFocus && !mouseOver && sunken)
        arrowColor = palette.color(QPalette::HighlightedText);
      else
        arrowColor = _helper->arrowColor(palette, QPalette::WindowText);

    } else if (empty || !enabled)
      arrowColor = _helper->arrowColor(palette, QPalette::Disabled,
                                       QPalette::ButtonText);
    // arrow in dropdown menu shoud use light color (highlightedtext) upon
    // mouseover
    else if (hasFocus)
      arrowColor = palette.color(QPalette::HighlightedText);
    // arrow in combo menu click should use light color in the On state
    // (checked)
    else if (state & State_On)
      arrowColor = palette.color(QPalette::HighlightedText);
    else
      arrowColor = _helper->arrowColor(palette, QPalette::ButtonText);

    // arrow rect
    auto arrowRect(
        subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget));

    // translate for non editable, non flat, sunken comboboxes
    if (sunken && !flat && !editable)
      arrowRect.translate(1, 1);

    // render
    arrowRect.translate(-3, 0);
    _helper->renderArrow(painter, arrowRect, arrowColor, ArrowDown);
  }

  return true;
}

} // namespace BlossomUI

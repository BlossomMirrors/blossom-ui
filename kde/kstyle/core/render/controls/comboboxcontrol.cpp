// SPDX-License-Identifier: GPL-2.0-or-later
#include "comboboxcontrol.h"
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "button.h"
#include "private.h"
#include "widgetrenderer.h"

#include <KColorUtils>
#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionComboBox>

namespace BlossomUI {

bool Render::ComboBoxControl::drawComboBoxLabelControl(const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const {
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption)
    return false;
  if (comboBoxOption->editable)
    return false;

  // need to alter palette for focused buttons
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool sunken(state & (QStyle::State_On | QStyle::State_Sunken));
  const bool mouseOver(enabled && (option->state & QStyle::State_MouseOver));
  const bool hasFocus(enabled && !mouseOver &&
                      (option->state & QStyle::State_HasFocus));
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
        _style->proxy()->subControlRect(QStyle::CC_ComboBox, cb, QStyle::SC_ComboBoxEditField, widget);
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
      const QPixmap pixmap = _style->_helper->coloredIcon(cb->currentIcon, cb->palette,
                                                  cb->iconSize, dpr, mode);
      auto iconRect(editRect);
      iconRect.setWidth(cb->iconSize.width() + 4);
      iconRect = _style->alignedRect(cb->direction, Qt::AlignLeft | Qt::AlignVCenter,
                             iconRect.size(), editRect);
      if (cb->editable)
        painter->fillRect(iconRect, option->palette.brush(QPalette::Base));
      _style->proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

      if (cb->direction == Qt::RightToLeft)
        editRect.translate(-4 - cb->iconSize.width(), 0);
      else
        editRect.translate(cb->iconSize.width() + 4, 0);
    }
    if (!cb->currentText.isEmpty() && !cb->editable) {
      _style->proxy()->drawItemText(
          painter, editRect.adjusted(1, 0, -1, 0),
          _style->visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
          cb->palette, cb->state & QStyle::State_Enabled, cb->currentText);
    }
    painter->restore();
  }

  return true;
}
bool Render::ComboBoxControl::drawComboBoxComplexControl(const QStyleOptionComplex *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption)
    return true;

  // rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // state
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);
  const bool mouseOver(enabled && (state & QStyle::State_MouseOver));
  const bool hasFocus(enabled && (state & (QStyle::State_HasFocus | QStyle::State_Sunken)));
  const bool editable(comboBoxOption->editable);
  const bool sunken(state & (QStyle::State_On | QStyle::State_Sunken));
  bool flat(!comboBoxOption->frame);

  // frame
  if (option->subControls & QStyle::SC_ComboBoxFrame) {
    if (editable) {
      flat |= (rect.height() <= 2 * Render::Frame_FrameWidth +
                                    Render::MenuButton_IndicatorWidth);
      if (flat) {
        const auto &background = palette.color(QPalette::Base);

        painter->setBrush(background);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);

      } else {
        _style->drawPrimitive(QStyle::PE_FrameLineEdit, option, painter, widget);
      }

    } else {
      // update animation state
      // hover takes precedence over focus
      _style->_animations->inputWidgetEngine().updateState(widget, AnimationHover,
                                                   mouseOver);
      _style->_animations->inputWidgetEngine().updateState(widget, AnimationFocus,
                                                   hasFocus && !mouseOver);
      const AnimationMode mode(
          _style->_animations->inputWidgetEngine().buttonAnimationMode(widget));
      const qreal opacity(
          _style->_animations->inputWidgetEngine().buttonOpacity(widget));

      Render::WidgetInteractionState wstate;
      wstate.enabled = enabled;
      wstate.pressed = sunken;
      wstate.hovered = mouseOver;
      wstate.focused = flat && hasFocus;
      wstate.mode = mode;
      wstate.opacity = opacity;
      wstate.palette = palette;

      // no accent-tinted focus on a non-flat combobox trigger - too
      // prominent once an item is selected
      const Render::WidgetSpec spec =
          flat ? Render::toolButtonFrame(_style->_helper, wstate)
               : Render::buttonFrame(_style->_helper, wstate);
      Render::WidgetRenderer(_style->_helper).render(painter, rect, spec, wstate);
    }
  }

  // arrow
  if (option->subControls & QStyle::SC_ComboBoxArrow) {
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
                                       QStyle::SC_ComboBoxArrow);
        _style->_animations->comboBoxEngine().updateState(widget, AnimationHover,
                                                  subControlHover);

        arrowColor = _style->_helper->arrowColor(palette, QPalette::WindowText);
      }

    } else if (flat) {
      if (empty || !enabled)
        arrowColor = _style->_helper->arrowColor(palette, QPalette::Disabled,
                                         QPalette::WindowText);
      else if (hasFocus && !mouseOver && sunken)
        arrowColor = palette.color(QPalette::HighlightedText);
      else
        arrowColor = _style->_helper->arrowColor(palette, QPalette::WindowText);

    } else if (empty || !enabled)
      arrowColor = _style->_helper->arrowColor(palette, QPalette::Disabled,
                                       QPalette::ButtonText);
    // arrow in dropdown menu shoud use light color (highlightedtext) upon
    // mouseover
    else if (hasFocus)
      arrowColor = palette.color(QPalette::HighlightedText);
    // arrow in combo menu click should use light color in the On state
    // (checked)
    else if (state & QStyle::State_On)
      arrowColor = palette.color(QPalette::HighlightedText);
    else
      arrowColor = _style->_helper->arrowColor(palette, QPalette::ButtonText);

    // arrow rect
    auto arrowRect(
        _style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxArrow, widget));

    // translate for non editable, non flat, sunken comboboxes
    if (sunken && !flat && !editable)
      arrowRect.translate(1, 1);

    // render
    arrowRect.translate(-3, 0);
    _style->_helper->renderArrow(painter, arrowRect, arrowColor, ArrowDown);
  }

  return true;
}

//* QComboBox: event filter, hover
void Style::polishComboBoxHover(QWidget *widget) {
  if (!qobject_cast<QComboBox *>(widget))
    return;
  addEventFilter(widget);
  widget->setAttribute(Qt::WA_Hover);
}

//* remove auto background from combobox popup
void Style::polishComboBoxPopupViewBackground(QWidget *widget) {
  auto view = const_cast<QAbstractItemView *>(itemViewParent(widget));
  if (!view)
    return;
  QWidget *w = view;
  while (w) {
    if (qobject_cast<QComboBox *>(w)) {
      view->setAutoFillBackground(false);
      view->viewport()->setAutoFillBackground(false);
      break;
    }
    w = w->parentWidget();
  }
}

//* child of QComboBoxListView
bool Style::polishComboBoxListViewChild(QWidget *widget) {
  if (!(widget->parent() && widget->parent()->inherits("QComboBoxListView")))
    return false;
  widget->setAutoFillBackground(false);
  return true;
}

//* QComboBox: replace default item delegate
bool Style::polishComboBox(QWidget *widget) {
  auto comboBox = qobject_cast<QComboBox *>(widget);
  if (!comboBox)
    return false;

  if (!hasParent(widget, "QWebView")) {
    auto itemView(comboBox->view());
    if (itemView && itemView->itemDelegate()) {
      const QByteArray delegateClass =
          itemView->itemDelegate()->metaObject()->className();
      const bool isDefaultDelegate =
          itemView->itemDelegate()->inherits("QComboBoxDelegate") ||
          delegateClass == "QStyledItemDelegate";
      if (isDefaultDelegate) {
        itemView->setItemDelegate(
            new BlossomUIPrivate::ComboBoxItemDelegate(itemView));
      }
    }
  }
  return true;
}

//* QComboBoxPrivateContainer (the popup window)
bool Style::polishComboBoxPopupContainer(QWidget *widget) {
  if (!widget->inherits("QComboBoxPrivateContainer"))
    return false;
  addEventFilter(widget);
  setTranslucentBackground(widget);
  return true;
}
bool Style::drawComboBoxLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ComboBoxControl(this).drawComboBoxLabelControl(option, painter, widget);
}

bool Style::drawComboBoxComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::ComboBoxControl(this).drawComboBoxComplexControl(option, painter, widget);
}
} // namespace BlossomUI

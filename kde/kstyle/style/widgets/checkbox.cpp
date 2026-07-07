// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuihelper.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "widgets/switch.h"

#include <KColorUtils>
#include <QCheckBox>
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>
#include <QVariantAnimation>

BlossomUISwitchWidget::BlossomUISwitchWidget(BlossomUI::Helper *helper,
                                             QCheckBox *parent)
    : QWidget(parent), _helper(helper), _checkBox(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents, false);
  setAttribute(Qt::WA_NoMousePropagation, false);
  setFocusPolicy(Qt::NoFocus);
  _checked = parent->isChecked();
  _visualPos = _checked ? 1.0 : 0.0;
  _anim = new QVariantAnimation(this);
  _anim->setDuration(200);
  _anim->setEasingCurve(QEasingCurve::OutBack);
  connect(_anim, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &value) {
            _visualPos = value.toReal();
            update();
          });
  setCursor(Qt::PointingHandCursor);
  connect(parent, &QCheckBox::toggled, this,
          &BlossomUISwitchWidget::updateFromParent);
}

void BlossomUISwitchWidget::setChecked(bool on) {
  if (_checked != on) {
    _checked = on;
    if (_checkBox && _checkBox->isChecked() != on)
      _checkBox->setChecked(on);
    if (!_dragging)
      animateTo(on ? 1.0 : 0.0);
    update();
    emit toggled(on);
  }
}

void BlossomUISwitchWidget::animateTo(qreal target) {
  _anim->stop();
  if (qFuzzyCompare(_visualPos, target))
    return;
  _anim->setStartValue(_visualPos);
  _anim->setEndValue(target);
  _anim->start();
}

void BlossomUISwitchWidget::updateFromParent() {
  if (_checkBox && _checked != _checkBox->isChecked()) {
    _checked = _checkBox->isChecked();
    if (!_dragging)
      animateTo(_checked ? 1.0 : 0.0);
    update();
  }
}

bool BlossomUISwitchWidget::hitTrack(const QPoint &pos) const {
  return rect().contains(pos);
}

void BlossomUISwitchWidget::paintEvent(QPaintEvent *) {
  if (!_helper || !_checkBox)
    return;
  QPainter p(this);
  const QPalette &palette = _checkBox->palette();
  _helper->renderSwitch(&p, rect(), palette, false, _hover,
                        BlossomUI::CheckAnimated, _visualPos);
}

void BlossomUISwitchWidget::mousePressEvent(QMouseEvent *e) {
  if (e->button() != Qt::LeftButton)
    return;
  e->accept();
  _pressed = true;
  _pressPos = e->pos();
  _dragging = false;
  update();
}

void BlossomUISwitchWidget::mouseMoveEvent(QMouseEvent *e) {
  if (!(e->buttons() & Qt::LeftButton)) {
    _pressed = false;
    _dragging = false;
    update();
    return;
  }
  e->accept();
  if (!_dragging) {
    if ((e->pos() - _pressPos).manhattanLength() >= _dragThreshold)
      _dragging = true;
    else {
      update();
      return;
    }
  }
  // thumb follows the pointer while dragging
  _anim->stop();
  const qreal thumbSpan = height();
  qreal frac = width() > thumbSpan
      ? (e->pos().x() - thumbSpan / 2.0) / (width() - thumbSpan)
      : 0.5;
  if (_checkBox->layoutDirection() == Qt::RightToLeft)
    frac = 1.0 - frac;
  _visualPos = qBound(0.0, frac, 1.0);

  const int cx = width() / 2;
  const bool on = e->pos().x() >= cx;
  if (_checkBox->layoutDirection() == Qt::RightToLeft) {
    if (_checked != !on)
      setChecked(!on);
  } else {
    if (_checked != on)
      setChecked(on);
  }
  update();
}

void BlossomUISwitchWidget::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() != Qt::LeftButton)
    return;
  e->accept();
  const bool wasDragging = _dragging;
  if (!wasDragging && hitTrack(e->pos()))
    setChecked(!_checked);
  _pressed = false;
  _dragging = false;
  if (wasDragging)
    animateTo(_checked ? 1.0 : 0.0);
  update();
}

bool BlossomUISwitchWidget::event(QEvent *e) {
  if (e->type() == QEvent::Enter) {
    _hover = true;
    update();
    return true;
  }
  if (e->type() == QEvent::Leave) {
    _hover = false;
    _pressed = false;
    _dragging = false;
    update();
    return true;
  }
  return QWidget::event(e);
}

namespace BlossomUI {

bool Style::drawCheckBoxLabelControl(const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const {
  // cast option and check
  const auto buttonOption(
      qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return true;

  // copy palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  // store state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);

  // text alignment
  const bool reverseLayout(option->direction == Qt::RightToLeft);
  const int textFlags(_mnemonics->textFlags() | Qt::AlignVCenter |
                      (reverseLayout ? Qt::AlignRight : Qt::AlignLeft));

  // text rect
  auto textRect(rect);

  // render icon
  if (!buttonOption->icon.isNull()) {
    const QIcon::Mode mode(enabled ? QIcon::Normal : QIcon::Disabled);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap(
        _helper->coloredIcon(buttonOption->icon, buttonOption->palette,
                             buttonOption->iconSize, dpr, mode));
    drawItemPixmap(painter, rect, textFlags, pixmap);

    // adjust rect (copied from QCommonStyle)
    textRect.setLeft(textRect.left() + buttonOption->iconSize.width() + 4);
    textRect = visualRect(option, textRect);
  }

  // render text
  if (!buttonOption->text.isEmpty()) {
    textRect = option->fontMetrics.boundingRect(textRect, textFlags,
                                                buttonOption->text);
    drawItemText(painter, textRect, textFlags, palette, enabled,
                 buttonOption->text, QPalette::WindowText);

    // check focus state
    const bool hasFocus(enabled && (state & State_HasFocus));

    // update animation state
    _animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                                 hasFocus);
    const bool isFocusAnimated(
        _animations->widgetStateEngine().isAnimated(widget, AnimationFocus));
    const qreal opacity(
        _animations->widgetStateEngine().opacity(widget, AnimationFocus));

    // focus color
    QColor focusColor;
    if (isFocusAnimated)
      focusColor = _helper->alphaColor(_helper->focusColor(palette), opacity);
    else if (hasFocus)
      focusColor = _helper->focusColor(palette);

    // render focus
    _helper->renderFocusLine(painter, textRect, focusColor);
  }

  return true;
}

} // namespace BlossomUI

// SPDX-License-Identifier: GPL-2.0-or-later
#include "checkboxcontrol.h"
#include "blossomuianimations.h"
#include "blossomuihelper.h"
#include "blossomuimnemonics.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "checkbox.h"
#include "radiobutton.h"
#include "switch.h"
#include "switchwidget.h"
#include "widgetrenderer.h"

#include <KColorUtils>
#include <QCheckBox>
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QRadioButton>
#include <QStyleOptionButton>
#include <QTimer>
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

bool Render::CheckBoxControl::drawCheckBoxLabelControl(const QStyleOption *option,
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
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);

  // text alignment
  const bool reverseLayout(option->direction == Qt::RightToLeft);
  const int textFlags(_style->_mnemonics->textFlags() | Qt::AlignVCenter |
                      (reverseLayout ? Qt::AlignRight : Qt::AlignLeft));

  // text rect
  auto textRect(rect);

  // render icon
  if (!buttonOption->icon.isNull()) {
    const QIcon::Mode mode(enabled ? QIcon::Normal : QIcon::Disabled);
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF()
                                        : qApp->devicePixelRatio();
    const QPixmap pixmap(
        _style->_helper->coloredIcon(buttonOption->icon, buttonOption->palette,
                             buttonOption->iconSize, dpr, mode));
    _style->drawItemPixmap(painter, rect, textFlags, pixmap);

    // adjust rect (copied from QCommonStyle)
    textRect.setLeft(textRect.left() + buttonOption->iconSize.width() + 4);
    textRect = _style->visualRect(option, textRect);
  }

  // render text
  if (!buttonOption->text.isEmpty()) {
    textRect = option->fontMetrics.boundingRect(textRect, textFlags,
                                                buttonOption->text);
    _style->drawItemText(painter, textRect, textFlags, palette, enabled,
                 buttonOption->text, QPalette::WindowText);

    // check focus state
    const bool hasFocus(enabled && (state & QStyle::State_HasFocus));

    // update animation state
    _style->_animations->widgetStateEngine().updateState(widget, AnimationFocus,
                                                 hasFocus);
    const bool isFocusAnimated(
        _style->_animations->widgetStateEngine().isAnimated(widget, AnimationFocus));
    const qreal opacity(
        _style->_animations->widgetStateEngine().opacity(widget, AnimationFocus));

    // focus color
    QColor focusColor;
    if (isFocusAnimated)
      focusColor = _style->_helper->alphaColor(_style->_helper->focusColor(palette), opacity);
    else if (hasFocus)
      focusColor = _style->_helper->focusColor(palette);

    // render focus
    _style->_helper->renderFocusLine(painter, textRect, focusColor);
  }

  return true;
}

void Helper::renderCheckBox(QPainter *painter, const QRect &rect,
                            const QPalette &palette, const bool isInMenu,
                            bool sunken, const bool mouseOver,
                            CheckBoxState state, const bool windowActive,
                            qreal animation) const {
  const bool on = (state == CheckOn || state == CheckAnimated);
  const bool partial = (state == CheckPartial);
  const qreal anim = (state == CheckAnimated) ? (animation == -1 ? 1.0 : animation) : -1.0;

  const Render::Fill fill =
      Render::checkBoxFill(palette, on, partial, mouseOver, isInMenu, anim);
  const Render::Border border = Render::checkBoxBorder(palette, fill, mouseOver, anim);

  Render::WidgetSpec spec = Render::CheckBoxBoxSpec;
  spec.geom.fixedRadius(qRound(frameRadius(PenWidth::NoPen) / 2.0) + spec.geom.inset);
  spec.fill(Render::StateStyle<Render::Fill>(fill));
  spec.border(border);

  Render::WidgetInteractionState wstate;
  wstate.enabled = true;
  wstate.palette = palette;

  const QRect boxRect = sunken ? rect.translated(1, 1) : rect;
  Render::WidgetRenderer renderer(this);
  renderer.render(painter, boxRect, spec, wstate);

  const QRectF frameRect = renderer.contentRect(boxRect, spec, wstate);
  const QColor color(palette.color(QPalette::HighlightedText));
  const qreal x = frameRect.x(), y = frameRect.y();
  painter->setRenderHint(QPainter::Antialiasing, true);

  if (state == CheckOn) {
    QPen pen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    QPainterPath check;
    check.moveTo(5 + x, 8.5 + y);
    check.lineTo(7 + x, 11 + y);
    check.lineTo(12 + x, 5 + y);
    painter->drawPath(check);

  } else if (state == CheckPartial) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(QRectF(3 + x, 7 + y, 2, 2));
    painter->drawEllipse(QRectF(7 + x, 7 + y, 2, 2));
    painter->drawEllipse(QRectF(11 + x, 7 + y, 2, 2));

  } else if (state == CheckAnimated) {
    QPen pen(alphaColor(color, anim), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    QPainterPath check;
    check.moveTo(5 + x, 8.5 + y);
    check.lineTo(5 + 2 * anim + x, 8.5 + 2.5 * anim + y);
    check.lineTo(7 + 5 * anim + x, 11 - 4 * anim * 1.5 + y);
    painter->drawPath(check);
  }
  Q_UNUSED(windowActive)
}

void Helper::renderSwitch(QPainter *painter, const QRect &rect,
                          const QPalette &palette, bool sunken,
                          const bool mouseOver, CheckBoxState state,
                          qreal animation) const {
  // pressed feedback comes from the thumb overshoot animation instead of a
  // sunken offset, matching the org.blossomos.style QML SwitchIndicator
  Q_UNUSED(sunken)
  painter->setRenderHint(QPainter::Antialiasing, true);

  qreal t = 0.0;
  if (state == CheckOn)
    t = 1.0;
  else if (state == CheckAnimated)
    t = (animation >= 0) ? qBound(-0.15, animation, 1.15) : 1.0;
  const qreal progress = qBound(0.0, t, 1.0);

  const Render::Fill trackFill = Render::switchTrackFill(palette, mouseOver);
  const Render::Fill trailFill = Render::switchFillTrailFill(palette, mouseOver);
  const Render::Border trackBorder =
      Render::switchTrackBorder(palette, trackFill, trailFill, mouseOver, progress);

  Render::WidgetInteractionState wstate;
  wstate.enabled = true;
  wstate.palette = palette;

  Render::WidgetSpec trackSpec = Render::SwitchTrackSpec;
  const qreal insetHeight = rect.height() - 2.0 * trackSpec.geom.inset;
  trackSpec.geom.fixedRadius(insetHeight / 2.0 + trackSpec.geom.inset);
  trackSpec.fill(Render::StateStyle<Render::Fill>(trackFill));
  trackSpec.border(trackBorder);

  Render::WidgetRenderer renderer(this);
  renderer.render(painter, rect, trackSpec, wstate);

  const QRectF trackRect = renderer.contentRect(rect, trackSpec, wstate);
  const qreal radius = trackRect.height() / 2.0;
  const int margin = Render::Switch_ThumbMargin;
  const qreal thumbDiameter = trackRect.height() - 2 * margin;
  const qreal travel = trackRect.width() - thumbDiameter - 2 * margin;
  const qreal overshoot = 2.0;
  const qreal thumbX =
      trackRect.x() + margin + qBound(-overshoot, t * travel, travel + overshoot);
  QRectF thumbRect(thumbX, trackRect.y() + margin, thumbDiameter, thumbDiameter);

  if (progress > 0.01) {
    QRectF fillRect(trackRect);
    fillRect.setWidth(
        qMin(trackRect.width(), thumbRect.right() + margin - trackRect.x()));
    painter->setPen(Qt::NoPen);
    painter->setBrush(trailFill.brush);
    painter->drawRoundedRect(fillRect, radius, radius);
    // redraw the border on top since the trail fill can cover it
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(trackBorder.color, trackBorder.width));
    painter->drawRoundedRect(trackRect, radius, radius);
  }

  const Render::Fill thumbFill = Render::switchThumbFill(palette);
  const QColor thumbColor = thumbFill.brush.color();
  const QColor thumbBorder = Render::switchThumbBorder(thumbFill, palette).brush.color();
  painter->setPen(QPen(thumbBorder, 1));
  painter->setBrush(thumbColor);
  painter->drawEllipse(thumbRect);

  const qreal glyphScale = thumbDiameter / 16.0;
  const auto gp = [&](qreal px, qreal py) {
    return QPointF((px - 8.0) * 1.35 + 8.0, (py - 8.0) * 1.35 + 8.0) *
               glyphScale +
           thumbRect.topLeft();
  };

  QPen glyphPen(Qt::SolidLine);
  glyphPen.setWidthF(1.5);
  glyphPen.setCapStyle(Qt::RoundCap);
  glyphPen.setJoinStyle(Qt::RoundJoin);
  painter->setBrush(Qt::NoBrush);

  const qreal xOpacity = 1.0 - progress;
  if (xOpacity > 0.01) {
    glyphPen.setColor(Render::switchThumbCrossColor(thumbFill, xOpacity).brush.color());
    painter->setPen(glyphPen);
    painter->drawLine(gp(4.5, 4.5), gp(11.5, 11.5));
    painter->drawLine(gp(11.5, 4.5), gp(4.5, 11.5));
  }
  if (progress > 0.01) {
    glyphPen.setColor(
        Render::switchThumbCheckColor(thumbFill, trailFill, progress).brush.color());
    painter->setPen(glyphPen);
    QPainterPath check;
    check.moveTo(gp(5, 8.5));
    check.lineTo(gp(7, 11));
    check.lineTo(gp(12, 5));
    painter->drawPath(check);
  }
}

void Helper::renderRadioButton(QPainter *painter, const QRect &rect,
                               const QPalette &palette, const bool mouseOver,
                               bool sunken, RadioButtonState state,
                               const bool isInMenu, qreal animation) const {
  const bool on = (state == RadioOn || state == RadioAnimated);
  const qreal anim = (state == RadioAnimated) ? (animation == -1 ? 1.0 : animation) : -1.0;

  const Render::Fill fill = Render::radioButtonFill(palette, on, mouseOver, isInMenu, anim);
  const Render::Border border = Render::radioButtonBorder(palette, fill, mouseOver, anim);

  Render::WidgetSpec spec = Render::RadioButtonSpec;
  spec.fill(Render::StateStyle<Render::Fill>(fill));
  spec.border(border);

  Render::WidgetInteractionState wstate;
  wstate.enabled = true;
  wstate.palette = palette;

  const QRect boxRect = sunken ? rect.translated(1, 1) : rect;
  Render::WidgetRenderer renderer(this);
  renderer.render(painter, boxRect, spec, wstate);

  if (state == RadioOff)
    return;

  const QRectF frameRect = renderer.contentRect(boxRect, spec, wstate);
  const QRectF markerRect(frameRect.adjusted(4, 4, -4, -4));
  const QColor color(palette.color(QPalette::HighlightedText));
  painter->setPen(Qt::NoPen);
  painter->setBrush(state == RadioAnimated ? alphaColor(color, qBound(0.0, anim, 1.0))
                                           : color);
  painter->drawEllipse(markerRect);
}

//* QCheckBox/QRadioButton: event filter, hover
bool Style::polishCheckableHover(QWidget *widget) {
  if (!(qobject_cast<QCheckBox *>(widget) ||
        qobject_cast<QRadioButton *>(widget)))
    return false;
  addEventFilter(widget);
  widget->setAttribute(Qt::WA_Hover);
  return true;
}

//* switch (pill) checkboxes: replace indicator with actual switch widget
//(draggable)
void Style::polishSwitchCheckBox(QWidget *widget) {
  auto checkBox = qobject_cast<QCheckBox *>(widget);
  if (!checkBox || !isSwitchWidget(checkBox))
    return;

  QStyleOptionButton opt;
  opt.initFrom(checkBox);
  opt.rect = checkBox->rect();
  opt.text = checkBox->text();
  opt.icon = checkBox->icon();
  opt.iconSize = checkBox->iconSize();
  QRect indRect = subElementRect(SE_CheckBoxIndicator, &opt, checkBox);
  BlossomUISwitchWidget *overlay =
      new BlossomUISwitchWidget(_helper, checkBox);
  overlay->setParent(checkBox);
  overlay->setGeometry(indRect);
  overlay->show();
  overlay->raise(); // ensure overlay is on top of any other children (e.g.
                    // label)
  checkBox->setProperty("blossomui-switch-overlay",
                        QVariant::fromValue<QObject *>(overlay));
  addEventFilter(checkBox);
  // update geometry after layout (checkbox rect may not be final at polish
  // time)
  QTimer::singleShot(0, checkBox, [this, checkBox]() {
    QObject *ov =
        checkBox->property("blossomui-switch-overlay").value<QObject *>();
    auto *o = qobject_cast<BlossomUISwitchWidget *>(ov);
    if (o) {
      QStyleOptionButton opt;
      opt.initFrom(checkBox);
      opt.rect = checkBox->rect();
      opt.text = checkBox->text();
      opt.icon = checkBox->icon();
      opt.iconSize = checkBox->iconSize();
      o->setGeometry(subElementRect(SE_CheckBoxIndicator, &opt, checkBox));
    }
  });
}

//* switch (pill) checkbox cleanup, mirrors polishSwitchCheckBox
void Style::unpolishSwitchCheckBox(QWidget *widget) {
  auto checkBox = qobject_cast<QCheckBox *>(widget);
  if (!checkBox || !isSwitchWidget(checkBox))
    return;

  QObject *ov =
      checkBox->property("blossomui-switch-overlay").value<QObject *>();
  if (ov) {
    ov->deleteLater();
    checkBox->setProperty("blossomui-switch-overlay", QVariant());
  }
  widget->removeEventFilter(this);
}
bool Style::drawCheckBoxLabelControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::CheckBoxControl(this).drawCheckBoxLabelControl(option, painter, widget);
}
} // namespace BlossomUI

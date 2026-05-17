// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuibutton.h"

#include <KDecoration3/DecoratedWindow>
#include <KIconLoader>

#include <QPainter>
#include <QVariantAnimation>

namespace BlossomUI {

using KDecoration3::DecorationButtonType;

Button::Button(DecorationButtonType type, Decoration *decoration,
               QObject *parent)
    : DecorationButton(type, decoration, parent),
      m_animation(new QVariantAnimation(this)) {
  m_animation->setStartValue(0.0);
  m_animation->setEndValue(1.0);
  m_animation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_animation, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &value) { setOpacity(value.toReal()); });

  connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this,
          SLOT(update()));
  connect(decoration->settings().get(),
          &KDecoration3::DecorationSettings::reconfigured, this,
          &Button::reconfigure);
  connect(this, &KDecoration3::DecorationButton::hoveredChanged, this,
          &Button::updateAnimationState);

  reconfigure();
}

Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(),
             args.at(1).value<Decoration *>(), parent) {
  setGeometry(QRectF(QPointF(0, 0), preferredSize()));
}

Button *Button::create(DecorationButtonType type,
                       KDecoration3::Decoration *decoration, QObject *parent) {
  if (auto d = qobject_cast<Decoration *>(decoration)) {
    Button *b = new Button(type, d, parent);
    const auto c = d->window();
    switch (type) {
    case DecorationButtonType::Close:
      b->setVisible(c->isCloseable());
      QObject::connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b,
                       &BlossomUI::Button::setVisible);
      break;
    case DecorationButtonType::Maximize:
      b->setVisible(c->isMaximizeable());
      QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged,
                       b, &BlossomUI::Button::setVisible);
      break;
    case DecorationButtonType::Minimize:
      b->setVisible(c->isMinimizeable());
      QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged,
                       b, &BlossomUI::Button::setVisible);
      break;
    case DecorationButtonType::ContextHelp:
      b->setVisible(c->providesContextHelp());
      QObject::connect(
          c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b,
          &BlossomUI::Button::setVisible);
      break;
    case DecorationButtonType::Shade:
      b->setVisible(c->isShadeable());
      QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b,
                       &BlossomUI::Button::setVisible);
      break;
    case DecorationButtonType::Menu:
      QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b,
                       [b]() { b->update(); });
      break;
    default:
      break;
    }

    return b;
  }

  return nullptr;
}

void Button::paint(QPainter *painter, const QRectF &repaintRegion) {
  Q_UNUSED(repaintRegion)

  if (!decoration())
    return;

  switch (type()) {
  case KDecoration3::DecorationButtonType::Menu: {
    const QRectF iconRect = geometry().marginsRemoved(m_padding);
    const auto c = decoration()->window();
    if (auto deco = qobject_cast<Decoration *>(decoration())) {
      const QPalette activePalette = KIconLoader::global()->customPalette();
      QPalette palette = c->palette();
      palette.setColor(QPalette::WindowText, deco->fontColor());
      KIconLoader::global()->setCustomPalette(palette);
      c->icon().paint(painter, iconRect.toRect());
      if (activePalette == QPalette())
        KIconLoader::global()->resetPalette();
      else
        KIconLoader::global()->setCustomPalette(palette);
    } else {
      c->icon().paint(painter, iconRect.toRect());
    }
    break;
  }
  case KDecoration3::DecorationButtonType::Spacer:
    break;
  default:
    painter->save();
    drawIconWithMask(painter);
    painter->restore();
    break;
  }
}

void Button::reconfigure() {
  auto d = qobject_cast<Decoration *>(decoration());
  if (!d)
    return;

  switch (type()) {
  case KDecoration3::DecorationButtonType::Spacer:
    setPreferredSize(QSizeF(d->buttonSize() * 0.5, d->buttonSize()));
    break;
  default:
    setPreferredSize(QSizeF(d->buttonSize(), d->buttonSize()));
    break;
  }

  m_animation->setDuration(d->animationDuration());
}

void Button::updateAnimationState(bool hovered) {
  auto d = qobject_cast<Decoration *>(decoration());
  if (!(d && d->internalSettings()->animationsEnabled()))
    return;

  m_animation->setDirection(hovered ? QAbstractAnimation::Forward
                                    : QAbstractAnimation::Backward);
  if (m_animation->state() != QAbstractAnimation::Running)
    m_animation->start();
}

} // namespace BlossomUI

#include "blossomuibutton.moc"

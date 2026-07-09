// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuibutton.h"
#include "blossomuidecoration.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <QVariantAnimation>

namespace BlossomUI {

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;
using KDecoration3::DecorationButtonType;

QColor Button::foregroundColor() const {
  auto d = qobject_cast<Decoration *>(decoration());
  if (!d) {
    return QColor();
  }

  const bool isClose = type() == DecorationButtonType::Close;
  if (isPressed()) {
    return isClose ? d->fontColor() : d->titleBarColor();
  } else if (isClose && d->internalSettings()->outlineCloseButton()) {
    if (m_animation->state() == QAbstractAnimation::Running)
      return KColorUtils::mix(d->titleBarColor(), d->fontColor(), m_opacity);
    return isHovered() ? d->fontColor() : d->titleBarColor();
  } else if ((type() == DecorationButtonType::KeepBelow ||
              type() == DecorationButtonType::KeepAbove ||
              type() == DecorationButtonType::Shade) &&
             isChecked()) {
    return d->titleBarColor();
  } else if (m_animation->state() == QAbstractAnimation::Running) {
    if (isClose)
      return d->fontColor();
    return KColorUtils::mix(d->fontColor(), d->titleBarColor(), m_opacity);
  } else if (isHovered()) {
    return isClose ? d->fontColor() : d->titleBarColor();
  } else {
    return d->fontColor();
  }
}

QColor Button::backgroundColor() const {
  auto d = qobject_cast<Decoration *>(decoration());
  if (!d) {
    return QColor();
  }

  auto c = d->window();
  if (isPressed()) {
    if (type() == DecorationButtonType::Close)
      return c->color(ColorGroup::Warning, ColorRole::Foreground);
    else
      return KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);

  } else if ((type() == DecorationButtonType::KeepBelow ||
              type() == DecorationButtonType::KeepAbove ||
              type() == DecorationButtonType::Shade) &&
             isChecked()) {
    return d->fontColor();

  } else if (m_animation->state() == QAbstractAnimation::Running) {
    if (type() == DecorationButtonType::Close) {
      if (d->internalSettings()->outlineCloseButton()) {
        return KColorUtils::mix(
            d->fontColor(),
            c->color(ColorGroup::Warning, ColorRole::Foreground).lighter(),
            m_opacity);
      } else {
        QColor color(
            c->color(ColorGroup::Warning, ColorRole::Foreground).lighter());
        color.setAlpha(color.alpha() * m_opacity);
        return color;
      }
    } else {
      QColor color(d->fontColor());
      color.setAlpha(color.alpha() * m_opacity);
      return color;
    }

  } else if (isHovered()) {
    if (type() == DecorationButtonType::Close)
      return c->color(ColorGroup::Warning, ColorRole::Foreground).lighter();
    else
      return d->fontColor();

  } else if (type() == DecorationButtonType::Close &&
             d->internalSettings()->outlineCloseButton()) {
    return d->fontColor();

  } else {
    return QColor();
  }
}

} // namespace BlossomUI

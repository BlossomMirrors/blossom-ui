// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {

QPalette Helper::disabledPalette(const QPalette &source, qreal ratio) const {
  QPalette copy(source);

  const QList<QPalette::ColorRole> roles = {
      QPalette::Window,     QPalette::Highlight, QPalette::WindowText,
      QPalette::ButtonText, QPalette::Text,      QPalette::Button};
  foreach (const QPalette::ColorRole &role, roles) {
    copy.setColor(role, KColorUtils::mix(source.color(QPalette::Active, role),
                                         source.color(QPalette::Disabled, role),
                                         1.0 - ratio));
  }

  return copy;
}

QColor Helper::alphaColor(QColor color, qreal alpha) const {
  if (alpha >= 0 && alpha < 1.0) {
    color.setAlphaF(alpha * color.alphaF());
  }
  return color;
}

} // namespace BlossomUI

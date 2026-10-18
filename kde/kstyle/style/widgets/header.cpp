// SPDX-License-Identifier: GPL-2.0-or-later
#include "header.h"

#include "blossomuihelper.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

Fill headerSectionFill(const Helper *helper, const QPalette &palette, bool sunken,
                       bool mouseOver, bool animated, qreal opacity) {
  const QColor normal = palette.color(QPalette::Button);
  const QColor hover = helper->hoverSurface(palette);

  if (sunken)
      return Fill(hover);
  if (animated)
    return Fill(KColorUtils::mix(normal, hover, opacity));
  if (mouseOver)
    return Fill(hover);
  return Fill(normal);
}

} // namespace Render
} // namespace BlossomUI

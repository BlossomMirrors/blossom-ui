// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"
#include "blossomuistyleconfigdata.h"

#include <QPainter>

namespace BlossomUI {

void Helper::renderBoxShadow(QPainter *painter, const QRect &rect,
                             const int xOffset, const int yOffset,
                             const int size, const QColor &color,
                             const int cornerRadius, const bool active,
                             TileSet::Tiles tiles) const {
  if (!StyleConfigData::widgetDrawShadow())
    return;
  Q_UNUSED(active)
  // if (!active) {renderOutline(painter, rect, cornerRadius, 30);return;}
  CustomShadowParams params =
      CustomShadowParams(QPoint(xOffset, yOffset), size, color);
  TileSet shadow = ShadowHelper::shadowTiles(cornerRadius, params);
  shadow.render(rect.adjusted(-params.radius, -params.radius,
                              params.radius + params.offset.x(),
                              params.radius + params.offset.y()),
                painter, tiles);
  // qDebug() << "shadow on: " << rect.adjusted(-params.radius, -params.radius,
  // params.radius, params.radius);
}

} // namespace BlossomUI

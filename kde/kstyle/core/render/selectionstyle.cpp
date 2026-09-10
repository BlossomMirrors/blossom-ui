#include "selectionstyle.h"

#include "accentresolver.h"

#include <KColorUtils>

#include <QPainter>

namespace BlossomUI {
namespace Render {

QColor SelectionStyle::foreground(const QPalette &palette,
                                  QPalette::ColorGroup group,
                                  QPalette::ColorRole surface,
                                  bool mouseOver) {
  const QColor accent = palette.color(group, QPalette::Highlight);
  const QColor tinted = KColorUtils::mix(palette.color(group, surface), accent,
                                         tintAlpha(mouseOver));
  return AccentResolver::onSurface(accent, tinted, MinContrast);
}

QColor SelectionStyle::tint(const QPalette &palette, QPalette::ColorGroup group,
                            bool mouseOver) {
  QColor color = palette.color(group, QPalette::Highlight);
  color.setAlphaF(tintAlpha(mouseOver));
  return color;
}

QFont SelectionStyle::font(const QFont &base, bool selected) {
  if (!selected)
    return base;
  QFont result(base);
  result.setBold(true);
  return result;
}

QPixmap SelectionStyle::icon(const QIcon &source, const QSize &size, qreal dpr,
                             const QColor &color, bool selected,
                             QIcon::Mode mode, QIcon::State state) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QPixmap pixmap = source.pixmap(size, dpr, mode, state);
#else
  Q_UNUSED(dpr)
  const QPixmap pixmap = source.pixmap(size, mode, state);
#endif
  if (!selected || pixmap.isNull())
    return pixmap;

  QPixmap result(pixmap.size());
  result.setDevicePixelRatio(pixmap.devicePixelRatio());
  result.fill(Qt::transparent);

  const qreal ratio = pixmap.devicePixelRatio();
  const qreal logicalWidth = pixmap.width() / ratio;
  const qreal perSide = 0.5 * IconExtraStroke * logicalWidth / IconCanvas;
  const qreal step = 1.0 / ratio;

  QPainter painter(&result);
  painter.setOpacity(qBound(0.0, perSide / step, 1.0));
  const QPointF offsets[] = {{-step, 0.0}, {step, 0.0}, {0.0, -step}, {0.0, step}};
  for (const QPointF &at : offsets)
    painter.drawPixmap(at, pixmap);
  painter.setOpacity(1.0);
  painter.drawPixmap(QPointF(0.0, 0.0), pixmap);

  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(result.rect(), color);
  return result;
}

}
}

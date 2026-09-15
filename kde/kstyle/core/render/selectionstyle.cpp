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

  QPixmap result = pixmap.copy();

  QPainter painter(&result);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(result.rect(), color);
  return result;
}

}
}

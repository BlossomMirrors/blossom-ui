#include "selectionstyle.h"

#include "accentresolver.h"

#include <KColorUtils>
#include <KIconColors>
#include <KIconLoader>


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
  result.setWeight(QFont::DemiBold);
  return result;
}

QPixmap SelectionStyle::icon(const QIcon &source, const QSize &size, qreal dpr,
                             const QPalette &palette, const QColor &color,
                             bool selected, QIcon::Mode mode,
                             QIcon::State state) {
  QIcon icon = source;
  if (selected && !source.name().isEmpty()) {
    KIconColors colors(palette);
    colors.setText(color);
    icon = KDE::icon(source.name(), colors);
  }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return icon.pixmap(size, dpr, mode, state);
#else
  Q_UNUSED(dpr)
  return icon.pixmap(size, mode, state);
#endif
}

}
}

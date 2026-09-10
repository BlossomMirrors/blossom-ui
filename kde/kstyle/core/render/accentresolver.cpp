#include "accentresolver.h"

#include <KColorUtils>

namespace BlossomUI {
namespace Render {

QColor AccentResolver::onSurface(const QColor &accent, const QColor &surface,
                                 qreal minContrast) {
  const bool lightSurface = KColorUtils::luma(surface) > 0.5;

  QColor result = accent;
  QColor last;
  for (int step = 1; step <= 24; ++step) {
    if (KColorUtils::contrastRatio(result, surface) >= minContrast)
      break;
    const qreal amount = 0.06 * step;
    const QColor next = lightSurface
                            ? KColorUtils::darken(accent, amount, 1.0)
                            : KColorUtils::lighten(accent, amount, 1.0);
    if (next == last)
      break;
    last = next;
    result = next;
  }
  return result;
}

}
}

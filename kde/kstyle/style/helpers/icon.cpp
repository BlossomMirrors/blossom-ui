// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"

#include <KIconLoader>

namespace BlossomUI {

QPixmap Helper::coloredIcon(const QIcon &icon, const QPalette &palette,
                            const QSize &size, qreal devicePixelRatio,
                            QIcon::Mode mode, QIcon::State state) {
  const QPalette activePalette = KIconLoader::global()->customPalette();
  const bool changePalette = activePalette != palette;
  if (changePalette) {
    KIconLoader::global()->setCustomPalette(palette);
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QPixmap pixmap = icon.pixmap(size, devicePixelRatio, mode, state);
#else
  Q_UNUSED(devicePixelRatio);
  const QPixmap pixmap = icon.pixmap(size, mode, state);
#endif
  if (changePalette) {
    if (activePalette == QPalette()) {
      KIconLoader::global()->resetPalette();
    } else {
      KIconLoader::global()->setCustomPalette(activePalette);
    }
  }
  return pixmap;
}

} // namespace BlossomUI

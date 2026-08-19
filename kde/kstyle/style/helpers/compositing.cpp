// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"

#if __has_include("config-blossomui.h")
#include "config-blossomui.h"
#else
#define BLOSSOMUI_HAVE_X11 0
#endif

#include <KWindowSystem>

#if __has_include(<KX11Extras>)
#include <KX11Extras>
#endif

namespace BlossomUI {

bool Helper::isX11() {
#if BLOSSOMUI_HAVE_X11
  static const bool s_isX11 = KWindowSystem::isPlatformX11();
  return s_isX11;
#endif

  return false;
}

bool Helper::isWayland() {
  static const bool s_isWayland = KWindowSystem::isPlatformWayland();
  return s_isWayland;
}

bool Helper::compositingActive() const {
#if BLOSSOMUI_HAVE_X11
  if (isX11()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return KWindowSystem::compositingActive();
#elif __has_include(<KX11Extras>)
    return KX11Extras::compositingActive();
#endif
  }
#endif

  return true;
}

bool Helper::hasAlphaChannel(const QWidget *widget) const {
  return compositingActive() && widget &&
         widget->testAttribute(Qt::WA_TranslucentBackground);
}

bool Helper::shouldWindowHaveAlpha(const QPalette &palette,
                                   bool isDolphin) const {
  if (_activeTitleBarColor.alphaF() < 1.0 ||
      (StyleConfigData::dolphinSidebarOpacity() < 100 && isDolphin) ||
      palette.color(QPalette::Window).alpha() < 255) {
    return true;
  }
  return false;
}

} // namespace BlossomUI

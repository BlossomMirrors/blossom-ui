#!/bin/bash

PROJECT="blossomui"
_PROJECT="BlossomUI"

if [ "$(whoami)" != "root" ]; then
    echo "This script requires sudo to remove system files. Re-running with sudo..."
    exec sudo "$0" "$@"
fi

# Qt5
rm -f /usr/lib/lib${PROJECT}common5.so*
rm -f /usr/lib64/lib${PROJECT}common5.so*
rm -f /usr/lib/qt5/plugins/styles/${PROJECT}5.so*
rm -f /usr/lib64/qt5/plugins/styles/${PROJECT}5.so*
rm -f /usr/lib64/qt/plugins/styles/${PROJECT}5.so*
rm -f /usr/lib/x86_64-linux-gnu/qt5/plugins/styles/${PROJECT}5.so*

# Qt6
rm -f /usr/lib/lib${PROJECT}common6.so*
rm -f /usr/lib64/lib${PROJECT}common6.so*
rm -f /usr/lib/qt6/plugins/styles/${PROJECT}6.so*
rm -f /usr/lib64/qt6/plugins/styles/${PROJECT}6.so*
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/styles/${PROJECT}6.so*
rm -f /usr/share/kstyle/themes/${PROJECT}.themerc
rm -f /usr/lib/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*
rm -f /usr/lib64/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*
rm -f /usr/share/applications/${PROJECT}styleconfig.desktop
rm -f /usr/bin/${PROJECT}-settings6
rm -f /usr/share/icons/hicolor/scalable/apps/${PROJECT}-settings.svgz

# KDecoration
rm -f /usr/lib/qt6/plugins/org.kde.kdecoration3/org.blossomos.ui.so*
rm -f /usr/lib64/qt6/plugins/org.kde.kdecoration3/org.blossomos.ui.so*
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.blossomos.ui.so*
rm -f /usr/share/kservices6/${PROJECT}decorationconfig.desktop
rm -f /usr/lib/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*
rm -f /usr/lib64/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*
rm -f /usr/share/applications/kcm_${PROJECT}decoration.desktop

# CMake
rm -rf /usr/lib/cmake/${_PROJECT}
rm -rf /usr/lib64/cmake/${_PROJECT}

# Color schemes
rm -f /usr/share/color-schemes/${_PROJECT}Dark.colors
rm -f /usr/share/color-schemes/${_PROJECT}Light.colors
rm -f /usr/share/color-schemes/${_PROJECT}DarkOLED.colors

# Plasma Look and Feel
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.ui.light.desktop
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.ui.dark.desktop
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.ui.darkoled.desktop

# Plasma Desktop Theme
rm -rf /usr/share/plasma/desktopthemes/${_PROJECT}

# Wallpapers
rm -rf /usr/share/wallpapers/Blossom\ Rays

# Icons
rm -rf "/usr/share/icons/${_PROJECT}"

# GTK themes
rm -rf /usr/share/themes/${_PROJECT}

# Zed theme
rm -rf /usr/share/blossomui/zed

# User config copied there by install.sh
REAL_HOME=$(getent passwd "${SUDO_USER:-$USER}" | cut -d: -f6)
if [ -n "$REAL_HOME" ] && [ -d "$REAL_HOME/.config/gtk-4.0" ]; then
    rm -f "$REAL_HOME/.config/gtk-4.0/gtk.css"
    rm -rf "$REAL_HOME/.config/gtk-4.0/blossomui-gtk-assets"
fi
if [ -n "$REAL_HOME" ]; then
    rm -f "$REAL_HOME/.config/zed/themes/blossomui.json"
fi

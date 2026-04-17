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
rm -f /usr/lib/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*
rm -f /usr/lib64/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*
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
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.${PROJECT}light.desktop
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.${PROJECT}dark.desktop
rm -rf /usr/share/plasma/look-and-feel/org.blossomos.${PROJECT}darkoled.desktop

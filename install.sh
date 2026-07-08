#!/bin/env bash
set -e

SRC_DIR=$(pwd)

if [ "$1" = "remove" ]; then
    echo "Use ./uninstall.sh to remove BlossomUI"
    exit 1
fi

echo " *** Unlcoking rpm-ostree so the changes persist after restart *** "
sudo rpm-ostree unlock --hotfix | true || true

sudo dnf install -y kdecoration-devel sassc gettext # FIXME: should be included in the DX OS image
echo " *** Building *** "
"$SRC_DIR/release.sh"

RPM=$(ls -1t "$SRC_DIR"/release/blossomui-*.rpm 2>/dev/null | head -1)
if [ -z "$RPM" ]; then
    echo "no RPM produced by release.sh, see build output above for the error"
    exit 1
fi

echo " *** Clearing cache *** "
rm -rf ~/.cache/ksplash/qmlcache

echo " *** Installing $(basename "$RPM") *** "
if rpm -q blossomui >/dev/null 2>&1 && \
   [ "$(rpm -q --qf '%{VERSION}-%{RELEASE}' blossomui)" = "$(rpm -qp --qf '%{VERSION}-%{RELEASE}' "$RPM")" ]; then
    sudo dnf reinstall -y "$RPM"
else
    sudo dnf install -y "$RPM"
fi

echo " *** Refreshing font cache *** "
sudo fc-cache -f >/dev/null
fc-cache -f >/dev/null 2>&1 || true

QT5_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*-qt5.flatpak 2>/dev/null | head -1)
GTK3THEME_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*-gtk3theme.flatpak 2>/dev/null | head -1)
ICONTHEME_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*-icontheme.flatpak 2>/dev/null | head -1)
QT6_610_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*-qt6-6.10.flatpak 2>/dev/null | head -1)
QT6_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*.flatpak 2>/dev/null | grep -v -- '-qt5\.flatpak' | grep -v -- '-gtk3theme\.flatpak' | grep -v -- '-icontheme\.flatpak' | grep -v -- '-qt6-6\.10\.flatpak' | head -1)

if [ -n "$QT5_FLATPAK" ]; then
    echo " *** Installing $(basename "$QT5_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$QT5_FLATPAK"
fi
if [ -n "$QT6_FLATPAK" ]; then
    echo " *** Installing $(basename "$QT6_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$QT6_FLATPAK"
fi
if [ -n "$QT6_610_FLATPAK" ]; then
    echo " *** Installing $(basename "$QT6_610_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$QT6_610_FLATPAK"
fi
if [ -n "$GTK3THEME_FLATPAK" ]; then
    echo " *** Installing $(basename "$GTK3THEME_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$GTK3THEME_FLATPAK"
fi
if [ -n "$ICONTHEME_FLATPAK" ]; then
    echo " *** Installing $(basename "$ICONTHEME_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$ICONTHEME_FLATPAK"
fi

echo " *** Done *** "

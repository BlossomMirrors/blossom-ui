#!/bin/env bash
set -e

SRC_DIR=$(pwd)

if [ "$1" = "remove" ]; then
    echo "Use ./uninstall.sh to remove BlossomUI"
    exit 1
fi

echo " *** Activating temporary /usr overlay (changes gone after reboot) *** "
sudo rpm-ostree usroverlay || true

sudo dnf install -y kdecoration-devel # FIXME: should be included in the DX OS image
echo " *** Building *** "
"$SRC_DIR/release.sh" || true

RPM=$(ls -1t "$SRC_DIR"/release/blossomui-*.rpm 2>/dev/null | head -1)
if [ -z "$RPM" ]; then
    echo "no RPM produced, is rpmbuild installed?"
    exit 1
fi

echo " *** Installing $(basename "$RPM") *** "
if rpm -q blossomui >/dev/null 2>&1 && \
   [ "$(rpm -q --qf '%{VERSION}-%{RELEASE}' blossomui)" = "$(rpm -qp --qf '%{VERSION}-%{RELEASE}' "$RPM")" ]; then
    sudo dnf reinstall -y "$RPM"
else
    sudo dnf install -y "$RPM"
fi

QT5_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*-qt5.flatpak 2>/dev/null | head -1)
QT6_FLATPAK=$(ls -1t "$SRC_DIR"/release/blossomui-*.flatpak 2>/dev/null | grep -v -- '-qt5\.flatpak' | head -1)

if [ -n "$QT5_FLATPAK" ]; then
    echo " *** Installing $(basename "$QT5_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$QT5_FLATPAK"
fi
if [ -n "$QT6_FLATPAK" ]; then
    echo " *** Installing $(basename "$QT6_FLATPAK") *** "
    flatpak install --user --or-update --noninteractive --bundle "$QT6_FLATPAK"
fi

echo " *** Done *** "

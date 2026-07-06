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
fc-cache -f
rm -rf ~/.cache/ksplash/qmlcache

echo " *** Installing $(basename "$RPM") *** "
if rpm -q blossomui >/dev/null 2>&1 && \
   [ "$(rpm -q --qf '%{VERSION}-%{RELEASE}' blossomui)" = "$(rpm -qp --qf '%{VERSION}-%{RELEASE}' "$RPM")" ]; then
    sudo dnf reinstall -y "$RPM"
else
    sudo dnf install -y "$RPM"
fi

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

# GTK4/libadwaita apps have no flatpak theme extension point, but they all load
# ~/.config/gtk-4.0/gtk.css, which sandboxes can see via the gtk-4.0 override the
# RPM sets up. The css must be copied (not symlinked) since /usr/share/themes
# isn't visible inside sandboxes.
GTK4_THEME_DIR=/usr/share/themes/BlossomUI/gtk-4.0
if [ -f "$GTK4_THEME_DIR/gtk.css" ]; then
    echo " *** Installing GTK4 theme into ~/.config/gtk-4.0 *** "
    mkdir -p ~/.config/gtk-4.0
    cp -f "$GTK4_THEME_DIR/gtk.css" ~/.config/gtk-4.0/gtk.css
    # keep kde-gtk-config's live color sync overriding our static fallbacks
    # (later @define-color wins; kde-gtk-config itself only appends this line)
    if [ -f ~/.config/gtk-4.0/colors.css ]; then
        printf "\n@import 'colors.css';\n" >> ~/.config/gtk-4.0/gtk.css
    fi
    rm -rf ~/.config/gtk-4.0/blossomui-gtk-assets
    cp -rL "$GTK4_THEME_DIR/blossomui-gtk-assets" ~/.config/gtk-4.0/blossomui-gtk-assets
fi

echo " *** Done *** "

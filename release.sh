#!/bin/bash
set -e

NAME=blossomui
CURRENT_VERSION=$(cat VERSION)

VERSION=${1:-$CURRENT_VERSION}
RELEASE=${2:-1}
CHANGELOG=${3:-"packaged $NAME $VERSION"}

if [ "$VERSION" != "$CURRENT_VERSION" ]; then
    echo "$VERSION" > VERSION
    echo "Updated VERSION to $VERSION"
fi

SRC_DIR=$(pwd)
RPMBUILD=~/rpmbuild
mkdir -p "$RPMBUILD"/{SPECS,SOURCES,BUILD,RPMS,SRPMS} release

# generate RPM spec file with complete file listings for all theme components
cat > "$RPMBUILD/SPECS/$NAME.spec" << EOF
Name:           $NAME
Version:        $VERSION
Release:        $RELEASE%{?dist}
Summary:        A modern global theme for Qt/KDE (Application Style, Window Decoration, Icons, Wallpapers, Plasma Theme)
License:        GPL-2.0-or-later
URL:            https://dev.blossomos.org/Blossom/ui/desktop-theme
%define debug_package %{nil}

# Build requirements for QT6
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  sassc
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  kf6-kcolorscheme-devel
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-kguiaddons-devel
BuildRequires:  kf6-ki18n-devel
BuildRequires:  kf6-kiconthemes-devel
BuildRequires:  kf6-kwindowsystem-devel
BuildRequires:  kf6-kcmutils-devel
BuildRequires:  kf6-frameworkintegration-devel
BuildRequires:  kf6-kirigami-devel
BuildRequires:  kwin-devel
BuildRequires:  libepoxy-devel
BuildRequires:  cmake(KDecoration3)

# Build requirements for QT5
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  kf5-kcoreaddons-devel
BuildRequires:  kf5-kconfig-devel
BuildRequires:  kf5-kguiaddons-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  kf5-kiconthemes-devel
BuildRequires:  kf5-kwindowsystem-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  kf5-frameworkintegration-devel
BuildRequires:  kf5-kirigami2-devel
BuildRequires:  kf5-kpackage-devel

# Runtime requirements
Requires:       qt6-qtbase
Requires:       qt5-qtbase
Requires:       kf6-kcoreaddons
Requires:       kf5-kcoreaddons
Requires:       kf6-kconfig
Requires:       bibata-cursor-themes

%description
BlossomUI is a modern global theme for Qt and KDE applications,
forked from Darkly (which is itself a fork of Lightly/KDE Breeze).

This package includes both Qt5 and Qt6 application styles:
- Application Style (KStyle) for Qt5 and Qt6
- Window Decoration for KWin (Qt6)
- Global Look-and-Feel themes
- Plasma Desktop Theme
- Icon Theme
- Wallpapers
- Color Schemes

%pretrans
rm -rf /usr/share/icons/BlossomUI

%prep
# nothing to unpack: binaries come from the incremental CMake tree in build/rpm

%build

%install
cp -a ${SRC_DIR}/build/rpm/staging/. %{buildroot}/

%post
if command -v kwriteconfig6 >/dev/null 2>&1; then
    KWRITE=kwriteconfig6
elif command -v kwriteconfig5 >/dev/null 2>&1; then
    KWRITE=kwriteconfig5
else
    exit 0
fi
\$KWRITE --file /etc/xdg/kdeglobals --group KDE --key LookAndFeelPackage 'org.blossomos.ui.light.desktop'
\$KWRITE --file /etc/xdg/kdeglobals --group KDE --key DefaultLightLookAndFeel 'org.blossomos.ui.light.desktop'
\$KWRITE --file /etc/xdg/kdeglobals --group KDE --key DefaultDarkLookAndFeel 'org.blossomos.ui.dark.desktop'
\$KWRITE --file /etc/xdg/kdeglobals --group General --key ColorScheme 'BlossomUI Light'
\$KWRITE --file /etc/xdg/kdeglobals --group General --key LightColorScheme 'BlossomUI Light'
\$KWRITE --file /etc/xdg/kdeglobals --group General --key DarkColorScheme 'BlossomUI Dark'
\$KWRITE --file /etc/xdg/plasmarc --group Theme --key name 'BlossomUI'
\$KWRITE --file /etc/xdg/plasmarc --group Theme --key LightColorScheme 'BlossomUI Light'
\$KWRITE --file /etc/xdg/plasmarc --group Theme --key DarkColorScheme 'BlossomUI Dark'
\$KWRITE --file /etc/xdg/plasmarc --group Theme --key LightLookAndFeel 'org.blossomos.ui.light.desktop'
\$KWRITE --file /etc/xdg/plasmarc --group Theme --key DarkLookAndFeel 'org.blossomos.ui.dark.desktop'

if command -v flatpak >/dev/null 2>&1; then
    flatpak override --system --filesystem=xdg-config/gtk-3.0:ro 2>/dev/null || true
    flatpak override --system --filesystem=xdg-config/gtk-4.0:ro 2>/dev/null || true
fi

if [ -d /var/lib/plasmalogin ]; then
    if [ -d '%{_datadir}/wallpapers/Blossom Rays' ]; then
        mkdir -p /var/lib/plasmalogin/wallpapers
        rm -rf '/var/lib/plasmalogin/wallpapers/Blossom Rays'
        cp -r '%{_datadir}/wallpapers/Blossom Rays' /var/lib/plasmalogin/wallpapers/
    fi
    mkdir -p /var/lib/plasmalogin/.config
    for f in kdeglobals plasmarc; do
        if [ ! -e "/var/lib/plasmalogin/.config/\$f" ]; then
            cp "%{_datadir}/blossomui/plasmalogin/greeter-\$f" "/var/lib/plasmalogin/.config/\$f"
        fi
    done
    chown -R plasmalogin:plasmalogin /var/lib/plasmalogin/wallpapers /var/lib/plasmalogin/.config 2>/dev/null || true
fi

%postun
if [ \$1 -eq 0 ]; then
    if command -v flatpak >/dev/null 2>&1; then
        flatpak override --system --nofilesystem=xdg-config/gtk-3.0 2>/dev/null || true
        flatpak override --system --nofilesystem=xdg-config/gtk-4.0 2>/dev/null || true
    fi

    rm -rf '/var/lib/plasmalogin/wallpapers/Blossom Rays'

    if command -v kwriteconfig6 >/dev/null 2>&1; then
        KWRITE=kwriteconfig6
    elif command -v kwriteconfig5 >/dev/null 2>&1; then
        KWRITE=kwriteconfig5
    else
        exit 0
    fi
    \$KWRITE --file /etc/xdg/kdeglobals --group KDE --key LookAndFeelPackage --delete
    \$KWRITE --file /etc/xdg/kdeglobals --group KDE --key DefaultLightLookAndFeel --delete
    \$KWRITE --file /etc/xdg/kdeglobals --group KDE --key DefaultDarkLookAndFeel --delete
    \$KWRITE --file /etc/xdg/kdeglobals --group General --key ColorScheme --delete
    \$KWRITE --file /etc/xdg/kdeglobals --group General --key LightColorScheme --delete
    \$KWRITE --file /etc/xdg/kdeglobals --group General --key DarkColorScheme --delete
    \$KWRITE --file /etc/xdg/plasmarc --group Theme --key name --delete
    \$KWRITE --file /etc/xdg/plasmarc --group Theme --key LightColorScheme --delete
    \$KWRITE --file /etc/xdg/plasmarc --group Theme --key DarkColorScheme --delete
    \$KWRITE --file /etc/xdg/plasmarc --group Theme --key LightLookAndFeel --delete
    \$KWRITE --file /etc/xdg/plasmarc --group Theme --key DarkLookAndFeel --delete
fi

%files
# Qt6 Application Style
%{_libdir}/qt6/plugins/styles/blossomui6.so
%{_libdir}/qt6/plugins/kstyle_config/blossomuistyleconfig.so

# Qt6 Window Decoration
%{_libdir}/qt6/plugins/org.kde.kdecoration3/org.blossomos.ui.so
%{_libdir}/qt6/plugins/org.kde.kdecoration3.kcm/kcm_blossomuidecoration.so

# Qt5 Application Style
%{_libdir}/qt5/plugins/styles/blossomui5.so

# KStyle Theme Files
%{_datadir}/kstyle/themes/blossomui.themerc

# Color Schemes
%{_datadir}/color-schemes/BlossomUIDark.colors
%{_datadir}/color-schemes/BlossomUILight.colors
%{_datadir}/color-schemes/BlossomUIDarkOLED.colors
%{_datadir}/konsole/BlossomUI.colorscheme

# Desktop Entries
%{_datadir}/applications/blossomuistyleconfig.desktop
%{_datadir}/applications/kcm_blossomuidecoration.desktop

# Settings Binary
%{_bindir}/blossomui-settings6

# Icons
%{_datadir}/icons/hicolor/scalable/apps/blossomui-settings.svgz
%{_datadir}/icons/BlossomUI/

# Plasma Look-and-Feel (Global Themes)
%{_datadir}/plasma/look-and-feel/org.blossomos.ui.light.desktop/
%{_datadir}/plasma/look-and-feel/org.blossomos.ui.dark.desktop/
%{_datadir}/plasma/look-and-feel/org.blossomos.ui.darkoled.desktop/

# Translations
%{_datadir}/locale/*/LC_MESSAGES/blossomui_ksplash.mo

# Plasma Desktop Theme
%{_datadir}/plasma/desktoptheme/BlossomUI/

# Wallpapers
%{_datadir}/wallpapers/

# Fonts
%{_datadir}/fonts/blossomui/

# GTK Theme
%{_datadir}/themes/BlossomUI/

# Zed Theme
%{_datadir}/blossomui/zed/

# QML style module (QtQuick Controls + Kirigami platform plugin)
%{_libdir}/qt6/qml/org/blossomos/
%{_libdir}/qt6/plugins/kf6/kirigami/platform/org.blossomos.style.so
%{_sysconfdir}/xdg/plasma-workspace/env/blossom-qqc2-style.sh

# Kirigami control overrides. We own only our own style subdirectory, the same
# way plasma-union owns styles/org.kde.union/; styles/ itself stays kf6-kirigami's.
%{_libdir}/qt6/qml/org/kde/kirigami/styles/org.blossomos.style/

%{_sysconfdir}/plasmalogin.conf.d/10-blossomui.conf
%{_datadir}/blossomui/plasmalogin/

# CMake Config (Qt6)
%{_libdir}/cmake/BlossomUI/

# KServices (Qt6)
%{_datadir}/kservices6/blossomuidecoration*.desktop

%changelog
* $(LC_TIME=C date "+%a %b %d %Y") packager - $VERSION-$RELEASE
- $CHANGELOG
EOF

echo "=== Building (incremental CMake) ==="
cmake -S "$SRC_DIR" -B "$SRC_DIR/build/rpm" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib64 \
    -DBUILD_TESTING=OFF \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
    -DBUILD_QT5=ON \
    -DBUILD_QT6=ON \
    -DWITH_DECORATIONS=ON \
    -DINSTALL_QQC2_STYLE_ENV=ON \
    -DFOR_FLATPAK=OFF
cmake --build "$SRC_DIR/build/rpm" --parallel
rm -rf "$SRC_DIR/build/rpm/staging"
DESTDIR="$SRC_DIR/build/rpm/staging" cmake --install "$SRC_DIR/build/rpm" >/dev/null

echo "=== Packaging RPM ==="
if command -v rpmbuild >/dev/null 2>&1; then
    rpmbuild -bb "$RPMBUILD/SPECS/$NAME.spec"
    find "$RPMBUILD/RPMS" -name "$NAME-$VERSION-$RELEASE*.rpm" -exec cp {} release/ \;
    echo "RPM package created in release/"
    echo "Note: RPM includes both Qt5 and Qt6 application styles with all theme components"
else
    echo "rpmbuild not found, skipping RPM release."
fi

# Flatpak
echo ""
echo "=== Building Flatpak ==="

rm -rf "$SRC_DIR/.flatpak-builder/cache" "$SRC_DIR/.flatpak-builder/build" "$SRC_DIR/.flatpak-builder/rofiles"

if command -v flatpak-builder >/dev/null 2>&1; then
    builder="flatpak-builder"
elif flatpak run org.flatpak.Builder --version >/dev/null 2>&1; then
    builder="flatpak run org.flatpak.Builder"
else
    echo "flatpak-builder not found, skipping Flatpak release."
    echo "Done: $(ls release/ 2>/dev/null || echo 'no files')"
    exit 0
fi

ARCH=$(uname -m)
REPO="$SRC_DIR/local"
mkdir -p "$REPO"

echo "Ensuring Flatpak SDKs and runtimes are installed..."
flatpak install --noninteractive --system flathub \
    org.kde.Sdk/x86_64/6.9 \
    org.kde.Sdk/x86_64/6.10 \
    org.kde.Sdk/x86_64/6.11 \
    org.kde.Sdk/x86_64/5.15-24.08 \
    org.kde.Platform/x86_64/6.9 \
    org.kde.Platform/x86_64/6.10 \
    org.kde.Platform/x86_64/6.11 \
    org.kde.Platform/x86_64/5.15-24.08 \
    org.freedesktop.Sdk/x86_64/24.08 \
    org.freedesktop.Platform/x86_64/24.08 \
    2>&1 | grep -v "^$" || true

# pre-compile the GTK3 theme CSS so the org.gtk.Gtk3theme.BlossomUI flatpak
# extension can just install it, without needing sassc inside the sandbox
mkdir -p "$SRC_DIR/gtk/build"
sassc -M -t compact -I "$SRC_DIR/build/rpm/gtk" "$SRC_DIR/gtk/sass/gtk3-light.scss" "$SRC_DIR/gtk/build/gtk3-light.css"
sassc -M -t compact -I "$SRC_DIR/build/rpm/gtk" "$SRC_DIR/gtk/sass/gtk3-dark.scss" "$SRC_DIR/gtk/build/gtk3-dark.css"

# create flatpak source archive (clean, no transform)
tar --warning=no-file-changed -czf "$SRC_DIR/flatpak/blossomui-flatpak-source.tar.gz" \
    --exclude=./.git \
    --exclude=./.flatpak-builder \
    --exclude=./release \
    --exclude=./build \
    --exclude=./build-test \
    --exclude=./redhat-linux-build \
    --exclude=./flatpak-build-qt5 \
    --exclude=./flatpak-build-qt6 \
    --exclude=./local \
    --exclude=./RPMS \
    --exclude=./SRPMS \
    --exclude=./flatpak/blossomui-flatpak-source.tar.gz \
    .


echo "Building Qt6 Flatpak extension with all theme components (KDE Platform 6.9)..."
$builder "$SRC_DIR/flatpak/build/qt6" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt6.json"

echo "Building Qt6 Flatpak extension with all theme components (KDE Platform 6.10)..."
$builder "$SRC_DIR/flatpak/build/qt6-6.10" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt6-6.10.json"

echo "Building Qt6 Flatpak extension with all theme components (KDE Platform 6.11)..."
$builder "$SRC_DIR/flatpak/build/qt6-6.11" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt6-6.11.json"

echo "Building Qt5 Flatpak extension with all theme components..."
$builder "$SRC_DIR/flatpak/build/qt5" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt5.json"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt5.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/5.15-24.08" --runtime
echo "Qt5 Flatpak bundle created"

# Create Qt6 bundles
flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.9" --runtime
echo "Qt6 Flatpak bundle (6.9) created with all theme components"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt6-6.10.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.10" --runtime
echo "Qt6 Flatpak bundle (6.10) created with all theme components"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt6-6.11.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.11" --runtime
echo "Qt6 Flatpak bundle (6.11) created with all theme components"

echo "Building GTK3 theme Flatpak extension..."
$builder "$SRC_DIR/flatpak/build/gtk3theme" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.gtk.Gtk3theme.BlossomUI.json"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-gtk3theme.flatpak" "runtime/org.gtk.Gtk3theme.BlossomUI/$ARCH/3.22" --runtime
echo "GTK3 theme Flatpak bundle created"

echo "Building icon theme Flatpak extension..."
$builder "$SRC_DIR/flatpak/build/icontheme" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.freedesktop.Platform.Icontheme.BlossomUI.json"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-icontheme.flatpak" "runtime/org.freedesktop.Platform.Icontheme.BlossomUI/$ARCH/1.0" --runtime
echo "Icon theme Flatpak bundle created"



echo ""
echo "=== Build Complete ==="
echo "Release files:"
ls -la release/
echo ""
echo "Flatpak manifests saved to: flatpak/"
ls flatpak/*.json 2>/dev/null || echo "  (none)"
echo ""
echo "Summary:"
echo "  - RPM includes: Qt5 style, Qt6 style, decorations, and all theme components"
echo "  - Flatpak (Qt6) includes: Qt6 style, decorations, and all theme components"
echo "  - Flatpak (Qt5) includes: Qt5 style and all theme components"

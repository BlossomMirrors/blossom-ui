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

tar --warning=no-file-changed -czf "$RPMBUILD/SOURCES/$NAME-$VERSION.tar.gz" \
    --transform "s|^\./|$NAME-$VERSION/|" \
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
    .

# generate RPM spec file with complete file listings for all theme components
cat > "$RPMBUILD/SPECS/$NAME.spec" << EOF
Name:           $NAME
Version:        $VERSION
Release:        $RELEASE%{?dist}
Summary:        A modern global theme for Qt/KDE (Application Style, Window Decoration, Icons, Wallpapers, Plasma Theme)
License:        GPL-2.0-or-later
URL:            https://git.blossomos.org/Blossom/ui
Source0:        %{name}-%{version}.tar.gz

# Build requirements for QT6
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  gettext
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

%prep
%autosetup

%build
# configure with both Qt5 and Qt6 support, including all theme components
%cmake -DBUILD_TESTING=OFF \
       -DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
       -DBUILD_QT5=ON \
       -DBUILD_QT6=ON \
       -DWITH_DECORATIONS=ON \
       -DFOR_FLATPAK=OFF
%cmake_build

%install
%cmake_install

%files
# Qt6 Application Style
%{_libdir}/qt6/plugins/styles/blossomui6.so
%{_libdir}/qt6/plugins/kstyle_config/blossomuistyleconfig.so
# Qt6 QtQuick Controls style (org.blossomos.style) — activated by
# plasma-patches-blossom's plasma-integration patch when the BlossomUI
# widget style is selected in the Application Style KCM
%{_libdir}/qt6/qml/org/blossomos/
%{_libdir}/qt6/plugins/kf6/kirigami/platform/org.blossomos.style.so
# Qt6 Window Decoration
%{_libdir}/qt6/plugins/org.kde.kdecoration3/org.blossomos.ui.so
%{_libdir}/qt6/plugins/org.kde.kdecoration3.kcm/kcm_blossomuidecoration.so
# Qt6 Common Library

# Qt5 Application Style
%{_libdir}/qt5/plugins/styles/blossomui5.so
# Qt5 Common Library

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
# Plasma Look-and-Feel (KSplash theme)
%{_datadir}/plasma/look-and-feel/org.blossomos.ui.splash.desktop/
# Translations
%{_datadir}/locale/*/LC_MESSAGES/blossomui_ksplash.mo
# Plasma Desktop Theme
%{_datadir}/plasma/desktoptheme/BlossomUI/
# Wallpapers
%{_datadir}/wallpapers/
# Fonts
%{_datadir}/fonts/blossomui/

# CMake Config (Qt6)
%{_libdir}/cmake/BlossomUI/
# KServices (Qt6)
%{_datadir}/kservices6/blossomuidecoration*.desktop

%changelog
* $(LC_TIME=C date "+%a %b %d %Y") packager - $VERSION-$RELEASE
- $CHANGELOG
EOF

echo "=== Building RPM ==="
if command -v rpmbuild >/dev/null 2>&1; then
    rpmbuild -ba "$RPMBUILD/SPECS/$NAME.spec"
    find "$RPMBUILD/RPMS" -name "$NAME-$VERSION-$RELEASE*.rpm" -exec cp {} release/ \;
    echo "RPM package created in release/"
    echo "Note: RPM includes both Qt5 and Qt6 application styles with all theme components"
else
    echo "rpmbuild not found, skipping RPM release."
fi

# Flatpak
echo ""
echo "=== Building Flatpak ==="
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
    org.kde.Sdk/x86_64/5.15-24.08 \
    org.kde.Platform/x86_64/6.9 \
    org.kde.Platform/x86_64/5.15-24.08 \
    2>&1 | grep -v "^$" || true

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


echo "Building Qt6 Flatpak extension with all theme components..."
$builder "$SRC_DIR/flatpak/build/qt6" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt6.json"

echo "Building Qt5 Flatpak extension with all theme components..."
$builder "$SRC_DIR/flatpak/build/qt5" --repo="$REPO" --force-clean --ccache "$SRC_DIR/flatpak/org.kde.KStyle.BlossomUI-qt5.json"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt5.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/5.15-24.08" --runtime
echo "Qt5 Flatpak bundle created"

# Create Qt6 bundle
flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.10" --runtime
echo "Qt6 Flatpak bundle created with all theme components"



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

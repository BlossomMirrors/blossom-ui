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

# Create source tarball excluding build artifacts and version control
tar -czf "$RPMBUILD/SOURCES/$NAME-$VERSION.tar.gz" \
    --transform "s|^\./|$NAME-$VERSION/|" \
    --exclude=./.git \
    --exclude=./.flatpak-builder \
    --exclude=./release \
    --exclude=./build \
    --exclude=./flatpak-build-qt5 \
    --exclude=./flatpak-build-qt6 \
    --exclude=./local \
    --exclude=./RPMS \
    --exclude=./SRPMS \
    .

# Generate RPM spec file with complete file listings for all theme components
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

%description
BlossomUI is a modern global theme for Qt and KDE applications,
forked from Darkly (which is itself a fork of Lightly/KDE Breeze).

This package includes:
- Application Style (KStyle) for QT5 and QT6
- Window Decoration for KWin (QT6)
- Global Look-and-Feel themes (Light, Dark, Dark OLED)
- Plasma Desktop Theme
- Icon Theme
- Wallpapers
- Color Schemes
- Dolphin Theme

%prep
%autosetup

%build
# Configure with both QT5 and QT6 support, including all theme components
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
# License files
%license LICENSE.md COPYING
# QT6 Application Style
%{_libdir}/qt6/plugins/styles/blossomui6.so
%{_libdir}/qt6/plugins/kstyle_config/blossomuistyleconfig.so
# QT6 Window Decoration
%{_libdir}/qt6/plugins/org.kde.kdecoration3/org.kde.blossomui.so
%{_libdir}/qt6/plugins/org.kde.kdecoration3.kcm/kcm_blossomuidecoration.so
# QT6 Common Library

# QT5 Application Style
%{_libdir}/qt5/plugins/styles/blossomui5.so
# QT5 Common Library

# KStyle Theme Files
%{_datadir}/kstyle/themes/blossomui.themerc
# Color Schemes
%{_datadir}/color-schemes/BlossomUIDark.colors
%{_datadir}/color-schemes/BlossomUILight.colors
%{_datadir}/color-schemes/BlossomUIDarkOLED.colors
# Desktop Entries
%{_datadir}/applications/blossomuistyleconfig.desktop
%{_datadir}/applications/kcm_blossomuidecoration.desktop
# Settings Binary
%{_bindir}/blossomui-settings6
# Icons
%{_datadir}/icons/hicolor/scalable/apps/blossomui-settings.svgz
%{_datadir}/icons/BlossomUI/
# Plasma Look-and-Feel (Global Themes)
%{_datadir}/plasma/look-and-feel/org.blossomos.blossomuilight.desktop/
%{_datadir}/plasma/look-and-feel/org.blossomos.blossomuidark.desktop/
%{_datadir}/plasma/look-and-feel/org.blossomos.blossomuidarkoled.desktop/
# Plasma Desktop Theme
%{_datadir}/plasma/desktoptheme/BlossomUI/
# Wallpapers
%{_datadir}/wallpapers/

# CMake Config (QT6)
%{_libdir}/cmake/BlossomUI/
# KServices (QT6)
%{_datadir}/kservices6/blossomuidecoration*.desktop

%changelog
* $(LC_TIME=C date "+%a %b %d %Y") packager - $VERSION-$RELEASE
- $CHANGELOG
EOF

echo "=== Building RPM ==="
if command -v rpmbuild >/dev/null 2>&1; then
    rpmbuild -ba "$RPMBUILD/SPECS/$NAME.spec"
    find "$RPMBUILD/RPMS" -name "$NAME-$VERSION-$RELEASE*.rpm" -exec cp {} release/ \;
    echo "RPM packages created in release/"
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

# Create updated Flatpak manifests that include ALL theme components
# (without FOR_FLATPAK flag to include wallpapers, icons, desktop theme, etc.)

cat > "$SRC_DIR/org.kde.KStyle.BlossomUI6.full.json" << FLATEOF
{
  "id": "org.kde.KStyle.BlossomUI",
  "branch": "6.9",
  "runtime": "org.kde.Platform",
  "build-extension": true,
  "sdk": "org.kde.Sdk",
  "runtime-version": "6.9",
  "appstream-compose": false,
  "separate-locales": false,
  "modules": [
    {
      "name": "BlossomUI",
      "buildsystem": "cmake",
      "builddir": true,
      "config-opts": [
        "-DCMAKE_INSTALL_PREFIX=/app",
        "-DKDE_INSTALL_PLUGINDIR=/app/lib/plugins",
        "-DKDE_INSTALL_KCONFMODULEDIR=/app/lib/plugins",
        "-DKDE_INSTALL_KCMMODULEDIR=/app/lib/qt6/plugins",
        "-DKDE_INSTALL_DATADIR=/app/share",
        "-DBUILD_QT6=ON",
        "-DBUILD_QT5=OFF",
        "-DWITH_DECORATIONS=OFF",
        "-DFOR_FLATPAK=OFF"
      ],
      "sources": [
        {
          "type": "git",
          "url": "https://git.blossomos.org/Blossom/ui",
          "branch": "main"
        }
      ]
    }
  ]
}
FLATEOF

cat > "$SRC_DIR/org.kde.KStyle.BlossomUI5.full.json" << FLATEOF
{
  "id": "org.kde.KStyle.BlossomUI",
  "branch": "5.15-24.08",
  "runtime": "org.kde.Platform",
  "build-extension": true,
  "sdk": "org.kde.Sdk",
  "runtime-version": "5.15-24.08",
  "appstream-compose": false,
  "separate-locales": false,
  "modules": [
    {
      "name": "BlossomUI",
      "buildsystem": "cmake",
      "builddir": true,
      "config-opts": [
        "-DCMAKE_INSTALL_PREFIX=/app",
        "-DKDE_INSTALL_PLUGINDIR=/app/lib/plugins",
        "-DKDE_INSTALL_KCONFMODULEDIR=/app/lib/plugins",
        "-DKDE_INSTALL_KCMMODULEDIR=/app/lib/qt5/plugins",
        "-DKDE_INSTALL_DATADIR=/app/share",
        "-DBUILD_QT6=OFF",
        "-DBUILD_QT5=ON",
        "-DWITH_DECORATIONS=OFF",
        "-DFOR_FLATPAK=OFF"
      ],
      "sources": [
        {
          "type": "git",
          "url": "https://git.blossomos.org/Blossom/ui",
          "branch": "main"
        }
      ]
    }
  ]
}
FLATEOF

# Build QT6 Flatpak (with decorations)
echo "Building QT6 Flatpak extension..."
$builder "$SRC_DIR/flatpak-build-qt6" --repo="$REPO" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI6.full.json"

# Build QT5 Flatpak (without decorations, KWin decorations not available for QT5)
echo "Building QT5 Flatpak extension..."
$builder "$SRC_DIR/flatpak-build-qt5" --repo="$REPO" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI5.full.json"

# Create Flatpak bundles
flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt6.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.9" --runtime
flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt5.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/5.15-24.08" --runtime

# Clean up temporary manifest files
rm -f "$SRC_DIR/org.kde.KStyle.BlossomUI6.full.json" "$SRC_DIR/org.kde.KStyle.BlossomUI5.full.json"

echo ""
echo "=== Build Complete ==="
echo "Release files:"
ls -la release/

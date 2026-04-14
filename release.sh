#!/bin/bash
set -e

NAME=blossomui
CURRENT_VERSION=$(cat VERSION)

read -rp "Version [$CURRENT_VERSION]: " VERSION
VERSION=${VERSION:-$CURRENT_VERSION}

read -rp "Release [1]: " RELEASE
RELEASE=${RELEASE:-1}

read -rp "Changelog entry [packaged $NAME $VERSION]: " CHANGELOG
CHANGELOG=${CHANGELOG:-"packaged $NAME $VERSION"}

if [ "$VERSION" != "$CURRENT_VERSION" ]; then
    echo "$VERSION" > VERSION
    echo "Updated VERSION to $VERSION"
fi

SRC_DIR=$(pwd)
RPMBUILD=~/rpmbuild
mkdir -p "$RPMBUILD"/{SPECS,SOURCES,BUILD,RPMS,SRPMS} release

tar -czf "$RPMBUILD/SOURCES/$NAME-$VERSION.tar.gz" \
    --transform "s|^\./|$NAME-$VERSION/|" \
    --exclude=./.git --exclude=./release --exclude=./build \
    --exclude=./flatpak-build-qt5 --exclude=./flatpak-build-qt6 --exclude=./local \
    .

cat > "$RPMBUILD/SPECS/$NAME.spec" << EOF
Name:           $NAME
Version:        $VERSION
Release:        $RELEASE%{?dist}
Summary:        A modern application style for Qt/KDE
License:        GPL-2.0-or-later
URL:            https://git.blossomos.org/Blossom/ui
Source0:        %{name}-%{version}.tar.gz

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
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Gui)
BuildRequires:  cmake(Qt5DBus)
BuildRequires:  cmake(KF5GuiAddons)
BuildRequires:  cmake(KF5WindowSystem)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5CoreAddons)
BuildRequires:  cmake(KF5ConfigWidgets)
BuildRequires:  cmake(KF5FrameworkIntegration)
BuildRequires:  kf5-kpackage-devel
BuildRequires:  kf5-kcmutils-devel
BuildRequires:  qt5-qtquickcontrols2-devel
BuildRequires:  kf5-kirigami2-devel

Requires:       qt6-qtbase
Requires:       kf6-kcoreaddons

%description
BlossomUI is a modern application style for Qt and KDE applications,
forked from Darkly (which is itself a fork of Lightly/KDE Breeze).

%prep
%autosetup

%build
%cmake -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
%cmake_build

%install
%cmake_install

%files
%license LICENSE.md COPYING
%{_libdir}/qt6/plugins/styles/blossomui6.so
%{_libdir}/qt6/plugins/kstyle_config/blossomuistyleconfig.so
%{_libdir}/qt6/plugins/org.kde.kdecoration3/org.kde.blossomui.so
%{_libdir}/qt6/plugins/org.kde.kdecoration3.kcm/kcm_blossomuidecoration.so
%{_libdir}/libblossomuicommon6.so*
%{_libdir}/qt5/plugins/styles/blossomui5.so
%{_libdir}/libblossomuicommon5.so*
%{_datadir}/kstyle/themes/blossomui.themerc
%{_datadir}/color-schemes/BlossomUI.colors
%{_datadir}/applications/blossomuistyleconfig.desktop
%{_datadir}/applications/kcm_blossomuidecoration.desktop
%{_datadir}/icons/hicolor/scalable/apps/blossomui-settings.svgz
%{_bindir}/blossomui-settings6
%{_libdir}/cmake/BlossomUI/
%{_datadir}/kservices6/blossomuidecoration*.desktop
%{_datadir}/plasma/look-and-feel/
%{_datadir}/color-schemes/

%changelog
* $(LC_TIME=C date "+%a %b %d %Y") packager - $VERSION-$RELEASE
- $CHANGELOG
EOF

if command -v rpmbuild >/dev/null 2>&1; then
    rpmbuild -ba "$RPMBUILD/SPECS/$NAME.spec"
    find "$RPMBUILD/RPMS" -name "$NAME-$VERSION-$RELEASE*.rpm" -exec cp {} release/ \;
else
    echo "rpmbuild not found, skipping RPM release."
fi

# Flatpak
echo " *** Building Flatpak extensions *** "
if command -v flatpak-builder >/dev/null 2>&1; then
    builder="flatpak-builder"
elif flatpak run org.flatpak.Builder --version >/dev/null 2>&1; then
    builder="flatpak run org.flatpak.Builder"
else
    echo "flatpak-builder not found, skipping Flatpak release."
    echo "Done: $(ls release/)"
    exit 0
fi

ARCH=$(uname -m)
REPO="$SRC_DIR/local"

$builder "$SRC_DIR/flatpak-build-qt6" --repo="$REPO" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI6.json"
$builder "$SRC_DIR/flatpak-build-qt5" --repo="$REPO" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI5.json"

flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt6.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/6.9"
flatpak build-bundle "$REPO" "release/${NAME}-${VERSION}-qt5.flatpak" "runtime/org.kde.KStyle.BlossomUI/$ARCH/5.15-24.08"

echo "Done: $(ls release/)"

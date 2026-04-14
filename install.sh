#!/bin/env bash

QT_BUILD=$1
SRC_DIR=$(pwd)
BUILD_DIR="$SRC_DIR/build"
CMAKE_OPTS=(
    -B $BUILD_DIR
    -S $SRC_DIR
    -DBUILD_TESTING=OFF
    -Wno-dev
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
)
PROJECT="blossomui"
_PROJECT="BlossomUI"

# look and feel theme IDs
LIGHT_THEME_ID="org.blossomos.blossomuilight.desktop"
DARK_THEME_ID="org.blossomos.blossomuidark.desktop"


build_qt6() {
    echo " *** Building with QT6 *** "
    remove_qt6_files
    if cmake "${CMAKE_OPTS[@]}" -DBUILD_QT6=ON -DBUILD_QT5=OFF \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .; then
        echo "Installation completed!"
    else
        echo "Installation failed!"
        exit 1
    fi
    cd "$SRC_DIR"
}

build_qt5() {
    echo " *** Building with QT5 *** "
    remove_qt5_files
    if cmake "${CMAKE_OPTS[@]}" -DBUILD_QT6=OFF -DBUILD_QT5=ON \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .; then
        echo "Installation completed!"
    else
        echo "Installation failed!"
        exit 1
    fi
    cd "$SRC_DIR"
}

has_qt5() {
    if command -v qmake-qt5 >/dev/null 2>&1; then
        return 0
    fi

    if command -v qmake >/dev/null 2>&1; then
        local qt_version
        qt_version=$(qmake -query QT_VERSION 2>/dev/null || true)
        [[ "$qt_version" == 5* ]]
        return $?
    fi

    return 1
}

has_kf5() {
    if ! command -v pkg-config >/dev/null 2>&1; then
        return 1
    fi

    pkg-config --exists KF5CoreAddons KF5Config KF5GuiAddons KF5I18n KF5IconThemes KF5WindowSystem
}

build_flatpak() {
    echo " *** Building Flatpak extensions *** "
    local builder
    if command -v flatpak-builder >/dev/null 2>&1; then
        builder="flatpak-builder"
    elif flatpak run org.flatpak.Builder --version >/dev/null 2>&1; then
        builder="flatpak run org.flatpak.Builder"
    else
        echo "flatpak-builder not found. Install it with: sudo dnf install flatpak-builder"
        exit 1
    fi

    local arch
    arch=$(uname -m)
    local repo="$SRC_DIR/local"

    if ! $builder "$SRC_DIR/flatpak-build-qt6" --repo="$repo" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI6.json" \
        || ! $builder "$SRC_DIR/flatpak-build-qt5" --repo="$repo" --force-clean --ccache "$SRC_DIR/org.kde.KStyle.BlossomUI5.json"; then
        echo "Flatpak build failed!"
        exit 1
    fi

    echo " *** Installing Flatpak extensions *** "

    flatpak remote-add --user --no-gpg-verify --if-not-exists blossomui-local "$repo"
    flatpak install --user --or-update --noninteractive blossomui-local "org.kde.KStyle.BlossomUI/$arch/6.9"
    flatpak install --user --or-update --noninteractive blossomui-local "org.kde.KStyle.BlossomUI/$arch/5.15-24.08"
    echo "Flatpak installation completed!"
}

build_default() {
    echo " *** Building with QT5 && QT6 *** "
    remove_qt5_files
    remove_qt6_files
    if cmake "${CMAKE_OPTS[@]}" \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .; then
        echo "Installation completed!"
    else
        echo "Installation failed!"
        exit 1
    fi
    cd "$SRC_DIR"
}

remove_build() {
    if [ -d "$SRC_DIR/build" ]; then
        echo "Removing existing build directory"
        sudo rm -rf $SRC_DIR/build
    fi

    cd "$SRC_DIR"
}

remove_qt6_files() {
    files=(
        "/usr/lib64/qt6/plugins/styles/${PROJECT}6.so*"
        "/usr/lib/qt6/plugins/styles/${PROJECT}6.so*"
        "/usr/share/kstyle/themes/${PROJECT}.themerc"
        "/usr/lib64/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*"
        "/usr/lib/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*"
        "/usr/share/applications/${PROJECT}styleconfig.desktop"
        "/usr/bin/${PROJECT}-settings6"
        "/usr/share/icons/hicolor/scalable/apps/${PROJECT}-settings.svgz"
        "/usr/lib64/lib${PROJECT}common6.so*"
        "/usr/lib/lib${PROJECT}common6.so.*"
        "/usr/lib64/lib${PROJECT}common6.so*"
        "/usr/lib/lib${PROJECT}common6.so*"
        "/usr/lib64/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*"
        "/usr/lib/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*"
        "/usr/lib64/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*"
        "/usr/lib/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*"
        "/usr/share/applications/kcm_${PROJECT}decoration.desktop"
        "/usr/lib64/cmake/${PROJECT}/${PROJECT}Config.cmake"
        "/usr/lib/cmake/${PROJECT}/${PROJECT}Config.cmake"
        "/usr/lib64/cmake/${PROJECT}/${PROJECT}ConfigVersion.cmake"
        "/usr/lib/cmake/${PROJECT}/${PROJECT}ConfigVersion.cmake"
        "/usr/share/color-schemes/${_PROJECT}.colors"
        /usr/lib/cmake/"${PROJECT^}"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3/org.kde.${PROJECT}.so*"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/kstyle_config/${PROJECT}styleconfig.so*"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/org.kde.kdecoration3.kcm/kcm_${PROJECT}decoration.so*"
        "/usr/lib/x86_64-linux-gnu/qt6/plugins/styles/${PROJECT}6.so*"
        "/usr/share/kservices6/${PROJECT}decorationconfig.desktop"
    )

    for f in ${files[@]}; do
        sudo rm -rf "$f"
    done
}

# if existing
remove_qt5_files() {
    files=(
        "/usr/lib64/qt5/plugins/styles/${PROJECT}5.so*"
        "/usr/lib/qt5/plugins/styles/${PROJECT}5.so*"
        "/usr/lib64/lib${PROJECT}common5.so*"
        "/usr/lib/lib${PROJECT}common5.so*"
        "/usr/lib64/lib${PROJECT}common5.so*"
        "/usr/lib/lib${PROJECT}common5.so*"
        "/usr/lib64/qt/plugins/styles/${PROJECT}5.so*"
        "/usr/lib/x86_64-linux-gnu/qt5/plugins/styles/${PROJECT}5.so*"
    )

    for f in ${files[@]}; do
        sudo rm -rf "$f"
    done
}

case "$QT_BUILD" in
qt5 | QT5)
    build_qt5
    ;;
qt6 | QT6)
    build_qt6
    ;;
flatpak | FLATPAK)
    build_flatpak
    ;;
remove)
    remove_qt5_files
    remove_qt6_files
    ;;
*)
    build_default
    ;;
esac

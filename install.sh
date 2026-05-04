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

build_qt6() {
    echo " *** Building with QT6 *** "
    cmake "${CMAKE_OPTS[@]}" -DBUILD_QT6=ON -DBUILD_QT5=OFF \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .
    cd "$SRC_DIR"
}

build_qt5() {
    echo " *** Building with QT5 *** "
    cmake "${CMAKE_OPTS[@]}" -DBUILD_QT6=OFF -DBUILD_QT5=ON \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .
    cd "$SRC_DIR"
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

    $builder "$SRC_DIR/flatpak-build-qt6" --repo="$repo" --force-clean --ccache "$SRC_DIR/flatpak/org.blossomos.ui-qt6.json" \
        && $builder "$SRC_DIR/flatpak-build-qt5" --repo="$repo" --force-clean --ccache "$SRC_DIR/flatpak/org.blossomos.ui-qt5.json"

    echo " *** Installing Flatpak extensions *** "
    flatpak remote-add --user --no-gpg-verify --if-not-exists blossomui-local "$repo"
    flatpak install --user --or-update --noninteractive blossomui-local "org.blossomos.ui/$arch/6.9"
    flatpak install --user --or-update --noninteractive blossomui-local "org.blossomos.ui/$arch/5.15-24.08"
}

build_default() {
    echo " *** Building with QT5 && QT6 *** "
    cmake "${CMAKE_OPTS[@]}" \
        && cmake --build $BUILD_DIR -j $(nproc) \
        && cd $BUILD_DIR \
        && sudo cmake --install .
    cd "$SRC_DIR"
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
    echo "Use ./uninstall.sh to remove BlossomUI"
    exit 1
    ;;
*)
    build_default
    ;;
esac

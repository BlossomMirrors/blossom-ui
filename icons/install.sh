#!/usr/bin/env bash

THEME_NAME="BlossomUI Icons"
INSTALL_DIR="/usr/share/icons/${THEME_NAME}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Installing '${THEME_NAME}' to ${INSTALL_DIR}..."
sudo rm -rf "${INSTALL_DIR}"
sudo mkdir -p "${INSTALL_DIR}"
sudo cp -r "${SCRIPT_DIR}/." "${INSTALL_DIR}/"
sudo gtk-update-icon-cache -f -t "${INSTALL_DIR}"

echo "Rebuilding icon cache..."
rm -f ~/.cache/icon-cache.kcache
kbuildsycoca6 --noincremental

echo "Reloading KWin..."
qdbus org.kde.KWin /KWin reconfigure 2>/dev/null || true

echo "Done."

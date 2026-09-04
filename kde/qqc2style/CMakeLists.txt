# SPDX-License-Identifier: GPL-2.0-or-later
#
# org.blossomos.style — QtQuick Controls style for BlossomUI.
# Installed alongside org.kde.desktop/org.kde.union and selected via
# QT_QUICK_CONTROLS_STYLE=org.blossomos.style; overwrites no system file.
# It does add one directory inside Kirigami's module tree (see below), which
# no other package owns — the same mechanism plasma-union uses.

add_subdirectory(kirigami-integration)

install(DIRECTORY style/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/blossomos/style)

# Kirigami's StyleSelector resolves each control through
# org/kde/kirigami/styles/<style-name>/ before falling back to
# org/kde/kirigami/controls/, so this is how a style overrides a Kirigami
# control. The directory is named after us and owned by us alone.
install(DIRECTORY kirigami-styles/
        DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami/styles/org.blossomos.style)

option(INSTALL_QQC2_STYLE_ENV "Install Plasma env script that activates the Blossom QQC2 style session-wide" OFF)
if(INSTALL_QQC2_STYLE_ENV)
    # KDE_INSTALL_FULL_SYSCONFDIR doesn't apply the /usr-prefix -> /etc FHS
    # special case (resolves to /usr/etc); CMAKE_INSTALL_FULL_SYSCONFDIR
    # (plain GNUInstallDirs) does.
    install(PROGRAMS env/blossom-qqc2-style.sh DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/plasma-workspace/env)
endif()

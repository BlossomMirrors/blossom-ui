# SPDX-License-Identifier: GPL-2.0-or-later
#
# Kirigami platform plugin for the org.blossomos.style QQC2 style.
# Kirigami discovers this plugin by matching the QT_QUICK_CONTROLS_STYLE name
# against plugin filenames in <plugindir>/kf6/kirigami/platform/, so the
# target name must contain "org.blossomos.style".
#
# Sources are copied from qqc2-desktop-style v6.27.0
# (kirigami-plasmadesktop-integration/) so Kirigami.Theme colors come from the
# active KColorScheme (the Blossom color schemes) and Units/animation speeds
# follow kdeglobals. Architecture mirrors Union's src/output/kirigami/.

set(blossom_kirigami_platform_SRCS
    animationspeedprovider.cpp
    plasmadesktoptheme.cpp
    kirigamiplasmafactory.cpp
    plasmadesktopunits.cpp
)

add_library(org.blossomos.style MODULE ${blossom_kirigami_platform_SRCS})

target_link_libraries(org.blossomos.style
    PUBLIC
        Qt6::Core
        KF6::KirigamiPlatform
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        KF6::ConfigCore
        KF6::ColorScheme
        KF6::IconThemes
)

if(TARGET Qt6::DBus)
    target_link_libraries(org.blossomos.style PRIVATE Qt6::DBus)
    target_compile_definitions(org.blossomos.style PRIVATE -DHAVE_QTDBUS=1)
else()
    target_compile_definitions(org.blossomos.style PRIVATE -DHAVE_QTDBUS=0)
endif()

if(WIN32)
    target_link_libraries(org.blossomos.style PRIVATE user32)
endif()

install(TARGETS org.blossomos.style DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kirigami/platform)

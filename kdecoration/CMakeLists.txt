add_definitions(-DTRANSLATION_DOMAIN="blossomui_kwin_deco")

################# configuration #################
configure_file(config-blossomui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-blossomui.h )

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libblossomuicommon)
include_directories(${CMAKE_BINARY_DIR}/libblossomuicommon)

################# newt target #################
### plugin classes
set(blossomuidecoration_SRCS
    blossomuibutton.cpp
    blossomuidecoration.cpp
    blossomuiexceptionlist.cpp
    blossomuisettingsprovider.cpp
)
### build library
kconfig_add_kcfg_files(blossomuidecoration_SRCS blossomuisettings.kcfgc)
add_library(blossomuidecoration MODULE
    ${blossomuidecoration_SRCS}
    ${blossomuidecoration_config_SRCS}
    ${blossomuidecoration_config_PART_FORMS_HEADERS})

set_target_properties(blossomuidecoration PROPERTIES
    OUTPUT_NAME org.kde.blossomui
)

target_link_libraries(blossomuidecoration
     PUBLIC
        Qt6::DBus
        Qt6::Svg
        KDecoration3::KDecoration
     PRIVATE
        blossomuicommon6
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KF6::KCMUtils
)
install(TARGETS blossomuidecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})
install(FILES config/blossomuidecorationconfig.desktop DESTINATION  ${KDE_INSTALL_DATADIR}/kservices6)

install(FILES ${CMAKE_SOURCE_DIR}/icons/close.svg
              ${CMAKE_SOURCE_DIR}/icons/maximize.svg
              ${CMAKE_SOURCE_DIR}/icons/normalize.svg
              ${CMAKE_SOURCE_DIR}/icons/minimize.svg
    DESTINATION ${KDE_INSTALL_DATADIR}/blossomui/icons)

add_subdirectory(config)

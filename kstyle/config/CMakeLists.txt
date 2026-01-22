### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"blossomui_style_config\")
set(kstyle_blossomui_config_PART_SRCS blossomuistyleconfig.cpp)

set(kstyle_blossomui_config_PART_SRCS ${kstyle_blossomui_config_PART_SRCS} blossomuistyleconfigmodule.cpp)

kconfig_add_kcfg_files(kstyle_blossomui_config_PART_SRCS ../blossomuistyleconfigdata.kcfgc)
set(kstyle_blossomui_config_PART_FORMS ui/blossomuistyleconfig.ui)
ki18n_wrap_ui(kstyle_blossomui_config_PART_FORMS_HEADERS ${kstyle_blossomui_config_PART_FORMS})

kcoreaddons_add_plugin(blossomuistyleconfig SOURCES
    ${kstyle_blossomui_config_PART_SRCS}
    ${kstyle_blossomui_config_PART_FORMS_HEADERS}
    INSTALL_NAMESPACE "kstyle_config"
)

target_link_libraries(blossomuistyleconfig Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)
target_link_libraries(blossomuistyleconfig KF6::CoreAddons KF6::ConfigCore KF6::ConfigWidgets KF6::I18n)
if(QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(blossomuistyleconfig KF6::KCMUtils)
endif()
kcmutils_generate_desktop_file(blossomuistyleconfig)

set(blossomui_settings_SOURCES main.cpp)
add_executable(blossomui-settings ${blossomui_settings_SOURCES} )
target_link_libraries(blossomui-settings Qt6::Core Qt6::Gui Qt6::Widgets )
target_link_libraries(blossomui-settings KF6::I18n KF6::KCMUtils KF6::CoreAddons)

set_target_properties(blossomui-settings PROPERTIES OUTPUT_NAME blossomui-settings${QT_MAJOR_VERSION})

install(TARGETS blossomui-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# icon
ecm_install_icons(ICONS sc-apps-blossomui-settings.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor )



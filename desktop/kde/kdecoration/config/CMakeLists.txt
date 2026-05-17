#####  config classes ææææææ

set(blossomuidecoration_config_SRCS
  ../core/blossomuiexceptionlist.cpp
    blossomuiconfigwidget.cpp
    blossomuidetectwidget.cpp
    blossomuiexceptiondialog.cpp
    blossomuiexceptionlistwidget.cpp
    blossomuiexceptionmodel.cpp
    blossomuiitemmodel.cpp
)
ki18n_wrap_ui(blossomuidecoration_config_SRCS
    ui/blossomuiconfigurationui.ui
    ui/blossomuidetectwidget.ui
    ui/blossomuiexceptiondialog.ui
    ui/blossomuiexceptionlistwidget.ui
)

kcoreaddons_add_plugin(kcm_blossomuidecoration SOURCES kcm_blossomuidecoration.cpp ${blossomuidecoration_config_SRCS} INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
kconfig_add_kcfg_files(kcm_blossomuidecoration blossomuisettings.kcfgc)
target_include_directories(kcm_blossomuidecoration PRIVATE ${CMAKE_SOURCE_DIR}/kde/kdecoration ${CMAKE_BINARY_DIR}/kde/kdecoration/)
target_link_libraries(kcm_blossomuidecoration
     PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::DBus
        KDecoration3::KDecoration
     PRIVATE
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KF6::KCMUtils
)
kcmutils_generate_desktop_file(kcm_blossomuidecoration)

################# includes #################
include_directories(
    style
    core/animations
    core/debug
    core/support
    ${CMAKE_SOURCE_DIR}/kde/libblossomuicommon
    ${CMAKE_BINARY_DIR}/libblossomuicommon
)

################# configuration #################
configure_file(config-blossomui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-blossomui.h)

########### next target ###############
set(blossomui_PART_SRCS
    core/animations/blossomuianimation.cpp
    core/animations/blossomuianimations.cpp
    core/animations/blossomuianimationdata.cpp
    core/animations/blossomuibaseengine.cpp
    core/animations/blossomuibusyindicatordata.cpp
    core/animations/blossomuibusyindicatorengine.cpp
    core/animations/blossomuidialdata.cpp
    core/animations/blossomuidialengine.cpp
    core/animations/blossomuienabledata.cpp
    core/animations/blossomuigenericdata.cpp
    core/animations/blossomuiheaderviewdata.cpp
    core/animations/blossomuiheaderviewengine.cpp
    core/animations/blossomuiscrollbardata.cpp
    core/animations/blossomuiscrollbarengine.cpp
    core/animations/blossomuispinboxengine.cpp
    core/animations/blossomuispinboxdata.cpp
    core/animations/blossomuistackedwidgetdata.cpp
    core/animations/blossomuistackedwidgetengine.cpp
    core/animations/blossomuitabbarengine.cpp
    core/animations/blossomuitabbardata.cpp
    core/animations/blossomuitoolboxengine.cpp
    core/animations/blossomuitransitiondata.cpp
    core/animations/blossomuitransitionwidget.cpp
    core/animations/blossomuiwidgetstateengine.cpp
    core/animations/blossomuiwidgetstatedata.cpp
    core/debug/blossomuiwidgetexplorer.cpp
    core/support/blossomuiaddeventfilter.cpp
    core/support/blossomuiblurhelper.cpp
    core/support/blossomuiframeshadow.cpp
    core/support/blossomuimdiwindowshadow.cpp
    core/support/blossomuimnemonics.cpp
    core/support/blossomuipropertynames.cpp
    core/support/blossomuishadowhelper.cpp
    core/support/blossomuisplitterproxy.cpp
    core/support/blossomuistyleplugin.cpp
    core/support/blossomuitileset.cpp
    core/support/blossomuitoolsareamanager.cpp
    core/support/blossomuiwindowmanager.cpp
    blossomuihelper.cpp
    style/widgets/switch.h
    style/blossomuistyle.cpp
    style/eventfilter.cpp
    style/dolphin/dockwidget.cpp
    style/dolphin/card.cpp
    style/dolphin/urlinput.cpp
    style/metrics.cpp
    style/geometry.cpp
    style/utils.cpp
    style/primitives/frames.cpp
    style/primitives/indicators.cpp
    style/primitives/panels.cpp
    style/widgets/button.cpp
    style/widgets/checkbox.cpp
    style/widgets/combobox.cpp
    style/widgets/groupbox.cpp
    style/widgets/header.cpp
    style/widgets/menu.cpp
    style/widgets/misc.cpp
    style/widgets/progressbar.cpp
    style/widgets/scrollbar.cpp
    style/widgets/slider.cpp
    style/widgets/spinbox.cpp
    style/widgets/tabs.cpp
    style/widgets/titlebar.cpp
    style/widgets/toolbar.cpp
)

kconfig_add_kcfg_files(blossomui_PART_SRCS ../kdecoration/config/blossomuisettings.kcfgc)
kconfig_add_kcfg_files(blossomui_PART_SRCS blossomuistyleconfigdata.kcfgc)

add_library(blossomui${QT_MAJOR_VERSION} MODULE ${blossomui_PART_SRCS})
set_target_properties(blossomui${QT_MAJOR_VERSION} PROPERTIES
    OUTPUT_NAME "blossomui${QT_MAJOR_VERSION}"
)

################# find Qt modules #################
if(QT_MAJOR_VERSION EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickWidgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Quick QuickWidgets)
endif()

################# link Qt libraries #################
target_link_libraries(blossomui${QT_MAJOR_VERSION}
    PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::DBus
    Qt${QT_MAJOR_VERSION}::Quick
)

if( BLOSSOMUI_HAVE_QTQUICK )
    target_link_libraries(blossomui${QT_MAJOR_VERSION}
        PRIVATE
            Qt${QT_MAJOR_VERSION}::QuickWidgets
            KF${QT_MAJOR_VERSION}::CoreAddons
    )
endif()

################# link KDE Frameworks #################
target_link_libraries(blossomui${QT_MAJOR_VERSION}
    PRIVATE
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::GuiAddons
    KF${QT_MAJOR_VERSION}::IconThemes
    KF${QT_MAJOR_VERSION}::WindowSystem
)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE KF5::ConfigWidgets)
    if(BLOSSOMUI_HAVE_QTQUICK)
        target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE KF5::Kirigami2)
    endif()
else()
    target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE KF6::ColorScheme)
    if(BLOSSOMUI_HAVE_QTQUICK)
        target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE KF6::KirigamiPlatform)
    endif()
endif()

################# link internal library #################
target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE blossomuicommon${QT_MAJOR_VERSION})

################# KDE style integration #################
if(KF${QT_MAJOR_VERSION}FrameworkIntegration_FOUND)
    target_link_libraries(blossomui${QT_MAJOR_VERSION} PRIVATE KF${QT_MAJOR_VERSION}::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(blossomui${QT_MAJOR_VERSION} PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()


########### install files ###############
if (FOR_FLATPAK)
    set(LIB_TARGET ${QT_PLUGINS_DIR}/styles/)
else()
    set(LIB_TARGET ${KDE_INSTALL_QTPLUGINDIR}/styles/)
endif()
install(TARGETS blossomui${QT_MAJOR_VERSION} DESTINATION ${LIB_TARGET})

if (NOT FOR_FLATPAK)
    install(FILES blossomui.themerc  DESTINATION  ${KDE_INSTALL_DATADIR}/kstyle/themes)
endif()

########### subdirectories ###############

if (QT_MAJOR_VERSION EQUAL "6" AND TARGET "KF6::KCMUtils" AND NOT FOR_FLATPAK)
    add_subdirectory(config)
endif()

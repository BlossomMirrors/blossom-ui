################# includes #################
include_directories(
    animations
    debug
    ${CMAKE_SOURCE_DIR}/libblossomuicommon
    ${CMAKE_BINARY_DIR}/libblossomuicommon
)

################# configuration #################
configure_file(config-blossomui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-blossomui.h)

########### next target ###############
set(blossomui_PART_SRCS
    animations/blossomuianimation.cpp
    animations/blossomuianimations.cpp
    animations/blossomuianimationdata.cpp
    animations/blossomuibaseengine.cpp
    animations/blossomuibusyindicatordata.cpp
    animations/blossomuibusyindicatorengine.cpp
    animations/blossomuidialdata.cpp
    animations/blossomuidialengine.cpp
    animations/blossomuienabledata.cpp
    animations/blossomuigenericdata.cpp
    animations/blossomuiheaderviewdata.cpp
    animations/blossomuiheaderviewengine.cpp
    animations/blossomuiscrollbardata.cpp
    animations/blossomuiscrollbarengine.cpp
    animations/blossomuispinboxengine.cpp
    animations/blossomuispinboxdata.cpp
    animations/blossomuistackedwidgetdata.cpp
    animations/blossomuistackedwidgetengine.cpp
    animations/blossomuitabbarengine.cpp
    animations/blossomuitabbardata.cpp
    animations/blossomuitoolboxengine.cpp
    animations/blossomuitransitiondata.cpp
    animations/blossomuitransitionwidget.cpp
    animations/blossomuiwidgetstateengine.cpp
    animations/blossomuiwidgetstatedata.cpp
    debug/blossomuiwidgetexplorer.cpp
    blossomuiaddeventfilter.cpp
    blossomuiblurhelper.cpp
    blossomuiframeshadow.cpp
    blossomuihelper.cpp
    blossomuimdiwindowshadow.cpp
    blossomuimnemonics.cpp
    blossomuipropertynames.cpp
    blossomuishadowhelper.cpp
    blossomuisplitterproxy.cpp
    blossomuiswitchwidget.cpp
    blossomuistyle.cpp
    blossomuistyleplugin.cpp
    blossomuitileset.cpp
    blossomuiwindowmanager.cpp
    blossomuitoolsareamanager.cpp
)

kconfig_add_kcfg_files(blossomui_PART_SRCS ../kdecoration/blossomuisettings.kcfgc)
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

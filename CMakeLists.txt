cmake_minimum_required(VERSION 3.19)

# Get version
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VER)

project(
    blossomui
    DESCRIPTION "A modern style for Qt applications."
    VERSION ${VER}
)

set(KF5_MIN_VERSION "5.102.0")
set(KF6_MIN_VERSION "6.10.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT5_MIN_VERSION "5.15.2")
set(QT_MIN_VERSION "6.7.0")

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include(FeatureSummary)
include(GNUInstallDirs)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
#find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH}  ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(BUILD_RPMS OR BUILD_DEBS)
  include(cmake/CPackLists.cmake)
endif()

include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(GtkUpdateIconCache)

option(FOR_FLATPAK "Build BlossomUI for Flatpak" OFF)
option(BUILD_QT5 "Build with QT5" ON)
option(BUILD_QT6 "Build with QT6" ON)
option(BUILD_GTK_THEME "Build the BlossomUI GTK theme (requires sassc)" ON)

set(QT_NO_CREATE_VERSIONLESS_TARGETS ON)
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS ON)

if(WIN32 OR APPLE OR FOR_FLATPAK)
  set(WITH_DECORATIONS OFF)
endif()

function(build_qt5)
  set(QT_MAJOR_VERSION 5)
  include(KDEInstallDirs5)
  include(KDECMakeSettings)
  option(WITH_DECORATIONS "Build BlossomUI window decorations for KWin" ON)

  find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets DBus)
  find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
            CoreAddons
            Config
            GuiAddons
            I18n
            IconThemes
            WindowSystem)
  if(NOT WIN32 AND NOT APPLE)
    find_package(KF5KCMUtils ${KF5_MIN_VERSION})
    set_package_properties(KF5KCMUtils PROPERTIES
                TYPE REQUIRED
                DESCRIPTION "Helps create configuration modules"
                PURPOSE "KCMUtils used for the configuration modules or the decoration and Qt Style"
            )
  endif()

  find_package(Qt5 ${QT5_MIN_VERSION} OPTIONAL_COMPONENTS Quick)
  if(${Qt5Quick_FOUND})
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2)
  endif()

  find_package(KF5FrameworkIntegration ${KF5_MIN_VERSION} CONFIG )
  set_package_properties(KF${QT_MAJOR_VERSION}FrameworkIntegration PROPERTIES
            DESCRIPTION "KF${QT_MAJOR_VERSION} Framework Integration"
            URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
            TYPE OPTIONAL
            PURPOSE "Required to use KStyle convenience functionalities in style")

  set(BLOSSOMUI_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})
  set(BLOSSOMUI_HAVE_QTQUICK ${Qt5Quick_FOUND})


  add_subdirectory(kde/kstyle kstyle5)
  add_subdirectory(kde/libblossomuicommon libblossomuicommon5)
  unset(QUERY_EXECUTABLE CACHE)
endfunction()
if(BUILD_QT5)
  message("-- **** Build QT5 - On ****")
  build_qt5()
endif()

function(build_qt6)
  set(QT_MAJOR_VERSION 6)
  include(KDEInstallDirs6)
  include(KDECMakeSettings)

  option(WITH_DECORATIONS "Build BlossomUI window decorations for KWin" ON)

  find_package(Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets DBus Svg)
  find_package(ZLIB REQUIRED)
  find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
            CoreAddons
            ColorScheme
            Config
            GuiAddons
            I18n
            IconThemes
            WindowSystem)
  if(NOT WIN32 AND NOT APPLE)
    find_package(KF6KCMUtils ${KF6_MIN_VERSION})
    set_package_properties(KF6KCMUtils PROPERTIES
                TYPE REQUIRED
                DESCRIPTION "Helps create configuration modules"
                PURPOSE "KCMUtils used for the configuration modules or the decoration and Qt Style"
            )
  endif()

  find_package(Qt6 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Quick)
  if(${Qt6Quick_FOUND})
    find_package(KF6KirigamiPlatform ${KF6_MIN_VERSION} REQUIRED)
  endif()

  find_package(KF6FrameworkIntegration ${KF6_MIN_VERSION} CONFIG )
  set_package_properties(KF${QT_MAJOR_VERSION}FrameworkIntegration PROPERTIES
            DESCRIPTION "KF${QT_MAJOR_VERSION} Framework Integration"
            URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
            TYPE OPTIONAL
            PURPOSE "Required to use KStyle convenience functionalities in style")

  set(BLOSSOMUI_HAVE_KSTYLE ${KF6FrameworkIntegration_FOUND})
  set(BLOSSOMUI_HAVE_QTQUICK ${Qt6Quick_FOUND})


  add_library(Qt::Core ALIAS Qt6::Core)
  add_library(Qt::Gui ALIAS Qt6::Gui)
  add_library(Qt::OpenGL ALIAS Qt6::OpenGL)
  add_library(Qt::Network ALIAS Qt6::Network)
  if(Qt6_VERSION_MINOR GREATER 6)
    add_library(Qt::PlatformModuleInternal ALIAS Qt6::PlatformModuleInternal)
  endif()

  add_subdirectory(kde/kstyle kstyle6)
  add_subdirectory(kde/libblossomuicommon libblossomuicommon6)

  if(${Qt6Quick_FOUND})
    add_subdirectory(kde/qqc2style)
  endif()

  if(WITH_DECORATIONS)
    find_package(KDecoration3 REQUIRED)
    add_subdirectory(kde/kdecoration)
  endif()

  ki18n_install(po)

  # create a Config.cmake and a ConfigVersion.cmake file and install them
  include(ECMSetupVersion)
  ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX BLOSSOMUI
                    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIConfigVersion.cmake"
                    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kstyle6/config/blossomuistyleversion.h"
    )



  set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/BlossomUI")

  if (NOT FOR_FLATPAK)
    configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/BlossomUIConfig.cmake.in"
                                        "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIConfig.cmake"
                                        PATH_VARS KDE_INSTALL_FULL_DATADIR
                                        INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                    )

    install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIConfig.cmake"
                        "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIConfigVersion.cmake"
                    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
                    COMPONENT Devel
                )
  endif()
  unset(QUERY_EXECUTABLE CACHE)
endfunction()

if(BUILD_QT6)
  message("-- **** Build QT6 - On ****")
  build_qt6()
endif()

function(build_colors)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(kde/colors)
endfunction()

function(build_lookandfeel)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(kde/lookandfeel)
endfunction()

function(build_desktoptheme)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(kde/desktoptheme)
endfunction()

function(build_wallpapers)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(wallpapers)
endfunction()

function(build_icons)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(icons)
endfunction()

function(build_fonts)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(fonts)
endfunction()

function(build_gtk)
  include(KDEInstallDirs${QT_MAJOR_VERSION})
  add_subdirectory(gtk)
endfunction()

if (NOT FOR_FLATPAK)
  build_colors()
  build_lookandfeel()
  build_desktoptheme()
  build_wallpapers()
  build_icons()
  build_fonts()
  if(BUILD_GTK_THEME)
    build_gtk()
  endif()
endif()

# Set default QT_MAJOR_VERSION for asset builds (icons, themes, etc.)
# This ensures KDEInstallDirs6 is used for proper installation paths
if(NOT DEFINED QT_MAJOR_VERSION)
  set(QT_MAJOR_VERSION 6)
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

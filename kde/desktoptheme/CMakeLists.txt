configure_file(metadata.json.in "${CMAKE_CURRENT_BINARY_DIR}/metadata.json" @ONLY)

install(DIRECTORY .
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme/BlossomUI
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "metadata.json.in" EXCLUDE)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme/BlossomUI)

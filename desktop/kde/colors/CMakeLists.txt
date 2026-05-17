include(BlossomUIColors)

configure_file(BlossomUIDark.colors.in     "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIDark.colors"     @ONLY)
configure_file(BlossomUILight.colors.in    "${CMAKE_CURRENT_BINARY_DIR}/BlossomUILight.colors"    @ONLY)
configure_file(BlossomUIDarkOLED.colors.in "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIDarkOLED.colors" @ONLY)
configure_file(BlossomUI.colorscheme.in    "${CMAKE_CURRENT_BINARY_DIR}/BlossomUI.colorscheme"    @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIDark.colors"
    "${CMAKE_CURRENT_BINARY_DIR}/BlossomUILight.colors"
    "${CMAKE_CURRENT_BINARY_DIR}/BlossomUIDarkOLED.colors"
    DESTINATION ${KDE_INSTALL_DATADIR}/color-schemes/)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/BlossomUI.colorscheme"
        DESTINATION ${KDE_INSTALL_DATADIR}/konsole/)

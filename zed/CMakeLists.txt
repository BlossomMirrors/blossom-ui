include(BlossomUIColors)

configure_file(
  blossomui/themes/blossomui.json.in
  "${CMAKE_CURRENT_BINARY_DIR}/blossomui.json" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blossomui.json"
    DESTINATION ${KDE_INSTALL_DATADIR}/blossomui/zed/themes/)

install(FILES blossomui/extension.toml
    DESTINATION ${KDE_INSTALL_DATADIR}/blossomui/zed/)

find_program(SASSC_EXECUTABLE NAMES sassc)
if(NOT SASSC_EXECUTABLE)
  message(FATAL_ERROR "sassc is required to build the BlossomUI GTK theme")
endif()

set(GTK_THEME_NAME BlossomUI)
set(GTK_THEME_DESTDIR ${KDE_INSTALL_DATADIR}/themes/${GTK_THEME_NAME})
set(GTK_THEME_FULL_DESTDIR ${KDE_INSTALL_FULL_DATADIR}/themes/${GTK_THEME_NAME})
set(SASSC_OPTS -M -t compact)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gtk3-light.css
  COMMAND ${SASSC_EXECUTABLE} ${SASSC_OPTS} ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk3-light.scss ${CMAKE_CURRENT_BINARY_DIR}/gtk3-light.css
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk3-light.scss
  COMMENT "Compiling BlossomUI GTK3 light theme"
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gtk3-dark.css
  COMMAND ${SASSC_EXECUTABLE} ${SASSC_OPTS} ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk3-dark.scss ${CMAKE_CURRENT_BINARY_DIR}/gtk3-dark.css
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk3-dark.scss
  COMMENT "Compiling BlossomUI GTK3 dark theme"
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gtk4.css
  COMMAND ${SASSC_EXECUTABLE} ${SASSC_OPTS} ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk4.scss ${CMAKE_CURRENT_BINARY_DIR}/gtk4.css
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sass/gtk4.scss
  COMMENT "Compiling BlossomUI GTK4 theme"
)

add_custom_target(blossomui-gtk-css ALL DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/gtk3-light.css
  ${CMAKE_CURRENT_BINARY_DIR}/gtk3-dark.css
  ${CMAKE_CURRENT_BINARY_DIR}/gtk4.css
)

install(FILES index.theme DESTINATION ${GTK_THEME_DESTDIR})

install(DIRECTORY assets/
        DESTINATION ${GTK_THEME_DESTDIR}/assets
        FILES_MATCHING PATTERN "*.png" PATTERN "*.svg")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gtk3-light.css DESTINATION ${GTK_THEME_DESTDIR}/gtk-3.0 RENAME gtk.css)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gtk3-dark.css DESTINATION ${GTK_THEME_DESTDIR}/gtk-3.0 RENAME gtk-dark.css)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gtk4.css DESTINATION ${GTK_THEME_DESTDIR}/gtk-4.0 RENAME gtk.css)

# the compiled CSS references its icons via a relative "blossomui-gtk-assets" path,
# and GTK4 has no separate dark stylesheet, so gtk-dark.css is just an alias of gtk.css
install(CODE "
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../assets \"\$ENV{DESTDIR}${GTK_THEME_FULL_DESTDIR}/gtk-3.0/blossomui-gtk-assets\")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../assets \"\$ENV{DESTDIR}${GTK_THEME_FULL_DESTDIR}/gtk-4.0/blossomui-gtk-assets\")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ./gtk.css \"\$ENV{DESTDIR}${GTK_THEME_FULL_DESTDIR}/gtk-4.0/gtk-dark.css\")
")

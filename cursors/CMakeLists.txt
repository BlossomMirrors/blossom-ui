# bibata points window edges and corners at its side and corner cursors so redirect them to its double arrows
set(CURSOR_ARROWS
    "n-resize:sb_v_double_arrow"
    "s-resize:sb_v_double_arrow"
    "top_side:sb_v_double_arrow"
    "bottom_side:sb_v_double_arrow"
    "e-resize:sb_h_double_arrow"
    "w-resize:sb_h_double_arrow"
    "left_side:sb_h_double_arrow"
    "right_side:sb_h_double_arrow"
    "ne-resize:fd_double_arrow"
    "sw-resize:fd_double_arrow"
    "top_right_corner:fd_double_arrow"
    "bottom_left_corner:fd_double_arrow"
    "size-bdiag:fd_double_arrow"
    "nw-resize:bd_double_arrow"
    "se-resize:bd_double_arrow"
    "top_left_corner:bd_double_arrow"
    "bottom_right_corner:bd_double_arrow"
    "size-fdiag:bd_double_arrow")

# modern has rounded double arrows and the classic themes take the sharp ones from bibata original
set(CURSOR_THEMES
    "Bibata-Blossom-Black:Bibata-Modern-Classic"
    "Bibata-Blossom-White:Bibata-Modern-Ice"
    "Bibata-Blossom-Classic-Black:Bibata-Original-Classic"
    "Bibata-Blossom-Classic-White:Bibata-Original-Ice")

foreach(entry ${CURSOR_THEMES})
  string(REPLACE ":" ";" parts ${entry})
  list(GET parts 0 theme)
  list(GET parts 1 base)

  set(out ${CMAKE_CURRENT_BINARY_DIR}/${theme})
  file(REMOVE_RECURSE ${out})
  file(MAKE_DIRECTORY ${out}/cursors)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${theme}/index.theme DESTINATION ${out})

  # the links point into the bibata theme next to ours in the icons folder so they only resolve once installed
  foreach(arrow ${CURSOR_ARROWS})
    string(REPLACE ":" ";" pair ${arrow})
    list(GET pair 0 name)
    list(GET pair 1 target)
    file(CREATE_LINK ../../${base}/cursors/${target} ${out}/cursors/${name} SYMBOLIC)
  endforeach()

  install(DIRECTORY ${out} DESTINATION ${KDE_INSTALL_ICONDIR})
endforeach()

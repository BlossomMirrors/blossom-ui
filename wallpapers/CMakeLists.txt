file(GLOB WALLPAPER_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach(dir ${WALLPAPER_DIRS})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
    install(DIRECTORY ${dir}
                DESTINATION ${KDE_INSTALL_DATADIR}/wallpapers)
  endif()
endforeach()

install(DIRECTORY .
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/BlossomUI-Icons
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "install.sh" EXCLUDE
        PATTERN "icongen.py" EXCLUDE
        PATTERN "requirements.txt" EXCLUDE)

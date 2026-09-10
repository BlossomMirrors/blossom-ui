# Install the icon theme directory
# Note: source directory MUST be included because icons are symlinks to it
install(DIRECTORY .
        DESTINATION ${KDE_INSTALL_ICONDIR}/BlossomUI
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "install.sh" EXCLUDE
        PATTERN "icongen.py" EXCLUDE
        PATTERN "foldergen.py" EXCLUDE
        PATTERN "icons.json" EXCLUDE
        PATTERN "MISSING-ICONS.md" EXCLUDE
        PATTERN "requirements.txt" EXCLUDE
        PATTERN "__pycache__" EXCLUDE
        PATTERN "folder-base.svg" EXCLUDE)

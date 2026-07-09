
install(FILES 10-blossomui.conf
        DESTINATION /etc/plasmalogin.conf.d)

install(FILES greeter-kdeglobals greeter-plasmarc
        DESTINATION ${KDE_INSTALL_DATADIR}/blossomui/plasmalogin)

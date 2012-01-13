# Make this folder installable on its own as long as the port to tpqt4-0.5 is not finished
find_package(KDE4)

kde4_install_icons( ${ICON_INSTALL_DIR} )
kde4_update_iconcache()

project (telepathy-kde-contactlist)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 4.4.75 REQUIRED)
find_package (TelepathyQt4 REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)

set (contactlist_SRCS
     contact-view-hover-button.cpp
     contact-overlays.cpp
     contact-delegate-overlay.cpp
     contact-delegate.cpp
     account-button.cpp
     accounts-model.cpp
     account-filter-model.cpp
     contact-model-item.cpp
     tree-node.cpp
     accounts-model-item.cpp
     filter-bar.cpp
     main.cpp
     main-widget.cpp
     add-contact-dialog.cpp
     remove-contact-dialog.cpp
     fetch-avatar-job.cpp
)


kde4_add_ui_files (contactlist_SRCS
                   main-widget.ui
                   add-contact-dialog.ui
                   remove-contact-dialog.ui
)

add_subdirectory (icons)

kde4_add_executable (telepathy-kde-contactlist
                     ${contactlist_SRCS}
)

target_link_libraries (telepathy-kde-contactlist
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${KDE4_KIO_LIBS}
                       ${KDE4_KCMUTILS_LIBS}
)

# Install:
install (TARGETS telepathy-kde-contactlist
         DESTINATION ${BIN_INSTALL_DIR}
)

install (PROGRAMS telepathy-kde-contactlist.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

project (telepathy-kde-contactlist)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 4.4.75 REQUIRED)
find_package (TelepathyQt4 0.7.1 REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
)



if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules)
execute_process(COMMAND git submodule init
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process(COMMAND git submodule update
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules)


set (contactlist_SRCS
     avatar-button.cpp
     abstract-contact-delegate.cpp
     contact-list-application.cpp
     contact-view-hover-button.cpp
     contact-overlays.cpp
     contact-delegate-overlay.cpp
     contact-delegate.cpp
     contact-delegate-compact.cpp
     account-button.cpp
     account-buttons-panel.cpp
     filter-bar.cpp
     main.cpp
     main-widget.cpp
     fetch-avatar-job.cpp
     global-presence.cpp
     global-presence-chooser.cpp
     dialogs/add-contact-dialog.cpp
     dialogs/join-chat-room-dialog.cpp
     dialogs/remove-contact-dialog.cpp
     dialogs/contact-info.cpp
     common/models/accounts-filter-model.cpp
     common/models/contact-model-item.cpp
     common/models/accounts-model-item.cpp
     common/models/tree-node.cpp
     common/models/accounts-model.cpp
     common/models/groups-model-item.cpp
     common/models/groups-model.cpp
     common/models/proxy-tree-node.cpp
     common/service-availability-checker.cpp
     presence-model.cpp
     kpresence.cpp
)


kde4_add_ui_files (contactlist_SRCS
                   main-widget.ui
                   dialogs/add-contact-dialog.ui
                   dialogs/join-chat-room-dialog.ui
                   dialogs/remove-contact-dialog.ui
                   dialogs/contact-info.ui
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
install(FILES ktelepathy.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)

install (PROGRAMS telepathy-kde-contactlist.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

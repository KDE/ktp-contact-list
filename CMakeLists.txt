project (ktp-contactlist)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

set(IS_KTP_INTERNAL_MODULE TRUE)
set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 4.4.75 REQUIRED)
find_package (TelepathyQt4 0.9.3 REQUIRED)
find_package (KTp REQUIRED)
find_package (TelepathyLoggerQt4 0.5.60 REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
                 ${TELEPATHY_LOGGER_QT4_DEFINITIONS}
)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${TELEPATHY_LOGGER_QT4_INCLUDE_DIRS}
                     ${KTP_INCLUDE_DIR}
)


set (ktp_contactlist_SRCS
     contact-list-widget.cpp
     context-menu.cpp
     abstract-contact-delegate.cpp
     contact-list-application.cpp
     contact-view-hover-button.cpp
     contact-overlays.cpp
     contact-delegate-overlay.cpp
     contact-delegate.cpp
     contact-delegate-compact.cpp
     contacts-model.cpp
     account-button.cpp
     account-buttons-panel.cpp
     filter-bar.cpp
     main.cpp
     main-widget.cpp
     global-presence-chooser.cpp
     dialogs/remove-contact-dialog.cpp
     dialogs/custom-presence-dialog.cpp
     presence-model.cpp
     tooltips/ktooltip.cpp
     tooltips/tooltipmanager.cpp
     tooltips/ktooltipwindow.cpp
     tooltips/contacttooltip.cpp
)


kde4_add_ui_files (ktp_contactlist_SRCS
                   main-widget.ui
                   dialogs/remove-contact-dialog.ui
                   tooltips/contacttooltip.ui
)

kde4_add_executable (ktp-contactlist
                     ${ktp_contactlist_SRCS}
)

target_link_libraries (ktp-contactlist
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KTP_LIBRARIES}
                       ${KTP_MODELS_LIBRARIES}
                       ${KTP_WIDGETS_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${KDE4_KIO_LIBS}
                       ${KDE4_KCMUTILS_LIBS}
                       ${TELEPATHY_LOGGER_QT4_LIBRARIES}
)


# Install:
install (TARGETS ktp-contactlist
         DESTINATION ${BIN_INSTALL_DIR}
)

install (PROGRAMS ktp-contactlist.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

project (telepathy-kde-contactlist)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

OPTION(TP_DEBUG  "Enable Telepathy-Qt4 Debug output" OFF )

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h )

set(KDE_MIN_VERSION "4.4.75")
find_package (KDE4 4.4.75 REQUIRED)
find_package (TelepathyQt4 REQUIRED)
# find_package (KTelepathy REQUIRED)
#find_package (Nepomuk REQUIRED)

# Find telepathy.trig, which is installed already by telepathy-integration-daemon.
# find_file (TELEPATHY_TRIG_SOURCE
#            telepathy.trig
#            PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}" "${CMAKE_INSTALL_PREFIX}/share"
#            ENV XDG_DATA_DIRS
#            PATH_SUFFIXES "ontology/telepathy"
# )

# We depend on telepathy.trig, so fail horribly if it's not found.${CMAKE_MODULE_PATH}
# if (NOT TELEPATHY_TRIG_SOURCE)
#     message (ERROR_FATAL
#              "Could not find telepathy.trig Telepathy source file. You must install telepathy-integration-daemon before trying to build this program."
#     )
# endif (NOT TELEPATHY_TRIG_SOURCE)

include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
#                      ${NEPOMUK_INCLUDES}
#                      ${KTELEPATHY_INCLUDE_DIRS}
#                      "${CMAKE_CURRENT_BINARY_DIR}/ontologies"
)

# add_subdirectory (ontologies)

set (contactlist_SRCS 
     contactviewhoverbutton.cpp 
     contactoverlays.cpp 
     contactdelegateoverlay.cpp
     contactdelegate.cpp
     accountbutton.cpp 
     accounts-model.cpp
     accountfiltermodel.cpp
     contact-model-item.cpp
     tree-node.cpp
     accounts-model-item.cpp
     main.cpp
     main-widget.cpp
)


kde4_add_ui_files (contactlist_SRCS
                   main-widget.ui
)

kde4_add_executable (telepathy-kde-contactlist
                     ${contactlist_SRCS}

)

target_link_libraries (telepathy-kde-contactlist
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
)

# Install:
install (TARGETS telepathy-kde-contactlist
         DESTINATION ${BIN_INSTALL_DIR}
)

project (telepathy-contactlist-prototype)

set (CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
     ${CMAKE_MODULE_PATH}
)

find_package (KDE4 REQUIRED)
find_package (TelepathyQt4 REQUIRED)
find_package(Nepomuk REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${KDE4_DEFINITIONS}
                 -DDISABLE_NEPOMUK_LEGACY)

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${NEPOMUK_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}/nie
)

# Find telepathy.trig, which is installed already by telepathy-integration-daemon.
find_file (TELEPATHY_TRIG_SOURCE
           telepathy.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}" "${CMAKE_INSTALL_PREFIX}/share"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/telepathy"
)

# We depend on telepathy.trig, so fail horribly if it's not found.${CMAKE_MODULE_PATH}
if (NOT TELEPATHY_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find telepathy.trig Telepathy source file. You must install telepathy-integration-daemon before trying to build this program."
    )
endif (NOT TELEPATHY_TRIG_SOURCE)

###################################
# First target - NIE static library

nepomuk_add_ontology_classes (nie_SRCS
                              ONTOLOGIES
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig
                              ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig
                              ${TELEPATHY_TRIG_SOURCE}
)

kde4_add_library (nie
                  STATIC
                  ${nie_SRCS}
)

target_link_libraries (nie
                       ${QT_QTCORE_LIBRARY}
                       ${NEPOMUK_LIBRARIES}
)

################################
# Second target - ContatList App

set (contactlist_SRCS
     main.cpp
     contacts-list-model.cpp
     contact-item.cpp
     main-widget.cpp
     nepomuk-signal-watcher.cpp
     grouped-contacts-proxy-model.cpp
     meta-contact-item.cpp
     abstract-tree-item.cpp
     telepathy-bridge.cpp
     telepathy-account-proxy.cpp
     telepathy-base-job.cpp
     add-contacts-to-group-job.cpp
     remove-contacts-from-group-job.cpp
     add-contact-job.cpp
     remove-contacts-job.cpp
     add-meta-contact-job.cpp
)

kde4_add_ui_files (contactlist_SRCS
                   main-widget.ui
)

# Add the ontologies we want to build the vocabulary for.
soprano_add_ontology (pimo_SRCS
                      ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig
                      "PIMO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (nco_SRCS
                      ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
                      "NCO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (nao_SRCS
                      ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nao.trig
                      "NAO"
                      "Nepomuk::Vocabulary"
                      "trig"
)

soprano_add_ontology (telepathy_SRCS
                      ${TELEPATHY_TRIG_SOURCE}
                      "Telepathy"
                      "Nepomuk::Vocabulary"
                      "trig"
)

kde4_add_executable (telepathy_contactlist_prototype
                     ${contactlist_SRCS}
                     ${contactlist_NEPOMUK_SRCS}
                     ${nco_SRCS}
                     ${nao_SRCS}
                     ${pimo_SRCS}
                     ${telepathy_SRCS}
)

target_link_libraries (telepathy_contactlist_prototype
                       ${QT_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${NEPOMUK_LIBRARIES}
                       ${NEPOMUK_QUERY_LIBRARIES}
                       nie

)

# Install:
install (TARGETS telepathy_contactlist_prototype
         DESTINATION ${BIN_INSTALL_DIR}
)


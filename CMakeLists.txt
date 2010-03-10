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
include (NepomukAddOntologyClasses)

# Find pimo.trig, which is installed already by kdebase.
find_file (PIMO_TRIG_SOURCE
           pimo.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/pimo"
)

# We depend on pimo.trig, so fail horribly if it's not found.
if (NOT PIMO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find pimo.trig PIMO source file. Is kdebase-runtime installed?"
    )
endif (NOT PIMO_TRIG_SOURCE)

# Find nco.trig, which is installed already by kdebase.
find_file (NCO_TRIG_SOURCE
           nco.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/nie"
)

# We depend on nco.trig, so fail horribly if it's not found.
if (NOT NCO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find nco.trig NCO source file. Is kdebase-runtime installed?"
    )
endif (NOT NCO_TRIG_SOURCE)

# Find nfo.trig, which is installed already by kdebase.
find_file (NFO_TRIG_SOURCE
           nfo.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/nie"
)

# We depend on nfo.trig, so fail horribly if it's not found.
if (NOT NFO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find nfo.trig NFO source file. Is kdebase-runtime installed?"
    )
endif (NOT NFO_TRIG_SOURCE)

# Find nie.trig, which is installed already by kdebase.
find_file (NIE_TRIG_SOURCE
           nie.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/nie"
)

# We depend on nie.trig, so fail horribly if it's not found.
if (NOT NIE_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find nie.trig NIE source file. Is kdebase-runtime installed?"
    )
endif (NOT NIE_TRIG_SOURCE)

# Find nmo.trig, which is installed already by kdebase.
find_file (NMO_TRIG_SOURCE
           nmo.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/nie"
)

# We depend on nmo.trig, so fail horribly if it's not found.
if (NOT NMO_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find nmo.trig NMO source file. Is kdebase-runtime installed?"
    )
endif (NOT NMO_TRIG_SOURCE)

# Find telepathy.trig, which is installed already by telepathy-integration-daemon.
find_file (TELEPATHY_TRIG_SOURCE
           telepathy.trig
           PATHS "${DATA_INSTALL_DIR}" "${KDE4_DATA_DIR}"
           ENV XDG_DATA_DIRS
           PATH_SUFFIXES "ontology/telepathy"
)

# We depend on telepathy.trig, so fail horribly if it's not found.${CMAKE_MODULE_PATH}
if (NOT TELEPATHY_TRIG_SOURCE)
    message (ERROR_FATAL
             "Could not find telepathy.trig Telepathy source file. You must install telepathy-integration-daemon before trying to build this program."
    )
endif (NOT TELEPATHY_TRIG_SOURCE)


add_definitions (${KDE4_DEFINITIONS})

include_directories (${KDE4_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${NEPOMUK_INCLUDES}
                     ${CMAKE_CURRENT_BINARY_DIR}/nie
)

###################################
# First target - NIE static library

nepomuk_add_ontology_classes (nie_SRCS
                              ONTOLOGIES
                              ${NIE_TRIG_SOURCE}
                              ${NFO_TRIG_SOURCE}
                              ${NCO_TRIG_SOURCE}
                              ${NMO_TRIG_SOURCE}
                              ${PIMO_TRIG_SOURCE}
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
)

kde4_add_ui_files (contactlist_SRCS
                   main-widget.ui
)

soprano_add_ontology(nco_SRCS ${NCO_TRIG_SOURCE} "NCO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(pimo_SRCS ${PIMO_TRIG_SOURCE} "PIMO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(telepathy_SRCS ${TELEPATHY_TRIG_SOURCE} "Telepathy" "Nepomuk::Vocabulary" "trig")

kde4_add_executable (telepathy_contactlist_prototype
                     ${contactlist_SRCS}
                     ${contactlist_NEPOMUK_SRCS}
                     ${nco_SRCS}
                     ${pimo_SRCS}
                     ${telepathy_SRCS}
)

target_link_libraries (telepathy_contactlist_prototype
                       ${QT_LIBRARIES}
                       ${KDE4_KDEUI_LIBS}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${NEPOMUK_LIBRARIES}
                       nie

)

# Install:
install (TARGETS telepathy_contactlist_prototype
         DESTINATION ${BIN_INSTALL_DIR}
)


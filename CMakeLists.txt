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

add_subdirectory(nie)

set (contactlist_SRCS
     main.cpp
     contacts-list-model.cpp
     contact-item.cpp
     main-widget.cpp
     nepomuk-signal-watcher.cpp
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


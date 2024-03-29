cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")

set(KTP_CONTACT_LIST_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project (ktp-contactlist VERSION ${KTP_CONTACT_LIST_VERSION})

find_package(ECM 1.6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package (Qt5 5.0 REQUIRED COMPONENTS Widgets Xml)
find_package (KF5 5.11 REQUIRED COMPONENTS DBusAddons KIO KCMUtils Notifications WindowSystem NotifyConfig I18n IconThemes XmlGui KCMUtils People)
find_package (TelepathyQt5 0.9.3 REQUIRED)
find_package (KTp REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

set_package_properties(KF5People PROPERTIES DESCRIPTION "KDE Contacts aggregation"
                       TYPE OPTIONAL
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (KF5People_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
endif ()
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

set (ktp_contactlist_SRCS
     contact-list-widget.cpp
     context-menu.cpp
     abstract-contact-delegate.cpp
     contact-view-hover-button.cpp
     contact-overlays.cpp
     contact-delegate-overlay.cpp
     contact-delegate.cpp
     contact-delegate-compact.cpp
     empty-row-filter.cpp
     ktp-contactlist-debug.cpp
     filter-bar.cpp
     main.cpp
     main-widget.cpp
     global-presence-chooser.cpp
     dialogs/remove-contact-dialog.cpp
     dialogs/custom-presence-dialog.cpp
     dialogs/advanced-presence-dialog.cpp
     tooltips/ktooltip.cpp
     tooltips/tooltipmanager.cpp
     tooltips/ktooltipwindow.cpp
     tooltips/contacttooltip.cpp
     tooltips/persontooltip.cpp
)

if(KF5People_FOUND)
    set (ktp_contactlist_LIBS
        KF5::PeopleWidgets
    )
endif()

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

ki18n_wrap_ui(ktp_contactlist_SRCS
    main-widget.ui
    dialogs/remove-contact-dialog.ui
    tooltips/contacttooltip.ui
    tooltips/persontooltip.ui
)

add_executable (ktp-contactlist ${ktp_contactlist_SRCS})

target_link_libraries (ktp-contactlist ${ktp_contactlist_LIBS}
    KF5::KIOWidgets
    KF5::KCMUtils
    KF5::NotifyConfig
    KF5::DBusAddons
    KF5::I18n
    KF5::IconThemes
    KF5::XmlGui
    KF5::KCMUtils
    KF5::Notifications
    KF5::WindowSystem
    KTp::Models
    KTp::Widgets
    KTp::Logger
    KTp::CommonInternals
)

configure_file(org.kde.ktpcontactlist.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktpcontactlist.service)


# Install:
install (TARGETS ktp-contactlist
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install (PROGRAMS org.kde.ktpcontactlist.desktop
         DESTINATION ${KDE_INSTALL_APPDIR}
)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktpcontactlist.service
         DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

ki18n_install(po)

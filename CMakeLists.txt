project (ktp-contactlist)

cmake_minimum_required(VERSION 2.8.12)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "16")
set (KDE_APPLICATIONS_VERSION_MINOR "04")
set (KDE_APPLICATIONS_VERSION_MICRO "0")

set(KTP_CONTACT_LIST_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

set(IS_KTP_INTERNAL_MODULE TRUE)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

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
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install (PROGRAMS org.kde.ktpcontactlist.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktpcontactlist.service
         DESTINATION ${DBUS_SERVICES_INSTALL_DIR}
)

# Try to find the KTelepathy library
# KTELEPATHY_FOUND - system has libkcmtelepathyaccounts
# KTELEPATHY_INCLUDE_DIR - the kcmtelepathyaccounts include directory
# KTELEPATHY_LIBRARIES - Link these to use libkcmtelepathyaccounts

# Copyright (c) 2008, Allen Winter <winter@kde.org>
# Copyright (c) 2010, Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET (KTELEPATHY_FIND_REQUIRED ${KTelepathy_FIND_REQUIRED})
if (KTELEPATHY_INCLUDE_DIRS AND KTELEPATHY_LIBRARIES)
  # Already in cache, be silent
  set(KTELEPATHY_FIND_QUIETLY TRUE)
endif (KTELEPATHY_INCLUDE_DIRS AND KTELEPATHY_LIBRARIES)

find_path(KTELEPATHY_INCLUDE_DIR
  NAMES abstract-tree-item.h
  PATHS ${KDE4_INCLUDE_DIR}
  PATH_SUFFIXES ktelepathy
)

find_path(KTELEPATHY_INCLUDE_DIRS
  NAMES KTelepathy/AbstractTreeItem
  PATHS ${KDE4_INCLUDE_DIR}
  PATH_SUFFIXES KDE
)

set(KTELEPATHY_INCLUDE_DIRS ${KTELEPATHY_INCLUDE_DIRS} ${KTELEPATHY_INCLUDE_DIR})

find_library(KTELEPATHY_LIBRARIES NAMES ktelepathy )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KTELEPATHY DEFAULT_MSG
                                  KTELEPATHY_LIBRARIES
                                  KTELEPATHY_INCLUDE_DIRS)

mark_as_advanced(KTELEPATHY_INCLUDE_DIRS KTELEPATHY_LIBRARIES)

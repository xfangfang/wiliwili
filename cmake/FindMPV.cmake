# Try to find MPV, once done this will define:
#
#  MPV_FOUND - system has MPV
#  MPV_INCLUDES - the MPV include directory
#  MPV_LIBRARIES - the libraries needed to use MPV
#
# Copyright (c) 2015 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_MPV QUIET mpv)
    set(MPV_DEFINITIONS ${PC_MPV_CFLAGS_OTHER})
    set(MPV_VERSION ${PC_MPV_VERSION})

    find_path(MPV_INCLUDE_DIR mpv/client.h HINTS ${PC_MPV_INCLUDEDIR} ${PC_MPV_INCLUDE_DIRS} PATH_SUFFIXES mpv)

    find_library(MPV_LIBRARY NAMES mpv libmpv HINTS ${PC_MPV_LIBDIR} ${PC_MPV_LIBRARY_DIRS})

    set(MPV_LIBRARIES ${MPV_LIBRARY})
    set(MPV_INCLUDE_DIRS ${MPV_INCLUDE_DIR})

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set MPV_FOUND to TRUE if all listed variables are TRUE
    find_package_handle_standard_args(MPV DEFAULT_MSG MPV_LIBRARY MPV_INCLUDE_DIR)

    mark_as_advanced(MPV_INCLUDE_DIR MPV_LIBRARY)
endif()

set(MPV_VERSION ${PC_MPV_VERSION})
if (MPV_VERSION VERSION_LESS 0.2.2)
    # workaround for incorrect version in pkg-config file, notably on OpenBSD
    set(MPV_VERSION 0.23.0)
endif()

if(NOT MPV_INCLUDES OR NOT MPV_LIBRARIES)
    find_path(MPV_INCLUDES
        NAMES client.h
        PATH_SUFFIXES mpv
        HINTS $ENV{MPVDIR}/include
    )

    find_library(MPV_LIBRARIES
        NAMES mpv
        HINTS $ENV{MPVDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPV
    VERSION_VAR MPV_VERSION
    REQUIRED_VARS MPV_LIBRARIES MPV_INCLUDES
)

mark_as_advanced(MPV_INCLUDES MPV_LIBRARIES)
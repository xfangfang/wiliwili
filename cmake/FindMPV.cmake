# - Try to find libmpv
# Once done this will define
#  MPV_FOUND - System has LibMPV
#  MPV_VERSION - LibMPV version
#  MPV_INCLUDE_DIRS - The LibMPV include directories
#  MPV_LIBRARIES - The libraries needed to use LibMPV
#  MPV_DEFINITIONS - Compiler switches required for using LibMPV

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
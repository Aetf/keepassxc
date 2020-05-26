# - Try to find XKBCommon
# Once done, this will define
#
#   XKBCommon_FOUND - System has XKBCommon
#   XKBCommon_INCLUDE_DIRS - The XKBCommon include directories
#   XKBCommon_LIBRARIES - The libraries needed to use XKBCommon
#   XKBCommon_DEFINITIONS - Compiler switches required for using XKBCommon

find_package(PkgConfig)
pkg_check_modules(PC_XKBCommon QUIET xkbcommon)
set(XKBCommon_DEFINITIONS ${PC_XKBCommon_CFLAGS_OTHER})

find_path(XKBCommon_INCLUDE_DIR
        NAMES xkbcommon/xkbcommon.h
        HINTS ${PC_XKBCommon_INCLUDE_DIR} ${PC_XKBCommon_INCLUDE_DIRS}
        )

find_library(XKBCommon_LIBRARY
        NAMES xkbcommon
        HINTS ${PC_XKBCommon_LIBRARY} ${PC_XKBCommon_LIBRARY_DIRS}
        )

set(XKBCommon_LIBRARIES ${XKBCommon_LIBRARY})
set(XKBCommon_LIBRARY_DIRS ${XKBCommon_LIBRARY_DIRS})
set(XKBCommon_INCLUDE_DIRS ${XKBCommon_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XKBCommon DEFAULT_MSG
        XKBCommon_LIBRARY
        XKBCommon_INCLUDE_DIR
        )

mark_as_advanced(XKBCommon_LIBRARY XKBCommon_INCLUDE_DIR)

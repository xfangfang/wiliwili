cmake_minimum_required(VERSION 3.15)

if (_USE_SHARED_LIB)
    find_package(cpr REQUIRED GLOBAL)

    find_package(qrcodegencpp REQUIRED GLOBAL)
    add_library(qrcode ALIAS qrcodegencpp::qrcodegencpp)

    find_path(PYSTRING_INCLUDE_DIR pystring.h PATH_SUFFIXES pystring REQUIRED)
    find_library(PYSTRING_LIBRARY NAMES pystring REQUIRED)
    list(APPEND PLATFORM_INCLUDES ${PYSTRING_INCLUDE_DIR})
    list(APPEND PLATFORM_LIBS ${PYSTRING_LIBRARY})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(opencc REQUIRED IMPORTED_TARGET opencc)
    add_library(libopencc ALIAS PkgConfig::opencc)

    set(PLATFORM_LIBS
        ${PLATFORM_LIBS}
        PARENT_SCOPE
    )
    set(PLATFORM_INCLUDES
        ${PLATFORM_INCLUDES}
        PARENT_SCOPE
    )
else ()
    # add cpr
    set(BUILD_SHARED_LIBS OFF)
    if (PLATFORM_DESKTOP)
        if (APPLE)
            set(CPR_USE_SYSTEM_CURL ON)
            set(CPR_FORCE_DARWINSSL_BACKEND ON)
        endif ()
        if (WIN32)
            set(CPR_USE_SYSTEM_CURL OFF)
            set(CPR_FORCE_WINSSL_BACKEND ON)
        endif ()
    elseif (PLATFORM_IOS)
        set(CPR_USE_SYSTEM_CURL OFF)
        set(CPR_SKIP_CA_BUNDLE_SEARCH ON)
        set(CPR_ENABLE_SSL ON)
    else ()
        set(CPR_USE_SYSTEM_CURL ON)
        set(CPR_FORCE_MBEDTLS_BACKEND ON)
    endif ()
    add_subdirectory(cpr EXCLUDE_FROM_ALL)

    # add libopencc
    if (NOT USE_LIBROMFS)
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        add_subdirectory(OpenCC EXCLUDE_FROM_ALL)
        target_compile_options(libopencc PRIVATE -w)
    endif ()

    # add QR-Code-generator
    add_library(
        qrcode STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp/qrcodegen.cpp
    )
    target_include_directories(
        qrcode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp
    )
    set_property(TARGET qrcode PROPERTY CXX_STANDARD 17)

    # add pystring
    add_library(
        pystring STATIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring/pystring.cpp
    )
    target_include_directories(
        pystring PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring
    )
    set_property(TARGET pystring PROPERTY CXX_STANDARD 17)
endif ()

# add lunasvg
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(lunasvg EXCLUDE_FROM_ALL)

# add mongoose
add_subdirectory(mongoose EXCLUDE_FROM_ALL)

# add borealis
if (USE_LIBROMFS)
    set(BRLS_USE_OPENCC OFF)
else ()
    set(BRLS_USE_OPENCC ON)
    if (_USE_SHARED_LIB)
        set(BRLS_OPENCC_INCLUDE ${opencc_INCLUDE_DIRS})
    else ()
        set(BRLS_OPENCC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/OpenCC/src)
    endif ()
endif ()
add_subdirectory(borealis/library)

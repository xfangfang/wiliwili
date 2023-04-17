cmake_minimum_required(VERSION 3.15)

# add cpr
if (PLATFORM_DESKTOP)
    if (APPLE)
        set(CPR_USE_SYSTEM_CURL ON)
        set(CPR_FORCE_DARWINSSL_BACKEND ON)
    endif ()
    if (WIN32)
        set(CPR_USE_SYSTEM_CURL OFF)
        set(CPR_FORCE_WINSSL_BACKEND ON)
    endif ()
else ()
    set(CPR_USE_SYSTEM_CURL ON)
    set(CPR_FORCE_MBEDTLS_BACKEND ON)
endif ()
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(cpr EXCLUDE_FROM_ALL)

# add lunasvg
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(lunasvg EXCLUDE_FROM_ALL)

# add libopencc
add_subdirectory(OpenCC EXCLUDE_FROM_ALL)
target_compile_options(libopencc PRIVATE -w)

# add QR-Code-generator
add_library(qrcode STATIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp/qrcodegen.cpp)
target_include_directories(qrcode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp)
set_property(TARGET qrcode PROPERTY CXX_STANDARD 17)

# add pystring
add_library(pystring STATIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring/pystring.cpp)
target_include_directories(pystring PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring)
set_property(TARGET pystring PROPERTY CXX_STANDARD 17)

# add borealis
set(BRLS_USE_OPENCC ON)
set(BRLS_OPENCC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/OpenCC/src)
add_subdirectory(borealis/library)
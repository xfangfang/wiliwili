cmake_minimum_required(VERSION 3.15)

# import cpr
set(CPR_USE_SYSTEM_CURL ON)
if (PLATFORM_DESKTOP)
    if (APPLE)
        set(CPR_FORCE_DARWINSSL_BACKEND ON)
    endif ()
    if (WIN32)
        set(CPR_USE_SYSTEM_CURL OFF)
        set(CPR_FORCE_WINSSL_BACKEND ON)
    endif ()
else ()
    set(CPR_FORCE_MBEDTLS_BACKEND ON)
endif ()

set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 142a496e49bc71dc1be85a3ce47bb25573670000) # 2022/11/04
FetchContent_MakeAvailable(cpr)

#import lunasvg
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(lunasvg
        GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
        GIT_TAG 7417baa0aff477f361e44e2aa793fdb0c7aae352) # 2.3.2
FetchContent_MakeAvailable(lunasvg)

##import libopencc
add_subdirectory(OpenCC)

# add QR-Code-generator
add_library(qrcode STATIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp/QrCode.cpp)
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
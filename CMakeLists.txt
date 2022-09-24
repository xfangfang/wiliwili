cmake_minimum_required(VERSION 3.15)

# build options
set(PLATFORM_DESKTOP OFF CACHE BOOL "build for desktop, ON for desktop; OFF for switch")
set(BUILTIN_NSP OFF CACHE BOOL "Built in NSP forwarder")
set(WIN32_TERMINAL ON CACHE BOOL "Show terminal when run on Windows")

IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message("Build Type: ${CMAKE_BUILD_TYPE}")
ENDIF ()


# toolchain
if (PLATFORM_DESKTOP)
    message("building for Desktop")
    set(TARGET_PLATFORM desktop CACHE STRING "")
    message("Build Type: ${CMAKE_BUILD_TYPE}")
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g2 -ggdb -Wall")
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
else ()
    message("building for SWITCH")
    message("Build Type: ${CMAKE_BUILD_TYPE}")
    set(CMAKE_USE_SYSTEM_ENVIRONMENT_PATH OFF)
    file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)
    if (NOT IS_DIRECTORY ${DEVKITPRO})
        message(FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
    endif ()
    set(DEVKITPRO $ENV{DEVKITPRO} CACHE BOOL "DEVKITPRO")
    include(${DEVKITPRO}/cmake/Switch.cmake REQUIRED)
    set(__SWITCH__ ON)
    set(CMAKE_C_FLAGS "-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include")
    set(CMAKE_CXX_FLAGS "-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include")
    message(STATUS "PKG_CONFIG_EXECUTABLE = ${PKG_CONFIG_EXECUTABLE}")
    message(STATUS "CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
    message(STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
    message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
    message(STATUS "NX_NACP" = ${NX_NACPTOOL_EXE})
    message(STATUS "NX_ELF2NRO" = ${NX_ELF2NRO_EXE})
endif ()


# project info
project(wiliwili)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "4")
set(VERSION_REVISION "1")
set(PROJECT_TITLEID "0x010FF000FFFF0001")
set(PROJECT_AUTHOR "xfangfang")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/bilibili.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(FetchContent)

# build borealis qrcode and other third party libraries
add_subdirectory(library)


# setting src and include
file(GLOB_RECURSE MAIN_SRC wiliwili/source/*.cpp)

set(PLATFORM_OPTION)
if (PLATFORM_DESKTOP)
    find_package(MPV REQUIRED)
    if (MPV_FOUND)
        message(STATUS "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIRS} ${MPV_LIBRARIES}")
    endif ()
    set(PLATFORM_LIBS ${MPV_LIBRARIES})
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(PLATFORM_OPTION -pthread)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

else ()
    set(PLATFORM_LIBS
            # needed by borealis
            glfw3 EGL glapi drm_nouveau
            # needed by mpv
            mpv swscale swresample avformat avfilter avcodec avutil ass freetype fribidi png bz2 z SDL2
            mbedx509 mbedtls mbedcrypto mbedx509 mbedtls
            # needed by cpr
            curl z
            # base lib
            nx m
            )
    if (BUILTIN_NSP)
        list(APPEND PLATFORM_LIBS nsp)
        set(PLATFORM_OPTION -DBUILTIN_NSP)
    endif ()
    list(APPEND MAIN_SRC ${CMAKE_SOURCE_DIR}/library/borealis/library/lib/platforms/switch/switch_wrapper.c)
endif ()


# building target
if (WIN32)
    if (WIN32_TERMINAL)
        add_executable(${PROJECT_NAME} ${MAIN_SRC} app_win32.rc)
    else ()
        add_executable(${PROJECT_NAME} WIN32 ${MAIN_SRC} app_win32.rc)
    endif ()
else ()
    add_executable(${PROJECT_NAME} ${MAIN_SRC})
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE
        wiliwili/include
        wiliwili/include/api)
target_compile_options(${PROJECT_NAME} PRIVATE
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        -D__VERSION_REVISION__=${VERSION_REVISION}
        -ffunction-sections
        -fdata-sections
        ${PLATFORM_OPTION}
        )
target_link_libraries(${PROJECT_NAME} PRIVATE
        cpr::cpr
        qrcode
        borealis
        lunasvg
        pystring
        ${PLATFORM_LIBS}
        )


# building release file
if (PLATFORM_DESKTOP)
    add_custom_target(${PROJECT_NAME}.data
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
            )
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
else ()
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            LINK_FLAGS_RELEASE -s
            LINK_FLAGS "-Wl,-no-as-needed -Wl,-Map,.map"
    )
    add_custom_target(${PROJECT_NAME}.nro
            DEPENDS ${PROJECT_NAME}
            COMMAND ${NX_NACPTOOL_EXE} --create "${PROJECT_NAME}"
            "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}"
            ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/resources/icon/wiliwili.ico
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/resources/test.flv
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/resources/Illegal-Font.ttf
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/resources/Wingdings.ttf
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/inter
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/font
            COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON}
            --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_BINARY_DIR}/resources
            )
endif ()

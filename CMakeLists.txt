cmake_minimum_required(VERSION 3.15)

# build options
set(PLATFORM_DESKTOP OFF CACHE BOOL "build for desktop, ON for desktop; OFF for switch")
set(BUILTIN_NSP OFF CACHE BOOL "Built in NSP forwarder")
set(WIN32_TERMINAL ON CACHE BOOL "Show terminal when run on Windows")
set(VERIFY_SSL ON CACHE BOOL "Whether to verify ssl")

# analytics
set(ANALYTICS OFF CACHE BOOL "Using Google Analytics")
set(ANALYTICS_ID "" CACHE STRING "Google Analytics ID")
set(ANALYTICS_KEY "" CACHE STRING "Google Analytics key")

# Strongly depends on the environment, these options may lead to build errors
set(MAC_10_15 OFF CACHE BOOL "Set the Minimum OS X deployment version to 10.15")
set(MAC_UNIVERSAL OFF CACHE BOOL "Support x86_64;arm64 at the same time")

option(USE_GLFW "using glfw for input and create window" ON)
option(USE_SDL2 "using sdl2 for input and create window" OFF)

option(INSTALL "Install to system. only use this option with Linux." OFF)

if (USE_SDL2)
    message("SDL2")
    set(USE_GLFW OFF)
else ()
    # default glfw3.3
    message("GLFW")
    set(USE_GLFW ON)
    set(USE_SDL2 OFF)
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra.cmake)

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
    message("DEVKITPRO: $ENV{DEVKITPRO}")
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
set(VERSION_MINOR "5")
set(VERSION_REVISION "0")
set(PROJECT_TITLEID "0x010FF000FFFF0001")
set(PROJECT_AUTHOR "xfangfang")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/bilibili.jpg)
set(PROJECT_ICON_PNG ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/bilibili.png)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(PACKAGE_NAME cn.xfangfang.wiliwili)
set(PROJECT_DESKTOP ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.desktop)
set(PROJECT_ICON_SVG ${CMAKE_CURRENT_SOURCE_DIR}/resources/svg/${PACKAGE_NAME}.svg)
set(RESOURCES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/wiliwili) # Only used in Linux
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
add_definitions(
        -DBUILD_VERSION_MAJOR=${VERSION_MAJOR}
        -DBUILD_VERSION_MINOR=${VERSION_MINOR}
        -DBUILD_VERSION_REVISION=${VERSION_REVISION})
if (USE_SDL2)
    add_definitions(-D__SDL2__)
else ()
    add_definitions(-D__GLFW__)
endif ()

# disable in-source build
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed")
endif ()

# set resources dir
if (INSTALL)
    set(BRLS_RESOURCES_DIR ${RESOURCES_INSTALL_DIR})
else ()
    set(BRLS_RESOURCES_DIR ".")
endif ()

# build borealis qrcode and other third party libraries
add_subdirectory(library)


# setting src and include
file(GLOB_RECURSE MAIN_SRC wiliwili/source/*.cpp)

set(PLATFORM_OPTION)
if (PLATFORM_DESKTOP)
    find_package(MPV REQUIRED)
    if (MPV_FOUND)
        message(STATUS "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIR} ${MPV_LIBRARY}")
    endif ()
    set(PLATFORM_LIBS ${MPV_LIBRARY})
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(PLATFORM_OPTION -pthread -I${MPV_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
else ()
    if (USE_SDL2)
        set(PLATFORM_LIBS SDL2)
    else ()
        set(PLATFORM_LIBS glfw3)
    endif ()
    list(APPEND PLATFORM_LIBS
            # needed by borealis
            EGL glapi drm_nouveau
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

# set google analytics
if (ANALYTICS)
    if (NOT ANALYTICS_ID OR NOT ANALYTICS_KEY)
        message(FATAL_ERROR "ANALYTICS set to ON, You need set ANALYTICS_ID and ANALYTICS_KEY too")
    endif ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -DANALYTICS=${ANALYTICS}
            -DANALYTICS_ID=${ANALYTICS_ID}
            -DANALYTICS_KEY=${ANALYTICS_KEY})
endif ()

if (VERIFY_SSL)
    target_compile_options(${PROJECT_NAME} PRIVATE -DVERIFY_SSL)
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
        -Wunused-variable
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

    if (APPLE)
        if (MAC_10_15)
            message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET: 10.15")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
        endif ()
        if (MAC_UNIVERSAL)
            message(STATUS "CMAKE_OSX_ARCHITECTURES: x86_64;arm64")
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
            set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm 64 minimum deployment target" FORCE)
        endif ()
        add_custom_target(${PROJECT_NAME}.app
                COMMAND "bash" "${CMAKE_BINARY_DIR}/../scripts/build_mac.sh"
                )
        add_dependencies(${PROJECT_NAME}.app ${PROJECT_NAME})
    elseif (UNIX)
        if (INSTALL)
            message("install prefix: ${CMAKE_INSTALL_PREFIX}")
            install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
            install(DIRECTORY ${PROJECT_RESOURCES}
                    DESTINATION ${RESOURCES_INSTALL_DIR}
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
                    )

            install(FILES ${PROJECT_ICON_SVG}
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps
                    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
                    )

            set(icon_dirs 16x16 22x22 24x24 32x32 48x48 64x64 128x128 256x256)
            foreach (icon_dir ${icon_dirs})
                install(FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/icon/${PACKAGE_NAME}.png
                        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${icon_dir}/apps
                        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
                        )
            endforeach ()

            install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.appdata.xml
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo
                    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
                    )

            install(FILES ${PROJECT_DESKTOP}
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
                    )

            add_custom_target(${PROJECT_NAME}.install
                    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/resources/icon/bilibili.png
                    ${CMAKE_CURRENT_BINARY_DIR}/resources/icon/${PACKAGE_NAME}.png
                    )
            add_dependencies(${PROJECT_NAME}.install ${PROJECT_NAME}.data)
            add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.install)
        endif ()
    endif ()
else ()
    add_custom_target(${PROJECT_NAME}.nro
            DEPENDS ${PROJECT_NAME}
            COMMAND ${NX_NACPTOOL_EXE} --create "${PROJECT_NAME}"
            "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}"
            ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/resources/icon/wiliwili.ico
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/font
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/gamepad
            COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON}
            --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_BINARY_DIR}/resources
            )
endif ()

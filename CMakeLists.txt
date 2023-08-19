cmake_minimum_required(VERSION 3.13)

set(BOREALIS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/library/borealis/library)

# Include Platform common options
# wiliwili only support PLATFORM_DESKTOP and PLATFORM_SWITCH
# example: cmake -B build -DPLATFORM_DESKTOP=ON
include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)

# NintendoSwitch
cmake_dependent_option(BUILTIN_NSP "Built in NSP forwarder" OFF "PLATFORM_SWITCH" OFF)

# macOS
# These four options are only for macOS app bundling usage.
# Do not use it until you know what you are doing.
cmake_dependent_option(MAC_IntelChip "Build for x86_64 (10.11)" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_AppleSilicon "Build for arm64 (11.0)" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_Universal "Support x86_64;arm64 at the same time" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_DOWNLOAD_DYLIB "Using precompiled dylib instead of system provided" OFF "APPLE;NOT PLATFORM_IOS" OFF)

# Linux
# Using CMAKE_INSTALL_PREFIX specifies the location where the application is installed.
cmake_dependent_option(INSTALL "Install to system path" OFF "UNIX;NOT APPLE" OFF)

# Program related
option(VERIFY_SSL "Whether to verify ssl in http request" ON)
if (VERIFY_SSL)
    list(APPEND APP_PLATFORM_OPTION -DVERIFY_SSL)
endif ()

# For Developer
option(DEBUG_SANITIZER "Turn on sanitizers (only available in debug build)" OFF)

# Google Analytics
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoogleAnalytics.cmake)

# Dependencies
option(USE_SHARED_LIB "Whether to use shared libs provided by system" OFF)
cmake_dependent_option(USE_SYSTEM_CURL "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_CPR "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_QRCODEGEN "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_PYSTRING "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_OPENCC "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_FMT "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_TINYXML2 "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_SDL2 "Only for PSV" OFF "PLATFORM_PSV" OFF)

option(DISABLE_OPENCC "Disable Chinese Simplified and Chinese Traditional Conversion " OFF)

# mpv related
# If your system does not support OpenGL(ES), you can use software rendering, but it will affect performance.
option(MPV_SW_RENDER "Using CPU to draw videos" OFF)
if (MPV_SW_RENDER)
    list(APPEND APP_PLATFORM_OPTION -DMPV_SW_RENDER)
endif ()

# On systems that do not support framebuffer, let MPV to draw to full screen and
# then cover unnecessary areas with UI.
option(MPV_NO_FB "Using system provided framebuffer" OFF)
if (MPV_NO_FB)
    list(APPEND APP_PLATFORM_OPTION -DMPV_NO_FB)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra.cmake)

# toolchain
include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)

# Git Info
git_info(GIT_TAG_VERSION GIT_TAG_SHORT)
list(APPEND APP_PLATFORM_OPTION -DBUILD_TAG_VERSION=${GIT_TAG_VERSION} -DBUILD_TAG_SHORT=${GIT_TAG_SHORT})

# Project Info
project(wiliwili)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")
set(VERSION_REVISION "1")
set(VERSION_BUILD "0")
set(PROJECT_AUTHOR "xfangfang")
set(PACKAGE_NAME cn.xfangfang.wiliwili)
set(VITA_TITLEID "WILIWILI0")
set(VITA_VERSION "01.11")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(Threads REQUIRED)
list(APPEND APP_PLATFORM_LIB ${CMAKE_THREAD_LIBS_INIT})

# Find libmpv and libwebp
if (PLATFORM_DESKTOP)
    find_package(MPV REQUIRED)
    message(STATUS "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIR} ${MPV_LIBRARY}")
    list(APPEND APP_PLATFORM_INCLUDE ${MPV_INCLUDE_DIR})
    list(APPEND APP_PLATFORM_LIB ${MPV_LIBRARY})

    find_package(WebP)
    if (WebP_FOUND)
        message(STATUS "Found webp: ${WebP_INCLUDE_DIRS} ${WebP_LIBRARY}")
        list(APPEND APP_PLATFORM_OPTION -DUSE_WEBP)
        list(APPEND APP_PLATFORM_INCLUDE ${WebP_INCLUDE_DIRS})
        list(APPEND APP_PLATFORM_LIB ${WebP_LIBRARY})
    endif ()
elseif (APP_PLATFORM_CUSTOM_LIBS)
    message(STATUS "Using custom libraries for mpv and webp")
    # If you want to manually specify dependencies:
    # cmake -B build -DPLATFORM_PSV \
    #   -DAPP_PLATFORM_CUSTOM_LIBS=ON \
    #   -DAPP_PLATFORM_INCLUDE="/opt/vita/include" \
    #   -DAPP_PLATFORM_LINK_OPTION="-L/opt/vita/lib -lmpv -lwebp"
else ()
    # Used for Cross Compile such as NintendoSwitch
    # If dependent files cannot be found, environment variables can be set before executing the cmake command:
    # PKG_CONFIG_LIBDIR=/some/path/arm64/lib/pkgconfig
    find_package(PkgConfig REQUIRED)

    pkg_search_module(MPV REQUIRED mpv)
    message(STATUS "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIRS} ${MPV_STATIC_LIBRARIES}")
    list(APPEND APP_PLATFORM_INCLUDE ${MPV_INCLUDE_DIRS})
    list(APPEND APP_PLATFORM_LIB ${MPV_STATIC_LIBRARIES})
    link_directories(${MPV_LIBRARY_DIRS})
    if (PLATFORM_IOS)
        list(APPEND APP_PLATFORM_LIB "-framework CoreMedia -framework CoreText -framework VideoToolbox")
    endif ()

    pkg_search_module(LIBWEBP QUIET libwebp)
    if (LIBWEBP_FOUND)
        message(STATUS "Found libwebp: ${LIBWEBP_INCLUDE_DIRS} ${LIBWEBP_LIBRARIES}")
        list(APPEND APP_PLATFORM_INCLUDE ${LIBWEBP_INCLUDE_DIRS})
        list(APPEND APP_PLATFORM_LIB ${LIBWEBP_LIBRARIES})
        list(APPEND APP_PLATFORM_OPTION -DUSE_WEBP)
        link_directories(${LIBWEBP_LIBRARY_DIRS})
    endif ()
endif ()

list(APPEND APP_PLATFORM_OPTION
   -DBUILD_PACKAGE_NAME=${PACKAGE_NAME}
   -DBUILD_VERSION_MAJOR=${VERSION_MAJOR}
   -DBUILD_VERSION_MINOR=${VERSION_MINOR}
   -DBUILD_VERSION_REVISION=${VERSION_REVISION}
)

if (BUILTIN_NSP)
    list(APPEND APP_PLATFORM_LIB nsp)
    list(APPEND APP_PLATFORM_OPTION -DBUILTIN_NSP)
    add_subdirectory(scripts/switch-forwarder)
endif ()

if (USE_LIBROMFS)
    add_libromfs(${PROJECT_NAME} ${PROJECT_RESOURCES})
endif ()

# set resources dir
if (CUSTOM_RESOURCES_DIR)
    set(BRLS_RESOURCES_DIR ${CUSTOM_RESOURCES_DIR})
elseif (INSTALL)
    set(BRLS_RESOURCES_DIR ${CMAKE_INSTALL_PREFIX}/share/wiliwili)
else ()
    set(BRLS_RESOURCES_DIR ".")
endif ()

# source cpp
file(GLOB_RECURSE MAIN_SRC wiliwili/source/*.cpp)
if (WIN32)
    configure_file("${CMAKE_SOURCE_DIR}/wiliwili/app_win32.rc.in" "${CMAKE_BINARY_DIR}/app_win32.rc")
    list(APPEND MAIN_SRC ${CMAKE_BINARY_DIR}/app_win32.rc)
elseif (PLATFORM_SWITCH)
    # System service initialization functions required for libnx
    list(APPEND MAIN_SRC ${BOREALIS_LIBRARY}/lib/platforms/switch/switch_wrapper.c)
endif ()

# build borealis qrcode and other third party libraries
add_subdirectory(library)

# Main Target
program_target(${PROJECT_NAME} "${MAIN_SRC}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# building release file
if (PLATFORM_DESKTOP)
    # Copy resources to build dir
    if (NOT USE_LIBROMFS)
        add_custom_target(${PROJECT_NAME}.data
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
            COMMAND "echo" "copy resources"
        )
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
    endif ()

    if (APPLE)
        if (MAC_DOWNLOAD_DYLIB)
            # Download macos dylib dependencies if require
            include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos.cmake)
            set(bundle_args1 "-nb")
            set(bundle_args2 "${MACOS_DYLIB_DIR}")
        endif ()
        add_custom_target(
            ${PROJECT_NAME}.app
            COMMAND "bash" "${CMAKE_BINARY_DIR}/../scripts/build_mac.sh" "${bundle_args1}" "${bundle_args2}"
        )
        add_dependencies(${PROJECT_NAME}.app ${PROJECT_NAME})
    elseif (UNIX AND INSTALL)
        set(icon_dirs 16x16 24x24 32x32 48x48 64x64 128x128 256x256)
        linux_install(
                "${PROJECT_RESOURCES}" "${BRLS_RESOURCES_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/resources/svg/${PACKAGE_NAME}.svg"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/icons" "${icon_dirs}"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.appdata.xml"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.desktop")
    endif ()
elseif (PLATFORM_IOS)
    ios_bundle(
            "${CMAKE_CURRENT_SOURCE_DIR}/library/borealis/demo/ios/tvos/Splash.storyboard"
            "${CMAKE_CURRENT_SOURCE_DIR}/library/borealis/demo/ios/iphoneos/Splash.storyboard"
            "${CMAKE_CURRENT_SOURCE_DIR}/library/borealis/demo/ios/Images.xcassets"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/ios/iOSBundleInfo.plist.in"
            "wiliwili"
            "${APP_VERSION}")
elseif (PLATFORM_PSV)
    set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12") # max heap size mode
    # I don't know why, but only manually specifying the link libraries in this order will not cause the program to crash
    set(APP_PLATFORM_LIB
            stdc++;mpv;
            avfilter;swscale;avformat;avcodec;swresample;avutil;
            ass;harfbuzz;fribidi;freetype;bz2;png16;
            webp;sharpyuv;
            m;z;
            SceCodecEngine_stub
            SceVideodec_stub
            SceAudiodec_stub)
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE NOASLR)
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
            VERSION ${VITA_VERSION}
            NAME ${PROJECT_NAME}
            FILE ${CMAKE_SOURCE_DIR}/resources resources
            FILE ${CMAKE_SOURCE_DIR}/scripts/psv/sce_sys sce_sys
            FILE ${CMAKE_SOURCE_DIR}/scripts/psv/module/ module
            )
elseif (PLATFORM_SWITCH)
    set(BUILD_FONT_DIR ${CMAKE_BINARY_DIR}/resources/font)
    if (GIT_TAG_VERSION)
        string(SUBSTRING ${GIT_TAG_VERSION} 1 -1 APP_VERSION)
    endif ()
    if (BUILTIN_NSP)
        add_dependencies(${PROJECT_NAME} nsp_forwarder.nsp)
    endif ()
    add_custom_target(${PROJECT_NAME}.nro
        DEPENDS ${PROJECT_NAME}
        COMMAND ${NX_NACPTOOL_EXE} --create ${PROJECT_NAME} ${PROJECT_AUTHOR} ${APP_VERSION} ${PROJECT_NAME}.nacp
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E remove -f ${BUILD_FONT_DIR}/*.txt ${BUILD_FONT_DIR}/switch_font.ttf
            ${BUILD_FONT_DIR}/keymap*.ttf
        COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON}
            --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_BINARY_DIR}/resources
        ALL
    )
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE wiliwili/include wiliwili/include/api ${APP_PLATFORM_INCLUDE})
target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections -Wunused-variable ${APP_PLATFORM_OPTION})
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr qrcode borealis lunasvg pystring pdr mongoose z ${APP_PLATFORM_LIB})
target_link_options(${PROJECT_NAME} PRIVATE ${APP_PLATFORM_LINK_OPTION})
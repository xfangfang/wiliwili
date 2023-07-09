cmake_minimum_required(VERSION 3.10)

# build options
set(PLATFORM_DESKTOP
    OFF
    CACHE BOOL "build for desktop, ON for desktop; OFF for switch"
)
set(PLATFORM_IOS
    OFF
    CACHE BOOL "build for iOS"
)
set(BUILTIN_NSP
    OFF
    CACHE BOOL "Built in NSP forwarder (only for NintendoSwitch)"
)
set(WIN32_TERMINAL
    ON
    CACHE BOOL "Show terminal when run on Windows"
)
set(VERIFY_SSL
    ON
    CACHE BOOL "Whether to verify ssl"
)
set(USE_SYSTEM_CURL
    OFF
    CACHE BOOL "Whether to use the curl in your system libs"
)
set(USE_BOOST_FILESYSTEM
    OFF
    CACHE BOOL "Whether to use the boost::filesystem instead of std::filesystem"
)
set(USE_SHARED_LIB
    OFF
    CACHE BOOL "Whether to use shared libs provided by system"
)
set(DEBUG_SANITIZER
    OFF
    CACHE BOOL "Turn on sanitizers (only available in debug build)"
)

# Do not use it until you know what you are doing.
set(CUSTOM_RESOURCES_DIR
    ""
    CACHE STRING "Custom resources dir"
)

# analytics
set(ANALYTICS
    OFF
    CACHE BOOL "Using Google Analytics"
)
set(ANALYTICS_ID
    ""
    CACHE STRING "Google Analytics ID"
)
set(ANALYTICS_KEY
    ""
    CACHE STRING "Google Analytics key"
)

# These four options are only for macOS app bundling usage.
# Do not use it until you know what you are doing.
set(MAC_IntelChip
    OFF
    CACHE BOOL "Set the Minimum OS X deployment version to 10.11 (x86_64)"
)
set(MAC_AppleSilicon
    OFF
    CACHE BOOL "Set the Minimum OS X deployment version to 11.0 (aarch64)"
)
set(MAC_Universal
    OFF
    CACHE BOOL "Support x86_64;arm64 at the same time"
)
set(MAC_DOWNLOAD_DYLIB
    OFF
    CACHE BOOL "Using precompiled dylib instead of system provided"
)

# mpv related

# If your system does not support Opengl(es), you can use software rendering before modifying MPV related content,
# but it will affect performance.
set(MPV_SW_RENDER
    OFF
    CACHE BOOL "Using CPU to draw videos"
)
# On systems that do not support framebuffer, let MPV to draw to full screen and
# then cover unnecessary areas with UI (only for OpenGL).
set(MPV_NO_FB
    OFF
    CACHE BOOL "Not using extra framebuffer"
)

option(USE_GLFW "using glfw for input and create window" ON)
option(USE_SDL2 "using sdl2 for input and create window" OFF)
option(USE_GL2 "using OpenGL 2.1" OFF)
option(USE_GLES2 "using OpenGL ES 2.0" OFF)
option(USE_GLES3 "using OpenGL ES 3.0" OFF)
option(INSTALL "Install to system. only use this option with Linux." OFF)
option(USE_LIBROMFS "using libromfs to bundle resources" OFF)

# =======================================================================

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra.cmake)

# toolchain
if (PLATFORM_DESKTOP)
    message(STATUS "building for Desktop")
    set(TARGET_PLATFORM
        desktop
        CACHE STRING ""
    )
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g2 -ggdb -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
    if (USE_SHARED_LIB)
        message("Using system shared libraries whenever possible.")
        set(_USE_SHARED_LIB ON)
        set(USE_SYSTEM_CURL ON)
    endif ()
elseif (PLATFORM_IOS)
    message(STATUS "building for iOS")
    set(USE_SDL2 ON)
    set(USE_GLES3 ON)
    set(MPV_NO_FB ON)
    set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2") # iphone, ipad
    set(USE_LIBROMFS ON) # package resources inside
    set(LIBROMFS_PREBUILT_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/libromfs-generator" CACHE STRING "")
    if (NOT EXISTS "${LIBROMFS_PREBUILT_GENERATOR}")
        message(FATAL_ERROR "libromfs-generator has not been built, please refer to library/borealis/build_libromfs_generator.sh for more information")
    endif()
else ()
    message(STATUS "building for SWITCH")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "DEVKITPRO: $ENV{DEVKITPRO}")
    set(CMAKE_USE_SYSTEM_ENVIRONMENT_PATH OFF)
    file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)
    if (NOT IS_DIRECTORY ${DEVKITPRO})
        message(
            FATAL_ERROR
                "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro"
        )
    endif ()
    set(DEVKITPRO
        $ENV{DEVKITPRO}
        CACHE BOOL "DEVKITPRO"
    )
    include(${DEVKITPRO}/cmake/Switch.cmake REQUIRED)
    set(__SWITCH__ ON)
    set(CMAKE_C_FLAGS
        "-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include"
    )
    set(CMAKE_CXX_FLAGS
        "-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include"
    )
    message(STATUS "PKG_CONFIG_EXECUTABLE = ${PKG_CONFIG_EXECUTABLE}")
    message(STATUS "CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
    message(STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
    message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
    message(STATUS "NX_NACP" = ${NX_NACPTOOL_EXE})
    message(STATUS "NX_ELF2NRO" = ${NX_ELF2NRO_EXE})
endif ()

if (USE_GL2)
    message(STATUS "USE_GL2")
    set(USE_GL2 ON)
    add_definitions(-DUSE_GL2)
    set(MPV_NO_FB ON)
elseif (USE_GLES2)
    message(STATUS "USE_GLES2")
    set(USE_GLES2 ON)
    add_definitions(-DUSE_GLES2)
    set(MPV_NO_FB ON)
elseif (USE_GLES3)
    message(STATUS "USE_GLES3")
    set(USE_GLES3 ON)
    add_definitions(-DUSE_GLES3)
else ()
    message(STATUS "USE_GL3/4")
endif ()

if (USE_SDL2)
    message(STATUS "SDL2")
    set(USE_GLFW OFF)
else ()
    message(STATUS "GLFW")
    set(USE_GLFW ON)
endif ()

# project info
project(wiliwili)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_REVISION "1")
set(PROJECT_TITLEID "0x010FF000FFFF0001")
set(PROJECT_AUTHOR "xfangfang")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(PACKAGE_NAME cn.xfangfang.wiliwili)
set(PROJECT_DESKTOP
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.desktop
)
set(PROJECT_ICON_SVG
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/svg/${PACKAGE_NAME}.svg
)
set(RESOURCES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/wiliwili) # Only used in
                                                                  # Linux
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(PLATFORM_INCLUDES)
set(PLATFORM_OPTION
    -DBUILD_PACKAGE_NAME=${PACKAGE_NAME} -DBUILD_VERSION_MAJOR=${VERSION_MAJOR}
    -DBUILD_VERSION_MINOR=${VERSION_MINOR}
    -DBUILD_VERSION_REVISION=${VERSION_REVISION}
)
if (USE_SDL2)
    add_definitions(-D__SDL2__)
else ()
    add_definitions(-D__GLFW__)
endif ()
if (MPV_SW_RENDER)
    list(APPEND PLATFORM_OPTION -DMPV_SW_RENDER)
endif ()
if (MPV_NO_FB)
    list(APPEND PLATFORM_OPTION -DMPV_NO_FB)
endif()
if (USE_LIBROMFS)
    message(STATUS "USE LIBROMFS")
    set(USE_LIBROMFS ON)
    add_definitions(-DUSE_LIBROMFS)
    set(LIBROMFS_PROJECT_NAME ${PROJECT_NAME})
    set(LIBROMFS_RESOURCE_LOCATION "${PROJECT_RESOURCES}")
endif ()

# disable in-source build
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed")
endif ()

# set resources dir
if (CUSTOM_RESOURCES_DIR)
    set(BRLS_RESOURCES_DIR ${CUSTOM_RESOURCES_DIR})
elseif (INSTALL)
    set(BRLS_RESOURCES_DIR ${RESOURCES_INSTALL_DIR})
else ()
    set(BRLS_RESOURCES_DIR ".")
endif ()

if (USE_SYSTEM_CURL)
    set(CPR_USE_SYSTEM_CURL ON)
else ()
    set(CPR_USE_SYSTEM_CURL OFF)
endif ()

# setting src and include
file(GLOB_RECURSE MAIN_SRC wiliwili/source/*.cpp)

if (PLATFORM_DESKTOP)
    find_package(MPV REQUIRED)
    if (MPV_FOUND)
        message(
            STATUS
                "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIR} ${MPV_LIBRARY}"
        )
    endif ()
    set(PLATFORM_LIBS ${MPV_LIBRARY})
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    list(APPEND PLATFORM_OPTION -pthread -I${MPV_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif (PLATFORM_IOS)
    if (NOT DEFINED IOS_DEPS_DIR)
        message(FATAL_ERROR "You should add -DIOS_DEPS_DIR=/some/path which contain include and lib for ios")
    endif ()
    set(IOS_INCLUDE_DIR "${IOS_DEPS_DIR}/include")
    set(IOS_LIB_DIR "${IOS_DEPS_DIR}/lib")
    list(APPEND PLATFORM_INCLUDES "${IOS_INCLUDE_DIR}")
    list(APPEND PLATFORM_OPTION -DUSE_WEBP)
    set(PLATFORM_LIBS
            ${IOS_LIB_DIR}/libmpv.a
            ${IOS_LIB_DIR}/libavcodec.a
            ${IOS_LIB_DIR}/libavdevice.a
            ${IOS_LIB_DIR}/libavfilter.a
            ${IOS_LIB_DIR}/libavformat.a
            ${IOS_LIB_DIR}/libswresample.a
            ${IOS_LIB_DIR}/libswscale.a
            ${IOS_LIB_DIR}/libavutil.a
            ${IOS_LIB_DIR}/libuchardet.a
            ${IOS_LIB_DIR}/libharfbuzz.a
            ${IOS_LIB_DIR}/libfribidi.a
            ${IOS_LIB_DIR}/libfreetype.a
            ${IOS_LIB_DIR}/libass.a
            ${IOS_LIB_DIR}/libssl.a
            ${IOS_LIB_DIR}/libcrypto.a
            ${IOS_LIB_DIR}/libwebp.a
            ${IOS_LIB_DIR}/libsharpyuv.a
            z
            bz2
            "-framework CoreMedia -framework CoreText -framework VideoToolbox"
            )
else ()
    if (USE_SDL2)
        set(PLATFORM_LIBS SDL2)
    else ()
        set(PLATFORM_LIBS glfw3)
    endif ()
    list(
        APPEND
        PLATFORM_LIBS
        # needed by borealis
        EGL
        glapi
        drm_nouveau
        # needed by mpv
        mpv
        swscale
        swresample
        avformat
        avfilter
        avcodec
        avutil
        ass
        freetype
        fribidi
        harfbuzz
        png
        bz2
        z
        mbedx509
        mbedtls
        mbedcrypto
        # needed by cpr
        curl
        z
        # base lib
        nx
        m
    )
    if (BUILTIN_NSP)
        list(APPEND PLATFORM_LIBS nsp)
        list(APPEND PLATFORM_OPTION -DBUILTIN_NSP)
    endif ()
    # compile option for mongoose
    list(APPEND PLATFORM_OPTION -DMG_ARCH=10086)
    list(
        APPEND
        MAIN_SRC
        ${CMAKE_SOURCE_DIR}/library/borealis/library/lib/platforms/switch/switch_wrapper.c
    )
endif ()

# build borealis qrcode and other third party libraries
add_subdirectory(library)

if (NOT PLATFORM_IOS)
    find_package(WebP)
    if (WebP_FOUND)
        message(STATUS "Found webp: ${WebP_INCLUDE_DIR} ${WebP_LIBRARY}")
        list(APPEND PLATFORM_OPTION -DUSE_WEBP)
        list(APPEND PLATFORM_INCLUDES ${WebP_INCLUDE_DIRS})
        list(APPEND PLATFORM_LIBS ${WebP_LIBRARY})
    endif ()
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

# download macos dylib dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos.cmake)

# set google analytics
if (ANALYTICS)
    if (NOT ANALYTICS_ID OR NOT ANALYTICS_KEY)
        message(
            FATAL_ERROR
                "ANALYTICS set to ON, You need set ANALYTICS_ID and ANALYTICS_KEY too"
        )
    endif ()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -DANALYTICS=${ANALYTICS} -DANALYTICS_ID=${ANALYTICS_ID}
                -DANALYTICS_KEY=${ANALYTICS_KEY}
    )
endif ()

if (VERIFY_SSL)
    list(APPEND PLATFORM_OPTION -DVERIFY_SSL)
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(
    ${PROJECT_NAME} PRIVATE wiliwili/include wiliwili/include/api
                            ${PLATFORM_INCLUDES}
)
target_compile_options(
    ${PROJECT_NAME}
    PRIVATE -D__VERSION_MAJOR__=${VERSION_MAJOR}
            -D__VERSION_MINOR__=${VERSION_MINOR}
            -D__VERSION_REVISION__=${VERSION_REVISION}
            -ffunction-sections
            -fdata-sections
            -Wunused-variable
            ${PLATFORM_OPTION}
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE cpr::cpr
            qrcode
            borealis
            lunasvg
            pystring
            mongoose
            z
            ${PLATFORM_LIBS}
)

# building release file
if (PLATFORM_DESKTOP)
    add_custom_target(
        ${PROJECT_NAME}.data
        COMMAND
            "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
    if (NOT USE_LIBROMFS)
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
    endif ()

    if (APPLE)
        if (MAC_DOWNLOAD_DYLIB)
            set(bundle_args1 "-nb")
            set(bundle_args2 "${CMAKE_BINARY_DIR}/deps/${MAC_OS_ARCH}")
        endif ()
        add_custom_target(
            ${PROJECT_NAME}.app
            COMMAND "bash" "${CMAKE_BINARY_DIR}/../scripts/build_mac.sh" "${bundle_args1}" "${bundle_args2}"
        )
        add_dependencies(${PROJECT_NAME}.app ${PROJECT_NAME})
    elseif (UNIX)
        if (INSTALL)
            message("install prefix: ${CMAKE_INSTALL_PREFIX}")
            # bin
            install(TARGETS ${PROJECT_NAME}
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            )

            # resources
            install(
                DIRECTORY ${PROJECT_RESOURCES}
                DESTINATION ${RESOURCES_INSTALL_DIR}
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            )

            # svg icon
            install(
                FILES ${PROJECT_ICON_SVG}
                DESTINATION
                    ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            )

            # png icon
            set(icon_dirs
                16x16
                24x24
                32x32
                48x48
                64x64
                128x128
                256x256
            )
            foreach (icon_dir ${icon_dirs})
                install(
                    FILES
                        ${CMAKE_SOURCE_DIR}/scripts/linux/icons/${icon_dir}/${PACKAGE_NAME}.png
                    DESTINATION
                        ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${icon_dir}/apps
                    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
                )
            endforeach ()

            # appdata
            install(
                FILES
                    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/${PACKAGE_NAME}.appdata.xml
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            )

            # desktop
            install(FILES ${PROJECT_DESKTOP}
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
            )
        endif ()
    endif ()
elseif (PLATFORM_IOS)
    set(IOS_CODE_SIGN_IDENTITY "" CACHE STRING "The code sign identity to use when building the IPA.")
    if(IOS_CODE_SIGN_IDENTITY STREQUAL "")
        set(IOS_CODE_SIGNING_ENABLED NO)
    else()
        set(IOS_CODE_SIGNING_ENABLED YES)
    endif()
    if (NOT DEFINED IOS_GUI_IDENTIFIER)
        set(IOS_GUI_IDENTIFIER "${PACKAGE_NAME}")
        message(WARNING "Using default package name: ${PACKAGE_NAME}")
    endif ()
    if (TVOS)
        set(IOS_SPLASH_STORYBOARD "${CMAKE_SOURCE_DIR}/library/borealis/demo/ios/tvos/Splash.storyboard")
    else ()
        set(IOS_SPLASH_STORYBOARD "${CMAKE_SOURCE_DIR}/library/borealis/demo/ios/iphoneos/Splash.storyboard")
    endif ()
    set(IOS_ASSETS ${CMAKE_SOURCE_DIR}/library/borealis/demo/ios/Images.xcassets)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${IOS_CODE_SIGN_IDENTITY}"
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "${IOS_CODE_SIGNING_ENABLED}"
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${IOS_CODE_SIGNING_ENABLED}"
            BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/scripts/ios/iOSBundleInfo.plist.in
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER ${IOS_GUI_IDENTIFIER}
            MACOSX_BUNDLE_BUNDLE_NAME wiliwili
            MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}"
            XCODE_ATTRIBUTE_ENABLE_BITCODE NO
            XCODE_ATTRIBUTE_SKIP_INSTALL NO
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
            )
    set_property(
            SOURCE ${IOS_ASSETS}
            PROPERTY MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_property(
            SOURCE ${IOS_SPLASH_STORYBOARD}
            PROPERTY MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${PROJECT_NAME} PRIVATE
            ${IOS_ASSETS}
            ${IOS_SPLASH_STORYBOARD}
            )
else ()
    set(BUILD_FONT_DIR ${CMAKE_BINARY_DIR}/resources/font)
    set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")
    if (GIT_TAG_VERSION)
        string(SUBSTRING ${GIT_TAG_VERSION} 1 -1 APP_VERSION)
    endif ()
    add_custom_target(${PROJECT_NAME}.nro
        DEPENDS ${PROJECT_NAME}
        COMMAND ${NX_NACPTOOL_EXE} --create ${PROJECT_NAME} ${PROJECT_AUTHOR}
                ${APP_VERSION} ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES}
                ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E remove -f
                ${CMAKE_BINARY_DIR}/resources/icon/wiliwili.ico
        COMMAND ${CMAKE_COMMAND} -E remove -f ${BUILD_FONT_DIR}/*.txt
                ${BUILD_FONT_DIR}/switch_font.ttf ${BUILD_FONT_DIR}/keymap*.ttf
        COMMAND
            ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro
            --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp
            --romfsdir=${CMAKE_BINARY_DIR}/resources
    )
endif ()

cmake_minimum_required(VERSION 3.13)

set(BOREALIS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library/borealis)
set(BOREALIS_LIBRARY ${BOREALIS_DIR}/library)

# Include Platform common options
# wiliwili only support PLATFORM_DESKTOP and PLATFORM_SWITCH
# example: cmake -B build -DPLATFORM_DESKTOP=ON
include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)

# NintendoSwitch
cmake_dependent_option(BUILTIN_NSP "Built in NSP forwarder" OFF "PLATFORM_SWITCH" OFF)

# macOS
# These four options are only for macOS app bundling usage.
# Do not use it until you know what you are doing.
cmake_dependent_option(MAC_IntelChip_Old "Build for x86_64 (10.11)" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_IntelChip "Build for x86_64 (10.15)" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_AppleSilicon "Build for arm64 (11.0)" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_Universal "Support x86_64;arm64 at the same time" OFF "APPLE;NOT PLATFORM_IOS" OFF)
cmake_dependent_option(MAC_DOWNLOAD_DYLIB "Using precompiled dylib instead of system provided" OFF "APPLE;NOT PLATFORM_IOS" OFF)

# Linux
# Using CMAKE_INSTALL_PREFIX to specify the location where the application is installed.
cmake_dependent_option(INSTALL "Install to system path" OFF "UNIX;NOT APPLE" OFF)

# For Developer
option(DEBUG_SANITIZER "Turn on sanitizers (only available in debug build)" OFF)

# Google Analytics
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoogleAnalytics.cmake)

# Dependencies
option(USE_SHARED_LIB "Whether to use shared libs provided by system" OFF)
option(USE_SYSTEM_CURL "" ${USE_SHARED_LIB})
option(USE_SYSTEM_CPR "" ${USE_SHARED_LIB})
option(USE_SYSTEM_QRCODEGEN "" ${USE_SHARED_LIB})
option(USE_SYSTEM_PYSTRING "" ${USE_SHARED_LIB})
option(USE_SYSTEM_OPENCC "" ${USE_SHARED_LIB})
option(USE_SYSTEM_FMT "" ${USE_SHARED_LIB})
option(USE_SYSTEM_TINYXML2 "" ${USE_SHARED_LIB})
option(USE_SYSTEM_SDL2 "" ${USE_SHARED_LIB})
option(USE_SYSTEM_MONGOOSE "" ${USE_SHARED_LIB})
option(USE_SYSTEM_GLFW "Not recommended because we used a modified GLFW" OFF)

option(DISABLE_OPENCC "Disable Chinese Simplified and Chinese Traditional Conversion" OFF)
option(DISABLE_WEBP "Disable webp" OFF)

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

# Bundle mpv.dll into wiliwili.exe (Windows only)
cmake_dependent_option(MPV_BUNDLE_DLL "Bundle mpv.dll" OFF "USE_LIBROMFS;WIN32" OFF)
if (MPV_BUNDLE_DLL)
    list(APPEND APP_PLATFORM_OPTION -DMPV_BUNDLE_DLL)
    list(APPEND APP_PLATFORM_LIB MemoryModule)
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra.cmake)

# toolchain
include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)

# Git Info
git_info(GIT_TAG_VERSION GIT_TAG_SHORT)
list(APPEND APP_PLATFORM_OPTION -DBUILD_TAG_VERSION=${GIT_TAG_VERSION} -DBUILD_TAG_SHORT=${GIT_TAG_SHORT})

# nanovg
add_definitions(-DFONS_HASH_LUT_SIZE=4096)

# Project Info
project(wiliwili)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "5")
set(VERSION_REVISION "0")
if (NOT VERSION_BUILD)
    set(VERSION_BUILD "0")
endif ()
set(PROJECT_AUTHOR "xfangfang")
set(PACKAGE_NAME cn.xfangfang.wiliwili)
set(VITA_TITLEID "WILIWILI0")
set(PSN_TITLE_ID "WILI00000")
set(VITA_VERSION "01.50")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(Threads REQUIRED)
list(APPEND APP_PLATFORM_LIB ${CMAKE_THREAD_LIBS_INIT})

# Find libmpv and libwebp
if (MAC_DOWNLOAD_DYLIB)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos.cmake)
    list(APPEND CMAKE_MODULE_PATH
            ${CMAKE_BINARY_DIR}/deps/lib/cmake)
elseif (PLATFORM_DESKTOP)
    find_package(MPV REQUIRED)
    message(STATUS "Found libmpv: ${MPV_VERSION} ${MPV_INCLUDE_DIR} ${MPV_LIBRARY}")
    list(APPEND APP_PLATFORM_INCLUDE ${MPV_INCLUDE_DIR})
    if (NOT MPV_BUNDLE_DLL)
        list(APPEND APP_PLATFORM_LIB ${MPV_LIBRARY})
    endif ()

    find_package(WebP)
    if (WebP_FOUND AND NOT DISABLE_WEBP)
        message(STATUS "Found webp: ${WebP_INCLUDE_DIRS} ${WebP_LIBRARY}")
        list(APPEND APP_PLATFORM_OPTION -DUSE_WEBP)
        list(APPEND APP_PLATFORM_INCLUDE ${WebP_INCLUDE_DIRS})
        list(APPEND APP_PLATFORM_LIB ${WebP_LIBRARY})
    endif ()

    if (MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static" CACHE STRING "" FORCE)
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
    if (LIBWEBP_FOUND AND NOT DISABLE_WEBP)
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
elseif (PLATFORM_PS4)
    # Show backtrace when app crash
    list(APPEND MAIN_SRC ${BOREALIS_LIBRARY}/lib/platforms/ps4/crashlog.c)
endif ()

# build borealis qrcode and other third party libraries
add_subdirectory(library)

# Main Target
program_target(${PROJECT_NAME} "${MAIN_SRC}")
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        UNITY_BUILD ${BRLS_UNITY_BUILD}
)
set_source_files_properties(
        wiliwili/source/utils/config_helper.cpp
        wiliwili/source/activity/setting_activity.cpp
        wiliwili/source/activity/dlna_activity.cpp
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

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
            set(bundle_args1 "-nb")
            set(bundle_args2 "${CMAKE_BINARY_DIR}/deps/lib")
        else ()
            # Use system provided dylib
            set(bundle_args1 "-b")
            set(bundle_args2 "system")
        endif ()
        add_custom_target(
            ${PROJECT_NAME}.app
            COMMAND "bash" "${CMAKE_SOURCE_DIR}/scripts/build_mac.sh" "${bundle_args1}" "${bundle_args2}" "${APP_VERSION}"
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
elseif (PLATFORM_PS4)
    add_self(${PROJECT_NAME})
    add_pkg(${PROJECT_NAME}
            ${CMAKE_BINARY_DIR}/ps4
            ${PSN_TITLE_ID}
            "wiliwili"
            ${VITA_VERSION}
            8388608
    )
    add_custom_target(${PROJECT_NAME}.data
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${BOREALIS_DIR}/ps4/sce_module ${CMAKE_BINARY_DIR}/ps4/sce_module
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_SOURCE_DIR}/scripts/ps4/sce_sys ${CMAKE_BINARY_DIR}/ps4/sce_sys
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${BOREALIS_DIR}/ps4/sce_sys/about ${CMAKE_BINARY_DIR}/ps4/sce_sys/about
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/ps4/resources
            # Rename ja-RYU to RYU, as there may be a bug in PkgTool that not allow dirs with the same prefix exist.
            COMMAND "${CMAKE_COMMAND}" -E remove_directory ${CMAKE_BINARY_DIR}/ps4/resources/i18n/RYU
            COMMAND "${CMAKE_COMMAND}" -E rename ${CMAKE_BINARY_DIR}/ps4/resources/i18n/ja-RYU ${CMAKE_BINARY_DIR}/ps4/resources/i18n/RYU
    )
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
elseif (PLATFORM_SWITCH)
    set(BUILD_FONT_DIR ${CMAKE_BINARY_DIR}/resources/font)
    if (GIT_TAG_VERSION)
        string(SUBSTRING ${GIT_TAG_VERSION} 1 -1 APP_VERSION)
    endif ()
    if (BUILTIN_NSP)
        add_dependencies(${PROJECT_NAME} nsp_forwarder.nsp)
    endif ()
    if (USE_DEKO3D)
        gen_dksh("${PROJECT_RESOURCES}/shaders")
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
target_link_libraries(${PROJECT_NAME} PRIVATE wiliwiliLibExtra borealis lunasvg pystring pdr mongoose z ${APP_PLATFORM_LIB})
target_link_options(${PROJECT_NAME} PRIVATE ${APP_PLATFORM_LINK_OPTION})

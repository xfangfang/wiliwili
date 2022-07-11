cmake_minimum_required(VERSION 3.15)

# import cpr
set(CPR_FORCE_USE_SYSTEM_CURL ON)
if (PLATFORM_DESKTOP)
    if (APPLE)
        set(CPR_FORCE_DARWINSSL_BACKEND ON)
    endif ()
else()
    set(CPR_FORCE_MBEDTLS_BACKEND ON)
endif()
FetchContent_Declare(cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 2157a3126c2ef1b76f339dedd6d7585e39651512) # 2022/06/01
FetchContent_MakeAvailable(cpr)

#import lunasvg
FetchContent_Declare(lunasvg
        GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
        GIT_TAG ef111f983225d7598cb9044796ed3b1146927135)
FetchContent_MakeAvailable(lunasvg)

# add QR-Code-generator
add_library(qrcode STATIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp/QrCode.cpp)
target_include_directories(qrcode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/QR-Code-generator/cpp)
set_property(TARGET qrcode PROPERTY CXX_STANDARD 17)

# add pystring
add_library(pystring STATIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring/pystring.cpp)
target_include_directories(pystring PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pystring)
set_property(TARGET pystring PROPERTY CXX_STANDARD 17)

# add borealis
set(BOREALIS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/borealis/library)
set(NANOVG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/nanovg/src)

set(BOREALIS_INCLUDE
        ${BOREALIS_PATH}/include
        ${BOREALIS_PATH}/lib/extern/fmt/include
        ${BOREALIS_PATH}/include/borealis/extern
        ${BOREALIS_PATH}/include/borealis/extern/tinyxml2
        ${BOREALIS_PATH}/lib/extern/yoga/src
        ${BOREALIS_PATH}/lib/extern/tweeny/include
        ${BOREALIS_PATH}/include/borealis/extern/nanovg-gl
#        ${NANOVG_PATH}
        ${BOREALIS_PATH}/lib/platforms/glfw
        )
set(BOREALIS_SOURCE
        ${BOREALIS_PATH}/lib/core
        ${BOREALIS_PATH}/lib/core/touch
        ${BOREALIS_PATH}/lib/views
        ${BOREALIS_PATH}/lib/views/cells
        ${BOREALIS_PATH}/lib/views/widgets
        ${BOREALIS_PATH}/lib/extern/glad
        ${BOREALIS_PATH}/lib/extern/tinyxml2
        ${BOREALIS_PATH}/lib/extern/libretro-common/compat
        ${BOREALIS_PATH}/lib/extern/libretro-common/encodings
        ${BOREALIS_PATH}/lib/extern/libretro-common/features
        ${BOREALIS_PATH}/lib/extern/yoga/src/yoga
        ${BOREALIS_PATH}/lib/extern/yoga/src/yoga/event
#        ${NANOVG_PATH}
        ${BOREALIS_PATH}/lib/extern/nanovg-gl
        )

set(BOREALIS_SRC)
if (PLATFORM_DESKTOP)
    find_package(glfw3 REQUIRED)
    list(APPEND BOREALIS_SOURCE
            ${BOREALIS_PATH}/lib/platforms/glfw
            ${BOREALIS_PATH}/lib/extern/fmt/src/
            )
    list(APPEND BOREALIS_SRC
            ${BOREALIS_PATH}/lib/platforms/switch/swkbd.cpp
            )
    set(BRLS_PLATFORM_RESOURCES_PATH "\"./resources/\"")
    set(BRLS_PLATFORM_LIBS glfw)
    set(BRLS_PLATFORM_OPTION -DNO_TOUCH_SCROLLING=true -D__GLFW__)
else()
    list(APPEND BOREALIS_SOURCE
            ${BOREALIS_PATH}/lib/platforms/switch
            ${BOREALIS_PATH}/lib/extern/nxfmtwrapper
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/archive
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/bfgrp
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/bfsar
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/bfwar
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/bfwav
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/bfwsd
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/src/player
            )
    list(APPEND BOREALIS_INCLUDE
            ${BOREALIS_PATH}/lib/extern/switch-libpulsar/include
            )
    list(APPEND BOREALIS_SRC ${BOREALIS_PATH}/lib/platforms/glfw/glfw_video.cpp)
    set(BRLS_PLATFORM_RESOURCES_PATH "\"romfs:/\"")
    set(BRLS_PLATFORM_LIBS glfw3 EGL glapi drm_nouveau nx m)
    set(BRLS_PLATFORM_OPTION $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile> -D__SWITCH__)
endif()
foreach(lib ${BOREALIS_SOURCE})
    aux_source_directory(${lib} TEMP_SRC)
    list(APPEND BOREALIS_SRC ${TEMP_SRC})
endforeach(lib)


add_library(borealis STATIC ${BOREALIS_SRC})
set_property(TARGET borealis PROPERTY CXX_STANDARD 17)
target_include_directories(borealis PUBLIC ${BOREALIS_INCLUDE})
target_link_libraries(borealis ${BRLS_PLATFORM_LIBS})
target_compile_options(borealis PUBLIC
        -DBRLS_RESOURCES=${BRLS_PLATFORM_RESOURCES_PATH}
        -DYG_ENABLE_EVENTS
        -g -Wall -O2
        -ffunction-sections
        -fdata-sections
        ${BRLS_PLATFORM_OPTION}
        )

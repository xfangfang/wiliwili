cmake_minimum_required(VERSION 3.15)
if(PLATFORM_SWITCH:BOOL)
  set(PLATFORM_SWITCH ON)
elseif(PLATFORM_LINUX:BOOL)
  set(PLATFORM_LINUX ON)
endif()

# toolchain
if(PLATFORM_LINUX)
  message("building for LINUX")
  set(CMAKE_SYSTEM_NAME "Linux")
  set(TARGET_PLATFORM linux CACHE STRING "")
elseif (PLATFORM_SWITCH)
  message("building for SWITCH")
  file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)
  if(NOT IS_DIRECTORY ${DEVKITPRO})
      message(FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
  endif()
  set(DEVKITPRO $ENV{DEVKITPRO} CACHE BOOL "DEVKITPRO")
  set(CMAKE_SYSTEM_NAME "Generic")
  set(CMAKE_SYSTEM_VERSION 1 )
  set(CMAKE_SYSTEM_PROCESSOR aarch64)
  set(CMAKE_EXE_LINKER_FLAGS_INIT "-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -ftls-model=local-exec -L/opt/devkitpro/portlibs/switch/lib -L/opt/devkitpro/libnx/lib -specs=/opt/devkitpro/libnx/switch.specs")
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS_INIT} CACHE STRING "" FORCE)
  set(CMAKE_C_COMPILER "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-gcc")
  set(CMAKE_CXX_COMPILER "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-g++")
  set(CMAKE_ASM_COMPILER "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-as")
  set(CMAKE_AR "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-gcc-ar" CACHE FILEPATH "")
  set(CMAKE_AS "${DEVKITA64}/bin/aarch64-none-elf-as" CACHE STRING "")
  set(CMAKE_NM "${DEVKITA64}/bin/aarch64-none-elf-gcc-nm" CACHE STRING "")
  set(CMAKE_LINKER "${DEVKITA64}/bin/aarch64-none-elf-ld")
  set(CMAKE_RANLIB "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-gcc-ranlib")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include" CACHE STRING "C flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fpermissive" CACHE STRING "C++ flags")
  set(ENV{PKG_CONFIG_DIR} "")
  set(ENV{PKG_CONFIG_LIBDIR} "${DEVKITPRO}/portlibs/switch/lib/pkgconfig")
  set(ENV{PKG_CONFIG_SYSROOT_DIR} "${DEVKITPRO}/portlibs/switch")
  set(CMAKE_FIND_ROOT_PATH ${DEVKITPRO}/libnx ${DEVKITPRO}/portlibs/switch)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  set(TARGET_PLATFORM switch CACHE STRING "")
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Shared libs not wanted")
endif()

# project info
project(wiliwili)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(PROJECT_AUTHOR "xfangfang")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/bilibili.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)


# import cpr library from github
set(USE_SYSTEM_CURL ON)
set(BUILD_CPR_TESTS OFF)
set(CMAKE_USE_OPENSSL OFF)
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

# building bilibili-api library
if (PLATFORM_LINUX)
elseif(PLATFORM_SWITCH)
        set(PLATFORM_LIBS_BILIBILI curl z nx)
endif()
aux_source_directory(${CMAKE_SOURCE_DIR}/library/bilibili-api-cpp/source BILIBILI_API_SRC)
set(BILIBILI_API_INCLUDE ${CMAKE_SOURCE_DIR}/library/bilibili-api-cpp/include)
add_library(bilibili_api STATIC ${BILIBILI_API_SRC})
target_compile_options(bilibili_api PRIVATE -g -Wall -O2 -ffunction-sections -fno-rtti -fno-exceptions)
target_include_directories(bilibili_api PRIVATE ${BILIBILI_API_INCLUDE})
target_link_libraries(bilibili_api PRIVATE cpr::cpr ${PLATFORM_LIBS_BILIBILI})


# setting borealis library src and include
set(BOREALIS_PATH ${CMAKE_SOURCE_DIR}/library/borealis/library)
set(BOREALIS_INCLUDE
        ${BOREALIS_PATH}/include
        ${BOREALIS_PATH}/lib/extern/fmt/include
        ${BOREALIS_PATH}/include/borealis/extern
        )
set(BOREALIS_SOURCE
        ${BOREALIS_PATH}/lib
        ${BOREALIS_PATH}/lib/extern/glad
        ${BOREALIS_PATH}/lib/extern/nanovg
        ${BOREALIS_PATH}/lib/extern/libretro-common/compat
        ${BOREALIS_PATH}/lib/extern/libretro-common/encodings
        ${BOREALIS_PATH}/lib/extern/libretro-common/features
        ${BOREALIS_PATH}/lib/extern/nxfmtwrapper
        )
set(BOREALIS_SRC)
foreach(lib ${BOREALIS_SOURCE})
        aux_source_directory(${lib} TEMP_SRC)
        list(APPEND BOREALIS_SRC ${TEMP_SRC})
endforeach(lib)

# seting wiliwili src and include
set(PLATFORM_OPTION)
if (PLATFORM_LINUX)
        add_definitions(-D__PLATFORM_LINUX__)
        set(PLATFORM_RESOURCES_PATH "\"./resources/\"")
        set(PLATFORM_LIBS mpv glfw)
        list(REMOVE_ITEM BOREALIS_SRC
                ${BOREALIS_PATH}/lib/switch_wrapper.c
                ${BOREALIS_PATH}/lib/extern/nxfmtwrapper/format.cpp
              )
        list(APPEND BOREALIS_SRC
                ${BOREALIS_PATH}/lib/extern/fmt/src/format.cc
                ${BOREALIS_PATH}/lib/extern/fmt/src/os.cc
              )
elseif (PLATFORM_SWITCH)
        set(PLATFORM_LIBS
          # needed by mpv
          mpv swscale swresample avformat avfilter avcodec avutil opus vpx ass freetype fribidi png bz2 z SDL2
          # needed by borealis
          glfw3 EGL glapi drm_nouveau m
          # base lib
          nx
        )
        add_definitions(-DSWITCH -D__SWITCH__ -DHAVE_LIBNX)
        set(PLATFORM_RESOURCES_PATH "\"romfs:\"")
        list(APPEND PLATFORM_OPTION 
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
        )
endif()
aux_source_directory(./bilibili WILIWILI_SRC)


# building target
add_executable(${PROJECT_NAME}
        ${WILIWILI_SRC}
        ${BOREALIS_SRC}
      )

target_compile_options(${PROJECT_NAME} PRIVATE
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        -DBOREALIS_RESOURCES=${PLATFORM_RESOURCES_PATH}
      )
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -g -Wall -O2 -ffunction-sections
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
        ${PLATFORM_OPTION}
      )
target_compile_options(${PROJECT_NAME} PRIVATE -DBOREALIS_RESOURCES=${PLATFORM_RESOURCES_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE
        bilibili_api
        ${PLATFORM_LIBS}
        pthread
      )
target_include_directories(${PROJECT_NAME} PRIVATE
        bilibili/
        ${BOREALIS_INCLUDE}
        ${BILIBILI_API_INCLUDE}
      )

# building release file
if(PLATFORM_LINUX)
    set(ZIP zip)
    add_custom_target(${PROJECT_NAME}.zip
            DEPENDS ${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}_PLATFORM_LINUX.zip
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/resources
            COMMAND cd ${CMAKE_BINARY_DIR}/release && ${ZIP} -r ../${PROJECT_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}_PLATFORM_LINUX.zip ${PROJECT_NAME}
          )
    add_custom_target(${PROJECT_NAME}.data
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
elseif(PLATFORM_SWITCH)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
  add_custom_target(${PROJECT_NAME}.nro
    DEPENDS ${PROJECT_NAME}
    COMMAND ${DEVKITPRO}/tools/bin/nacptool --create "${PROJECT_NAME}" "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}" ${PROJECT_NAME}.nacp
    COMMAND ${DEVKITPRO}/tools/bin/elf2nro ${PROJECT_NAME} ${PROJECT_NAME}.nro --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp --romfsdir=${PROJECT_RESOURCES}
  )
endif()

cmake_minimum_required(VERSION 3.15)

# Add debug info
IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
ENDIF ()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-D_DEBUG)
    add_definitions(-D_GLIBCXX_ASSERTIONS)
    if (DEBUG_SANITIZER)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -O0 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address")
    endif ()
endif ()

# Add git info
execute_process(COMMAND git describe --tags
        TIMEOUT 5
        OUTPUT_VARIABLE GIT_TAG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
execute_process(COMMAND git rev-parse --short HEAD
        TIMEOUT 5
        OUTPUT_VARIABLE GIT_TAG_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

add_definitions(-DBUILD_TAG_VERSION=${GIT_TAG_VERSION} -DBUILD_TAG_SHORT=${GIT_TAG_SHORT})

message(STATUS "building from git tag ${GIT_TAG_VERSION}")
message(STATUS "building from git commit ${GIT_TAG_SHORT}")

if (APPLE)
    if (MAC_IntelChip)
        message(STATUS "CMAKE_OSX_ARCHITECTURES: x86_64")
        message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET: 10.11")
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "" FORCE)

        set(USE_BOOST_FILESYSTEM ON)
    endif ()
    if(MAC_AppleSilicon)
        # Build a Universal binary on macOS
        message(STATUS "CMAKE_OSX_ARCHITECTURES: arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
    endif()
    if(MAC_Universal)
        # Build a Universal binary on macOS
        message(STATUS "CMAKE_OSX_ARCHITECTURES: x86_64;arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "" FORCE)
        set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "" FORCE)

        set(USE_BOOST_FILESYSTEM ON)
    endif()
endif()

if (USE_BOOST_FILESYSTEM)
    set(CPR_USE_BOOST_FILESYSTEM ON)
    add_definitions(-DCPR_USE_BOOST_FILESYSTEM)
    add_definitions(-DUSE_BOOST_FILESYSTEM)
endif ()

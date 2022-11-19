cmake_minimum_required(VERSION 3.15)

# Add debug info
IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    message("Build Type: ${CMAKE_BUILD_TYPE}")
ENDIF ()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-D_DEBUG)
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
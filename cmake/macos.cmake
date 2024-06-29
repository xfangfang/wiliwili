# Download precompiled dylib dependencies
# arm64: 11.0
# x86_64: 10.11
# universal: arm64 + x86_64

if (APPLE)

    if (MAC_IntelChip)
        set(MAC_OS_ARCH x86_64)
    elseif (MAC_AppleSilicon)
        set(MAC_OS_ARCH arm64)
    elseif (MAC_Universal)
        set(MAC_OS_ARCH universal)
    endif ()

    if (NOT DISABLE_WEBP)
        list(APPEND APP_PLATFORM_OPTION -DUSE_WEBP)
        list(APPEND APP_PLATFORM_LIB ${CMAKE_BINARY_DIR}/deps/lib/libwebp.7.1.9.dylib)
    endif ()

    list(APPEND APP_PLATFORM_LIB
            ${CMAKE_BINARY_DIR}/deps/lib/libmpv.2.dylib
            ${CMAKE_BINARY_DIR}/deps/lib/libboost_filesystem-mt.dylib
            ${CMAKE_BINARY_DIR}/deps/lib/libssl.3.dylib
            ${CMAKE_BINARY_DIR}/deps/lib/libcrypto.3.dylib)
    list(APPEND APP_PLATFORM_INCLUDE
            ${CMAKE_BINARY_DIR}/deps/include)

    # download deps
    include(FetchContent)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(macos_prebuild
            URL "https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/macos_dylib_ffmpeg7_mpv38_${MAC_OS_ARCH}.tar.gz"
    )
    if (NOT macos_prebuild_POPULATED)
        FetchContent_POPULATE(macos_prebuild)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/deps)
        file(COPY ${macos_prebuild_SOURCE_DIR}/lib DESTINATION ${CMAKE_BINARY_DIR}/deps)
        file(COPY ${macos_prebuild_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR}/deps)
    endif()

endif ()
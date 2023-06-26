# Download precompiled dylib dependencies
# arm64: 11.0
# x86_64: 10.11
# universal: arm64 + x86_64

# header files: `brew tap xfangfang/wiliwili && brew install mpv-wiliwili webp boost`

if (APPLE)

    if (MAC_INTEL)
        set(MAC_OS_ARCH x86_64)
    elseif (MAC_ARM)
        set(MAC_OS_ARCH arm64)
    elseif (MAC_UNIVERSAL)
        set(MAC_OS_ARCH universal)
    endif ()

    if (MAC_DOWNLOAD_DYLIB)
        link_libraries(
                ${CMAKE_BINARY_DIR}/deps/${MAC_OS_ARCH}
        )
        set(PLATFORM_LIBS
                ${CMAKE_BINARY_DIR}/deps/${MAC_OS_ARCH}/libmpv.2.dylib
                ${CMAKE_BINARY_DIR}/deps/${MAC_OS_ARCH}/libwebp.7.1.6.dylib
                ${CMAKE_BINARY_DIR}/deps/${MAC_OS_ARCH}/libboost_filesystem-mt.dylib)

        # download deps
        add_custom_target(
                ${PROJECT_NAME}.macos_${MAC_OS_ARCH}_deps
                COMMAND "bash" "${CMAKE_BINARY_DIR}/../scripts/macos_dylib_downloader.sh" "${MAC_OS_ARCH}"
        )
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.macos_${MAC_OS_ARCH}_deps)
    endif ()
endif ()
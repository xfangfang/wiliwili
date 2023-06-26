if (MAC_UNIVERSAL)
    link_libraries(
            ${CMAKE_BINARY_DIR}/deps/universal
    )
    set(PLATFORM_LIBS
            ${CMAKE_BINARY_DIR}/deps/universal/libmpv.2.dylib
            ${CMAKE_BINARY_DIR}/deps/universal/libwebp.7.1.6.dylib)

    # download deps
#    add_custom_target(
#            ${PROJECT_NAME}.universal_deps
#            COMMAND "bash" "${CMAKE_BINARY_DIR}/../scripts/build_mac.sh"
#    )
endif ()
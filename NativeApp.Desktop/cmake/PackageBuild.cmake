set(PACKAGE_DIR "${SOLUTION_DIR}/artifacts/${CMAKE_BUILD_TYPE}-${NATIVE_ARCH_ID}")
option(PACKAGE_BUILD "Should the CMake package the executable build to a separate directory." OFF)

# This will copy everything to a clean directory for distribution.
if (PACKAGE_BUILD)
    add_custom_command(TARGET NativeApp.Desktop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Packaging all files together to the executable directory: ${PACKAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MSBUILD_GENERATED} ${PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MONO_RUNTIME_DIST} ${PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:NativeApp.Desktop> ${PACKAGE_DIR}
    )
    
    if (DEFINED WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(TARGET NativeApp.Desktop POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Packaging DEBUG-only files together to the executable directory: ${PACKAGE_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:NativeApp.Desktop> ${PACKAGE_DIR}
        )
    endif()
endif()

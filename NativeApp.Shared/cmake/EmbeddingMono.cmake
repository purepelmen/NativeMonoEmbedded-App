# DESCRIPTION ==================================================
# Define all required paths and names, but don't perform actual embedding. This is what an app container should do.

set(MONO_RUNTIME_DIR "${SOLUTION_DIR}/thirdparty/Runtime.Mono/")

if(ANDROID)
    set(MONO_RUNTIME_LIBS "${MONO_RUNTIME_DIR}/gen-android/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
else()
    set(MONO_RUNTIME_LIBS "${MONO_RUNTIME_DIR}/gen-${OSNAME_ID}/libs-${NATIVE_ARCH_ID}")
    set(MONO_RUNTIME_DIST "${MONO_RUNTIME_DIR}/gen-${OSNAME_ID}/dist-${NATIVE_ARCH_ID}")

    if(NOT EXISTS ${MONO_RUNTIME_DIST})
        message(FATAL_ERROR "Mono Runtime dist files are missing for your setup. They should be at path: ${MONO_RUNTIME_DIST}")
    endif()
endif()

if(WIN32)
    set(MONO_REQUIRED_LINK_LIBS "coreclr.lib")
elseif(ANDROID)
    set(MONO_REQUIRED_LINK_LIBS "libmonosgen-2.0.so")
else()
    set(MONO_REQUIRED_LINK_LIBS "coreclr.so")
endif()

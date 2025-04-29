# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

cmake_minimum_required(VERSION 3.8)

set(SOLUTION_DIR "${CMAKE_SOURCE_DIR}/../../../../../")
set(ANDROID_ASSETS_GENERATED "${CMAKE_SOURCE_DIR}/../assetsGen/")

set(OSNAME_ID "android")
if (CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
    set(NATIVE_ARCH_ID "arm")
elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    set(NATIVE_ARCH_ID "arm64")
elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    set(NATIVE_ARCH_ID "x64")
else()
    # Assuming x86 architecture.
    set(NATIVE_ARCH_ID "${CMAKE_ANDROID_ARCH_ABI}")
endif()
add_compile_definitions(PLATFORM_ANDROID)
add_compile_definitions(PLATFORM_ANDROID_ARCH=${CMAKE_ANDROID_ARCH_ABI})
add_compile_definitions(PLATFORM_ANDROID_ARCH_STRINGIFIED="${CMAKE_ANDROID_ARCH_ABI}")

add_subdirectory("${SOLUTION_DIR}/NativeApp.Shared/" ./Shared/)

project("nativeapp")
add_library(${CMAKE_PROJECT_NAME} SHARED native_lib.cpp android_mono.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE NativeApp.Shared)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    android
    log
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)
endif()

include("${SOLUTION_DIR}/NativeApp.Shared/cmake/EmbeddingMono.cmake")
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE "${MONO_RUNTIME_LIBS}")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MONO_REQUIRED_LINK_LIBS})

add_custom_target(CopyManagedProject
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MSBUILD_GENERATED} ${ANDROID_ASSETS_GENERATED}
        COMMENT "Copying MSBuild outputs."
        VERBATIM
)
add_dependencies(CopyManagedProject BuildManagedProject)
add_dependencies(${CMAKE_PROJECT_NAME} CopyManagedProject)

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

cmake_minimum_required(VERSION 3.22.1)

set(SOLUTION_DIR "${CMAKE_SOURCE_DIR}/../../../../../")
add_compile_definitions(PLATFORM_ANDROID)

add_subdirectory("${SOLUTION_DIR}/NativeApp.Shared/" ./Shared/)

project("nativeapp")
add_library(${CMAKE_PROJECT_NAME} SHARED native_lib.cpp android_mono.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE NativeApp.Shared)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    android
    log
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

include("${SOLUTION_DIR}/NativeApp.Shared/cmake/EmbeddingMono.cmake")
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE "${MONO_RUNTIME_LIBS}")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MONO_REQUIRED_LINK_LIBS})


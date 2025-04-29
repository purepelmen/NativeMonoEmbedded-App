cmake_minimum_required(VERSION 3.8)
project(NativeApp.Desktop)

# Enable hot reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(ALLOWED_ARCH_IDS "x64" "x86")

set(NATIVE_ARCH_ID "x64" CACHE STRING "Specify the architecture being used for MSBuild and import lib generation.")
set_property(CACHE NATIVE_ARCH_ID PROPERTY STRINGS ${ALLOWED_ARCH_IDS})

if(NOT NATIVE_ARCH_ID IN_LIST ALLOWED_ARCH_IDS)
    message(FATAL_ERROR "Specified NATIVE_ARCH_ID is '${NATIVE_ARCH_ID}' which is an unsupported value (must be: ${ALLOWED_ARCH_IDS}).")
endif()

if(WIN32)
    set(OSNAME_ID "win")
    add_compile_definitions(PLATFORM_WINDOWS)
else()
    set(OSNAME_ID "linux")
    add_compile_definitions(PLATFORM_LINUX)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(SOLUTION_DIR "${CMAKE_SOURCE_DIR}/../")


add_subdirectory("../NativeApp.Shared/" ./Shared/)

add_executable(NativeApp.Desktop "main.cpp")
target_link_libraries(NativeApp.Desktop PRIVATE NativeApp.Shared)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NativeApp.Desktop PROPERTY CXX_STANDARD 17)
endif()

include("${SOLUTION_DIR}/NativeApp.Shared/cmake/EmbeddingMono.cmake")
if(WIN32)
    include("${CMAKE_SOURCE_DIR}/cmake/GenImportLibs.cmake")
else()
    message(STATUS "Skipping GenImportLibs generation on non-Windows platforms.")
endif()

target_link_directories(NativeApp.Desktop PRIVATE "${MONO_RUNTIME_LIBS}")
if(NOT WIN32)
    # On Linux platforms we must link with .so files, which are in the distribution folder.
    target_link_directories(NativeApp.Desktop PRIVATE "${MONO_RUNTIME_DIST}")
endif()
# EmbeddingMono.cmake reports all libs we should link in MONO_REQUIRED_LINK_LIBS, so use it.
target_link_libraries(NativeApp.Desktop PRIVATE ${MONO_REQUIRED_LINK_LIBS})


# At configure, runtime distribution files should be copied. We don't need to copy it every build.
# MONO_RUNTIME_DIST, provided by EmbeddingMono.cmake script, is the right path to all files we should distribute with the executable.
# For Desktop, it usually contains the runtime (coreclr.dll/.so), and the 'Runtime.Framework' dir with all managed system DLLs.
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MONO_RUNTIME_DIST} ${NativeApp.Desktop_BINARY_DIR}
)

# At every build, copy all that MSBuild project (.csproj) has generated (the whole 'Managed' folder) to binary dir.
# This ensures that we have recently compiled C# project near the executable.
add_custom_target(CopyManagedProject
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MSBUILD_GENERATED} ${NativeApp.Desktop_BINARY_DIR}
    COMMENT "Copying MSBuild outputs."
    VERBATIM
)
add_dependencies(CopyManagedProject BuildManagedProject)
add_dependencies(NativeApp.Desktop CopyManagedProject)

include("${CMAKE_SOURCE_DIR}/cmake/PackageBuild.cmake")

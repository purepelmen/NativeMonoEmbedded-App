project(NativeApp.Shared)

add_library(NativeApp.Shared "src/shared_main.cpp" "src/shared_main.h" "src/abst_env.h" "src/abst_env.cpp" )
target_include_directories(NativeApp.Shared PUBLIC "src/")
target_include_directories(NativeApp.Shared PUBLIC "../thirdparty/Runtime.Mono/include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NativeApp.Shared PROPERTY CXX_STANDARD 17)
endif()

include("cmake/BuildManagedLib.cmake")
add_dependencies(NativeApp.Shared BuildManagedProject)

# Redefine MSBUILD_GENERATED to make it public to the parent project (app container). 
# It should include the built files in the app.
set(MSBUILD_GENERATED "${MSBUILD_GENERATED}" PARENT_SCOPE)

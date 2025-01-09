find_program(IS_DOTNET_INSTALLED dotnet HINTS ENV PATH REQUIRED)
if(NOT IS_DOTNET_INSTALLED)
    message(FATAL_ERROR "'dotnet' CLI tool has not been found (not installed, or inaccessible). It's REQUIRED to build the managed project.")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CSPROJ_BUILD_CONFIGURATION "Debug")
else()
    set(CSPROJ_BUILD_CONFIGURATION "Release")
endif()

set(CSPROJ_FILE "${SOLUTION_DIR}/Assembly-Main/Assembly-Main.csproj")
set(MSBUILD_GENERATED "${SOLUTION_DIR}/artifacts/msbuild_generated/${CSPROJ_BUILD_CONFIGURATION}/")
set(CSPROJ_OUT_DIR "${MSBUILD_GENERATED}/Managed/")

file(MAKE_DIRECTORY ${CSPROJ_OUT_DIR})

# "BuildManagedProject" target helps us to build the .csproj into specific directory for later bundling and with defined configuration.
add_custom_target(BuildManagedProject
    COMMAND ${CMAKE_COMMAND} -E echo "Building .NET project: ${CSPROJ_FILE}"
    COMMAND dotnet build ${CSPROJ_FILE} -c ${CSPROJ_BUILD_CONFIGURATION} /p:OutDir=${CSPROJ_OUT_DIR}
    COMMENT "Building .NET managed project."
    VERBATIM
)
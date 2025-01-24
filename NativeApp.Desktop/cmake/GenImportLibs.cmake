set(GEN_DEF_TOOL "${SOLUTION_DIR}/thirdparty/dev-tools/gendef.exe")
set(CORECLR_LIB "${MONO_RUNTIME_LIBS}/coreclr.lib")

# Ensure the libs folder exists because we're gonna generate files to it.
file(MAKE_DIRECTORY ${MONO_RUNTIME_LIBS})

if(NOT EXISTS ${GEN_DEF_TOOL})
    message(FATAL_ERROR "GenDefTool not found at ${GEN_DEF_TOOL}. Please ensure the tool exists.")
endif()

if(NOT EXISTS ${CORECLR_LIB})
    execute_process(COMMAND ${GEN_DEF_TOOL} "${MONO_RUNTIME_DIST}/coreclr.dll")
    execute_process(COMMAND ${CMAKE_COMMAND} -E rename ./coreclr.def "${MONO_RUNTIME_LIBS}/coreclr.def")
    execute_process(COMMAND lib /def:${MONO_RUNTIME_LIBS}/coreclr.def /out:${CORECLR_LIB} /machine:${NATIVE_ARCH_ID})
endif()

#include <iostream>
#include <filesystem>

#define MONO_DLL_IMPORT
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/image.h>

#include "abst_env.h"

using std::filesystem::path;

const bool enableMonoDebugLogging = false;

int sharedNative_main(path exePath)
{
    std::cout << "Hello World on the native side!\n";
    if (enableMonoDebugLogging)
    {
        Env_SetEnvVar("XUNIT_VERBOSE", "true");
        Env_SetEnvVar("MONO_LOG_LEVEL", "debug");
        Env_SetEnvVar("MONO_LOG_MASK", "all");
    }

    MonoDomain* domain = mono_jit_init("My Native App");
    
    path mainAssemblyPath = exePath / "Managed/Assembly-Main.dll";

    MonoAssembly* assembly = mono_assembly_open(mainAssemblyPath.string().c_str(), NULL);
    if (assembly != nullptr)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        MonoClass* mainClass = mono_class_from_name(image, "AssemblyMain", "Program");
        MonoMethod* mainMethod = mono_class_get_method_from_name(mainClass, "Main", 0);

        std::cout << "Calling managed Program::Main() ...\n";
        char* chars = mono_string_to_utf8(reinterpret_cast<MonoString*>(mono_runtime_invoke(mainMethod, 0, nullptr, nullptr)));

        std::string returnedString{ chars };
        mono_free(chars);
        
        std::cout << "The managed side has returned the string: " << returnedString << "\n";
    }
    else
    {
        std::cout << "Failed to load .NET assembly.\n";
    }

    mono_jit_cleanup(domain);
    return 0;
}

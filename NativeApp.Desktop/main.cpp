#include <iostream>
#include <filesystem>

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <limits.h>
#include <string.h>
#endif

#define MONO_DLL_IMPORT
#include <mono/metadata/assembly.h>

#include <shared_main.h>
#include <abst_env.h>

using std::filesystem::path;

static path GetExecutablePath()
{
#ifdef PLATFORM_WINDOWS
    char buffer[MAX_PATH]{};
    path exePath = std::string(buffer, GetModuleFileName(NULL, buffer, MAX_PATH));
#else
    char buffer[PATH_MAX];
    memset(buffer, 0, sizeof(buffer)); // readlink does not null terminate!

    int chRead = readlink("/proc/self/exe", buffer, PATH_MAX);
    path exePath = std::string(buffer, chRead);
#endif

    return exePath.parent_path();
}

int main()
{
    path exePath = GetExecutablePath();
    
    std::string monoPaths{};
    (monoPaths += exePath.string()) += "/Runtime.Framework/";
    monoPaths += ";";
    (monoPaths += exePath.string()) += "/Managed/";

    mono_set_assemblies_path(monoPaths.c_str());
    return sharedNative_main(exePath);
}

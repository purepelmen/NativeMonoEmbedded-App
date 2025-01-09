#pragma once
#include <android/asset_manager.h>

struct _MonoAssembly;
struct _MonoAssemblyName;

void AndroidMono_Init(AAssetManager* assetManager);

// Utility that helps to look-up framework assemblies on Android platform by checking the APK assets,
// inside the 'Runtime.Framework' directory. This is expected to be called only when loading assemblies by an Assembly Name,
// not its direct path. The assembly will be found by its name specified in the given Assembly Name, and appending '.dll' extension.
_MonoAssembly* PreloadFrameworkAssemblyHook(_MonoAssemblyName* assemblyName, char** assembliesPath, void* userData);

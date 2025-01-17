#include "android_mono.h"

#include <filesystem>

#include <android/log.h>
#include <android/asset_manager.h>

#include <mono/metadata/assembly.h>
#include <mono/metadata/image.h>

using std::filesystem::path;

static AAssetManager* g_assetManager;
const char* FRAMEWORK_PRELOADER_TAG = "MonoFrameworkPreLoader";

static MonoAssembly* LoadAssemblyFromRawBytes(char* data, uint32_t dataLength, const char* assocAssemblyName);
// Utility that helps to look-up framework assemblies on Android platform by checking the APK assets,
// inside the 'Runtime.Framework' directory. This is expected to be called only when loading assemblies by an Assembly Name,
// not its direct path. The assembly will be found by its name specified in the given Assembly Name, and appending '.dll' extension.
static MonoAssembly* PreloadFrameworkAssemblyHook(MonoAssemblyName* assemblyName, char** assembliesPath, void* userData);

void AndroidMono_Init(const std::filesystem::path& exePath, AAssetManager* assetManager)
{
    g_assetManager = assetManager;

    // We need to install this utility because we can't just give path inside the APK assets folder.
    // We need to use AssetManager for that, so this utility will help loading framework assemblies via AssetManager.
    mono_install_assembly_preload_hook(PreloadFrameworkAssemblyHook, nullptr);

    std::string monoPaths{};
    (monoPaths += exePath.string()) += "/Managed/";

    mono_set_assemblies_path(monoPaths.c_str());
}

MonoAssembly* PreloadFrameworkAssemblyHook(MonoAssemblyName* assemblyName, char** assembliesPath, void* userData)
{
    const char* assemblyNameStr = mono_assembly_name_get_name(assemblyName);

    path assetPath { "Runtime.Framework" };
    assetPath /= assemblyNameStr;
    assetPath += ".dll";

    path fakeAssemblyPath { "apk_assets://" };
    fakeAssemblyPath /= assetPath;

    __android_log_print(ANDROID_LOG_DEBUG, FRAMEWORK_PRELOADER_TAG, "Requested loading from the assembly name '%s' [using virtual path: %s]", assemblyNameStr, fakeAssemblyPath.c_str());

    AAsset* asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_UNKNOWN);
    if (asset == nullptr)
    {
        __android_log_print(ANDROID_LOG_DEBUG, FRAMEWORK_PRELOADER_TAG, "Unable to locate '%s' at the assets path: %s", assemblyNameStr, assetPath.c_str());
        return nullptr;
    }

    size_t assetLength = AAsset_getLength(asset);
    __android_log_print(ANDROID_LOG_DEBUG, FRAMEWORK_PRELOADER_TAG, "Reading the assembly from assets file [Asset size: %f KiB]: %s", (double)assetLength / 1024., assetPath.c_str());

    std::unique_ptr<char[]> buffer{ new char[assetLength] };
    AAsset_read(asset, buffer.get(), assetLength);

    AAsset_close(asset);

    MonoAssembly* assembly = LoadAssemblyFromRawBytes(buffer.get(), assetLength, assemblyNameStr);
    if (assembly == nullptr)
    {
        __android_log_print(ANDROID_LOG_ERROR, FRAMEWORK_PRELOADER_TAG, "Unable to load assembly '%s' from raw bytes, though the file exists (loading from virtual path: %s)", assemblyNameStr, fakeAssemblyPath.c_str());
    }
    return assembly;
}

MonoAssembly* LoadAssemblyFromRawBytes(char* data, uint32_t dataLength, const char* assocAssemblyName)
{
    MonoImageOpenStatus status;

    MonoImage* image = mono_image_open_from_data_with_name(data, dataLength, true, &status, false, assocAssemblyName);
    if (image == nullptr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "android_mono", "LoadAssemblyFromRawBytes() failed to open image '%s'", assocAssemblyName);
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from(image, assocAssemblyName, &status);
    if (assembly == nullptr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "android_mono", "LoadAssemblyFromRawBytes() failed to load assembly from image '%s'", assocAssemblyName);

        mono_image_close(image);
        return nullptr;
    }

    return assembly;
}

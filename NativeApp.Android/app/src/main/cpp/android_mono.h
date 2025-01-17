#pragma once
#include <android/asset_manager.h>
#include <filesystem>

void AndroidMono_Init(const std::filesystem::path& exePath, AAssetManager* assetManager);

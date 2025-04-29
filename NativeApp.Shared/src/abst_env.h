#pragma once
#include <string_view>

// =================================================
// Cross-platform abstraction for Environment utils.
// =================================================

#if PLATFORM_WINDOWS
constexpr char ENV_PATHSEP = ';';
#elif PLATFORM_LINUX || PLATFORM_ANDROID
constexpr char ENV_PATHSEP = ':';
#endif

void Env_SetEnvVar(std::string_view name, std::string_view value);

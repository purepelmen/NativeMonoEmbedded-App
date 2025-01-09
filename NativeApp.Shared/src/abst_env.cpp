#include "abst_env.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <cstdlib>
#endif

void Env_SetEnvVar(std::string_view name, std::string_view value)
{
#if PLATFORM_WINDOWS
	SetEnvironmentVariable(name.data(), value.data());
#else
	setenv(name.data(), value.data(), true);
#endif
}

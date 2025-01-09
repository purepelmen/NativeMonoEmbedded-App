#pragma once
#include <string_view>

// =================================================
// Cross-platform abstraction for Environment utils.
// =================================================

void Env_SetEnvVar(std::string_view name, std::string_view value);

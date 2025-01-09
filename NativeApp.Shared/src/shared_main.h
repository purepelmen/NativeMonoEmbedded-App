#include <filesystem>

/// The cross-platform native entry point. The user-code starts here, called by an app container.
/// At the time this function is called, everything must be prepared. For example, the Mono Runtime framework libs path
/// must be already specified, so the initialization can happen immediately. Also some required environment data should be passed like 'exePath'.
/// @param exePath This is the absolute path to an executable directory (not the CWD). It needn't contain an executable but rather Managed libs folder at least (on Desktop, it also contains 'Runtime.Framework').
int sharedNative_main(std::filesystem::path exePath);

# NativeMonoEmbedded-App
This is a fully-working template of a project embedding Mono Runtime (the modern one, Mono CLR) which allows you to write both crossplatform C++ and C# code for Windows, Linux, and Android.

## Structure
Everything is pretty simple. There are two main projects: `NativeApp.Shared` and `Assembly-Main`.

CMake is mostly used here to build everything. `NativeApp.Shared` is a native C++ cross-platform CMake project. It defines its code but it also must initialize Mono runtime, as well as load the `Assembly-Main` managed assembly, and define how to build the `Assembly-Main` MSBuild C# project. The managed project contains cross-platform C# code.

They both define *user code*, and this is where main application code should typically go in this template.
The other projects are *app containers*. They're not really cross-platform and its purpose is to build the two main cross-platform projects and include them into the executable/application, which is what an app container is. It's a wrapper for the cross-platform projects that handles platform differencies of how things must be initialized for different platforms.
Here's app containers this project has now:
* `NativeApp.Desktop` (Executable): Intended for all Desktop platforms (currently only Windows and Linux supported); Uses CMake;
* `NativeApp.Android` (Android application): Android only for all architectures; More code is required to make things work on this platform; Uses Gradle and CMake;

## Building
### Requirements
Generally for a Desktop build you need this:
- .NET SDK
- C++ compiler (MSVC, GCC, Clang)
- CMake
- Python 3 (optional)

For Android you will need this additionally:
- Android SDK & NDK
- Java

Note: about exact versions... it depends. In this project I use:
> - `Assembly-Main` is targetting **net8.0**.
> - `NativeApp.Shared`, `NativeApp.Desktop` and `nativeapp` (in `NativeApp.Android`) set the C\+\+20 in CMake. They also use C++17 features like **std::filesystem**.
> - `NativeApp.Android` targets SDK 34, sets min SDK as 24, uses Java 8 and doesn't specify a specific NDK version.

> Of course you can change these versions and apply some tweaks so everything can work on older/newer versions.

Note: Python is needed only if you would use `prepare-runtime.py`. It's not used when building the project.

### Getting runtime libraries
Before you build you must install Mono runtime binaries (it's not included in the repo, would be too big). They're typically placed at paths like `thirdparty/Runtime.Mono/gen-windows/`. You can of course use your own built runtime, or... better use my simple script `prepare-runtime.py` to easily install and unpack the runtime in the expected format (what the build system recognises). It downloads them from NuGet. With it you can also learn how exactly and where to put your own runtime binaries.

Call the script without arguments to show the usage and some additional info. Then download the binaries for the desired platforms, here's some examples:
- `py prepare-runtime.py win-x64` installs the runtime for Windows x64 platform with the default version (the default is NET 8.0.11).
- `py prepare-runtime.py linux-x64 9.0.0-preview.7.24405.7` - for Linux x64, with version specified.
- `py prepare-runtime.py android-arm64` - for Android arm64-v8a architecture.

Note that if you want to know exact platform identifiers (like `win-x64`) and available versions, you should search for NuGet packages like `Microsoft.NETCore.App.Runtime.Mono.*`, and check what versions are available for them.

### Building executable/application
Now you can simply build the `NativeApp.Desktop` project. You can also do the same pretty simple with `NativeApp.Android`. Just make sure that required SDKs are installed.

## TODO
- [x] Add Linux support, and convert the project to a CMake project (don't use MSBuild's .vcxproj).
> Done, as well as the concept of app containers was introduced. Now all desktop platforms are built through a CMake project `NativeApp.Desktop`. Also `NativeApp.Shared` appeared and it's supposed to have only fully cross-platform native code. 
- [x] Add Android support (new app-container: NativeApp.Android).
- [x] Stop reextracting tons of files on every application start-up for `NativeApp.Android`.
> Done. Now ~~the framework libs and~~ 'Managed' folders (user-code) are extracted and copied only once to the internal cache folder. Start-up time has increased significantly.
- [x] Eliminate copying a lot of files (Mono Runtime files) for `NativeApp.Android` as this increases the space used by the app.
> Done. Mono API requires to specify a specific path to the framework libs directory which was hard to do if I wanna ship the whole runtime, as I haven't found a way to get the direct path to Android APK's assets folder. Now I'm providing custom PreLoad hook which can locate and load necessary assemblies. I managed to decrease the space the app uses significantly (app size + files + cache): roughly **40 MB -> 15 MB**, in my specific case (`arm64-v8a` only, Debug build type, some framework components removed).

> The user-code ('Managed' dir) is STILL copied at the first start-up though, and won't be located the same way (I want to have full real paths to the directory).
- [x] Fix `NativeApp.Android` project building so it can build Managed libs, copy files to assets at the right time.
- [x] Fix `NativeApp.Android` can't determine what Managed libs it need to copy (Debug or Release).
- [x] Add support for other architectures for Desktop platforms (i.e. windows 32-bit).
- [ ] Provide a script to build all native projects and prepare build output files for using so the project can be built by only having .NET SDK (+ optionally Android SDK).
- [x] Fix problems with building of the managed project: it does not copy NuGet package contents (managed and native libs).
> Using `dotnet publish` instead of `build` builds and prepares everything required for the app to work. Apparently it should have been used from the start.
- [x] Fix `NativeApp.Shared` so the dependencies of the main assembly (`Assembly-Main`) are loaded by the CLR.
> The 'Managed' directory is now added to assembly paths on all platforms so it's even now possible to just specify an AssemblyName instead of path.
- [x] Fix `NativeApp.Android` fails to extract Managed binary assets with sub-folders.
- [ ] Fix `NativeApp.Android` dont-reextract optimization: updating Android application does not trigger reextraction of newly compiled Managed libraries.
- [x] Make proper native shared library loading (through P/Invoke) for Desktop platforms.
> From my understanding, Mono CLR by default tries first to locate native libs near the assembly containing the P/Invoke method. So P/Invoke-s should work fine. However this is not the case with `NativeLibrary.Load()` calls. For example Silk.NET doesn't work when native libs are placed in the 'Managed' folder automatically. They must be placed near the executable.

> So I specified the RID when executing `dotnet publish`, and that's the only thing I did. I decided not to add the 'Managed' library in the shared libs search paths because I need consistent support for it on all desktop OS. On Windows I can do this easily at runtime by `AddDllDirectory()`, while on Linux only `RPATH` seems to work as I want which is deprecated and discouraged from using, so I won't rely on it.
- [ ] Debugger?
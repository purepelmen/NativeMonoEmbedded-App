# NativeMonoEmbedded-App
This is a fully-working template of a project embedding Mono Runtime (the modern one, Mono CLR) which allows you to write both crossplatform C++ and C# code for Windows, Linux, and Android.

## Use cases
You can use this template if you want to avoid using **.NET MAUI** or **.NET for Android**.
As this also allows you to directly edit how you embed, start and control the runtime, you can use it whenever you need full control (and the access to Mono API), or when you need very minimal .NET embedded apps.

It also a good project where you can look, inspect, and understand how you can embed and start the modern .NET Mono runtime on different platforms. Docs and comments can additionally help you.

## Structure
Everything is pretty simple. There are two main projects: `NativeApp.Shared` and `Assembly-Main`.

CMake is mostly used here to build everything. `NativeApp.Shared` is a native C++ cross-platform CMake project. It defines its code but it also must initialize Mono runtime, as well as load the `Assembly-Main` managed assembly, and define how to build the `Assembly-Main` MSBuild C# project. The managed project contains cross-platform C# code.

They both define *user code*, and this is where main application code should typically go in this template.
The other projects are *app containers*. They're not really cross-platform and its purpose is to build the two main cross-platform projects and include them into the executable/application, which is what an app container is. It's a wrapper for the cross-platform projects that handles platform differencies of how things must be initialized for different platforms.
Here's app containers this project has now:
* `NativeApp.Desktop` (Executable): Intended for all Desktop platforms (currently only Windows and Linux supported); Uses CMake;
* `NativeApp.Android` (Android application): Android only for all architectures; More code is required to make things work on this platform; Uses Gradle and CMake;

### Built executable (Desktop)
At the program folder there's some required folders and the executable. All this listed below.
- NativeApp.Desktop(.exe) - the main executable. By default there's the `NativeApp.Desktop` project code, and statically linked `NativeApp.Shared` code.
- coreclr.dll | libcoreclr.so - the shared library of the runtime. Tightly linked to the executable and must reside at the root of the program.
- `Runtime.Framework/` dir - contains all frameworks libs. There's the corelib (System.Private.CoreLib), other managed framework libs, but some native libs required by the runtime might also be there. This folder is added to mono assemblies dirs.
- `Managed/` dir - contains compiled user code. This folder is also added to mono assemblies dirs. There can be also some native libs for P/Invoke methods defined in user code assemblies. Note though this folder is not added to shared library search paths, so `NativeLibrary.Load()`, `dlopen()` or `LoadLibrary()` calls wouldn't load native libs from there. The MSBuild project must copy such native libs to the parent folder of the `$(PublishDir)` after Publish which will be placed automatically near the executable by CMake.

### Built application (Android)
The APK contains the whole runtime. 
- `assets/Runtime.Framework/` - contains all frameworks libs except native ones. 
- `assets/Managed/` - contains compiled user code. This folder is automatically unpacked at start-up so it's possible to have full path to the contents of this folder.
- `libs/*/` - contains native libs. Specifically...
	- libnativeapp.so - the compiled native project inside the `NativeApp.Android` project. It has required code to prepare Mono. `NativeApp.Shared` is statically linked with this library.
	- libmonosgen-2.0.so - the Mono runtime itself. Tightly linked to `libnativeapp.so`.
	- libmono-component-*.so - some additional optional mono components may be present here.
	- libSystem.Native.so (and other like: libSystem.Globalization.Native.so, ...) - some native libs required by the framework may also be here. They placed here, and not in `Runtime.Framework/` folder as in Desktop builds.
- The compiled Java code contains some required functionality to bootstrap the application (like extracting files from the APK).

When the application is started, the `assets/Managed` folder is extracted (only once) to the app cache directory: `/data/data/<package_name>/cache/Managed`.
This folder is added to mono assemblies paths. All the data that the MSBuild project exports will be here (that is, not only managed libs).

## Requirements
Generally to build for Desktop, you need this:
- .NET SDK
- C++ compiler (MSVC/GCC/Clang)
- CMake
- Python 3 (optional)

For Android you will need this additionally:
- Android SDK & NDK
- Java

Note: about exact versions... it depends. In this project I use:
> - `Assembly-Main` is targetting **net8.0**.
> - `NativeApp.Shared`, `NativeApp.Desktop` and `nativeapp` (in `NativeApp.Android`) require C\+\+17 (defined in CMake).
> - `NativeApp.Android` targets SDK 34, sets min SDK as 24, uses Java 8 and doesn't set a specific NDK version.
> - CMake minimum version: **3.8**.

> Of course you can change these versions and apply some tweaks so everything can work on older/newer versions.

Note: Python is needed only if you would use `prepare-runtime.py`. It's not used when building the project.

### Getting runtime libraries
Before you build you must install Mono runtime binaries (it's not included in the repo, would be too big). They're typically placed at paths like `thirdparty/Runtime.Mono/gen-windows/`. You can of course use your own built runtime, or... better use my simple script `prepare-runtime.py` to easily install and unpack the runtime in the expected format (what the build system recognises). It downloads them from NuGet. With it you can also learn how exactly and where to put your own runtime binaries.

Call the script without arguments to show the usage and some additional info. Then download the binaries for the desired platforms, here's some examples:
- `py prepare-runtime.py win-x64` installs the runtime for Windows x64 platform with the default version (the default is NET 8.0.11).
- `py prepare-runtime.py linux-x64 9.0.0-preview.7.24405.7` - for Linux x64, with version specified.
- `py prepare-runtime.py android-arm64` - for Android arm64-v8a architecture.

Note that if you want to know exact platform identifiers (like `win-x64`) and available versions, you should search for NuGet packages like `Microsoft.NETCore.App.Runtime.Mono.*`, and check what versions are available for them.

## Building
Now you can simply build the `NativeApp.Desktop` project. You can also do the same pretty simple with `NativeApp.Android`. Just make sure that required SDKs are installed.

### Building for Windows/Linux
#### Architecture
By default the CMake script assumes you build for `x64` architecture. It doesn't care what compilers you use (no automatic determination). So if you would like to use an `x86` compiler (or specify architecture with compiler flags), you must give a hint about it to CMake via the `NATIVE_ARCH_ID` cache variable. [Available values: x86; x64] 

For Windows it's easier because `CMakePresets.json` provides some configuration presets specifically for Windows: `x64-debug`, `x64-release`, `x86-debug`, `x86-release`.

#### Packaging
Set `PACKAGE_BUILD` bool cache variable to true to prepare a fully ready and clean build into a separate folder after build. It should be at path like `artifacts/Debug-x64/`.
It's not necessary because the normal build places everything required to run the application in the CMake build directory.

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
> Motivation: to provide an ability to pre-build the native part of the app to act like a bootstrapper for the managed app, and then only work with C# code.
- [x] Fix problems with building of the managed project: it does not copy NuGet package contents (managed and native libs).
> Using `dotnet publish` instead of `build` builds and prepares everything required for the app to work. Apparently it should have been used from the start.
- [x] Fix `NativeApp.Shared` so the dependencies of the main assembly (`Assembly-Main`) are loaded by the CLR.
> The 'Managed' directory is now added to assembly paths on all platforms so it's even now possible to just specify an AssemblyName instead of path.
- [x] Fix `NativeApp.Android` fails to extract Managed binary assets with sub-folders.
- [x] Fix `NativeApp.Android` dont-reextract optimization: updating Android application does not trigger reextraction of newly compiled Managed libraries.
- [x] Make proper native shared library loading (through P/Invoke) for Desktop platforms.
> From my understanding, Mono CLR by default tries first to locate native libs near the assembly containing the P/Invoke method. So P/Invoke-s should work fine. However this is not the case with `NativeLibrary.Load()` calls. For example Silk.NET doesn't work when native libs are placed in the 'Managed' folder automatically. They must be placed near the executable.

> So I specified the RID when executing `dotnet publish`, and that's the only thing I did. I decided not to add the 'Managed' library in the shared libs search paths because I need consistent support for it on all desktop OS. On Windows I can do this easily at runtime by `AddDllDirectory()`, while on Linux only `RPATH` seems to work as I want which is deprecated and discouraged from using, so I won't rely on it.
- [x] Put multiple System.Private.CoreLib for different architectures in `NativeApp.Android`.
> This library probably has some native code or something, and at least this framework lib should be dublicated. The others are probably safe to overwrite with only one architecture variant.

> Done. Now Runtime.Framework folder contains all managed framework libs that are shared on all architectures, and architecture specific libs will go in `Runtime.Framework/{arch}/` folder.
- [ ] Debugger?
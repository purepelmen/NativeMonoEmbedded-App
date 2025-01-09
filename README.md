# NativeMonoEmbedded-App
This is a fully-working template of a project embedding Mono Runtime (the modern one, Mono CLR) which allows you to write both crossplatform C++ and C# code for Windows, Linux, and Android.

## Structure
Everything is pretty simple: this projects uses CMake as a build system for the whole project (which may also use MSBuild to build .NET projects).
There are two main projects: `NativeApp.Shared` and `Assembly-Main`. The former one is a native C++ cross-platform project which also contains code to initialize the Mono runtime. The latter one is cross-platform C# project.

The other projects are *app containers*. They're not really cross-platform and its purpose is to build the two main cross-platform projects and include them into the executable/application, which is what an app container is. It's a wrapper for the cross-platform projects that handles platform differencies of how things must be initialized for different platforms.
Here's app containers the project has now:
* `NativeApp.Desktop` (executable for all Desktop platforms, but currently only Windows and Linux).
* `NativeApp.Android` (Android application) - Some more code is required to make things work on this platform.

## TODO
- [x] Add Linux support, and convert the project to a CMake project (don't use MSBuild's .vcxproj).
> Done, as well as the concept of app containers was introduced. Now all desktop platforms are built through a CMake project `NativeApp.Desktop`. Also `NativeApp.Shared` appeared and it's supposed to have only fully cross-platform native code. 
- [x] Add Android support (new app-container: NativeApp.Android).
- [x] Stop reextracting tons of files on every application start-up for `NativeApp.Android`.
> Done. Now ~~the framework libs and~~ 'Managed' folders (user-code) are extracted and copied only once to the internal cache folder. Start-up time has increased significantly.
- [x] Eliminate copying a lot of files (Mono Runtime files) for `NativeApp.Android` as this increases the space used by the app.
> Done. Mono API requires to specify a specific path to the framework libs directory which was hard to do if I wanna ship the whole runtime, as I haven't found a way to get the direct path to Android APK's assets folder. Now I'm providing custom PreLoad hook which can locate and load necessary assemblies. I managed to decrease the space the app uses significantly (app size + files + cache): roughly **40 MB -> 15 MB**, in my specific case (`arm64-v8a` only, Debug build type, some framework components removed).

> The user-code ('Managed' dir) is STILL copied at the first start-up though, and won't be located the same way (I want to have full real paths to the directory).
- [ ] Fix `NativeApp.Android` project building so it can build Managed libs, copy files to assets at the right time.
- [ ] Fix `NativeApp.Android` can't determine what Managed libs it need to copy (Debug or Release).
- [ ] Add support for other architectures for Desktop platforms (i.e. windows 32-bit).

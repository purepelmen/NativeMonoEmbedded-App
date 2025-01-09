import os
import shutil
import sys
import urllib.request

from zipfile import ZipFile
from distutils.dir_util import copy_tree, mkpath
from distutils.file_util import copy_file

def try_get_arg(index) -> str|None:
    if index + 1 >= len(sys.argv):
        return None
    
    return sys.argv[index + 1]

def print_usage():
    print("[!] You must input one or two arguments. Usage:")
    print()

    print("Prepare for specified architecture and use the default version:")
    print("\t$script$ linux-x64")
    print("The same as the above but with runtime version specification:")
    print("\t$script$ linux-x64 8.0.11")
    print()

    print("You can find specific variants of architecture IDs and valid .NET versions at nuget.org. All package names should start with: 'Microsoft.NETCore.App.Runtime.Mono.'")
    print("Example arch IDs: win-x64, linux-x64, android-arm64")
    print("Example versions: 8.0.0, 8.0.11, 9.0.0-preview.7.24405.7")


class OSArch:
    WIN = "windows"
    LINUX = "linux"
    ANDROID = "android"

    X86 = "x86"
    X64 = "x64"
    ARM32 = "armeabi-v7a"
    ARM64 = "arm64-v8a"

    KNOWN_OS = [ WIN, LINUX, ANDROID ]
    KNOWN_ARCH = [ X86, X64, ARM32, ARM64 ]

    def __init__(self, os: str, arch: str):
        if os not in OSArch.KNOWN_OS:
            raise ValueError(f"Incorrectly initialized OSArch object with unknown OS: '{os}'")
        if arch not in OSArch.KNOWN_ARCH:
            raise ValueError(f"Incorrectly initialized OSArch object with unknown arch: '{arch}'")
        
        self.__os = os
        self.__arch = arch

    def is_os(self, compare_os: str):
        if compare_os not in OSArch.KNOWN_OS:
            raise ValueError(f"is_os() requires you to specify a known os, but you specified: '{compare_os}'")
        
        return self.__os == compare_os
    
    def get_os(self):
        return self.__os
    
    def get_arch(self):
        return self.__arch

def osarch_from_archid(given_archid: str) -> OSArch|None:
    if given_archid.startswith("win"):
        os_id = OSArch.WIN
    elif given_archid.startswith("linux"):
        os_id = OSArch.LINUX
    elif given_archid.startswith("android"):
        os_id = OSArch.ANDROID
    else:
        return None
    
    if given_archid.endswith("arm64"):
        arch_name = OSArch.ARM64
    elif given_archid.endswith("arm"):
        arch_name = OSArch.ARM32
    elif given_archid.endswith("x64"):
        arch_name = OSArch.X64
    elif given_archid.endswith("x86"):
        arch_name = OSArch.X86
    else:
        return None

    return OSArch(os_id, arch_name)


def prepare_runtime() -> None:
    if os.path.exists(local_archive):
        print("Downloading skipped: the local cached file exists.")
        return

    print("Retrieving the package from nuget.org ...")
    urllib.request.urlretrieve(f"https://www.nuget.org/api/v2/package/{nuget_package}/{package_version}", local_archive)

def extract_runtime():
    with ZipFile(local_archive, 'r') as archive:
        i = 0
        for file in archive.namelist():
            if not file.startswith(ar_runtime_path):
                continue

            i += 1
            archive.extract(file, temp_runtime_dir)

    if i == 0:
        raise RuntimeError(f"The NuGet package doesn't contain the runtime folder, or it's empty: {ar_runtime_path}")

def clean_up():
    shutil.rmtree(temp_runtime_dir)

def find_libs_dir() -> str:
    lib_dir = f"{temp_runtime_dir}/{ar_runtime_path}/lib/"

    lib_content = os.listdir(lib_dir)
    if len(lib_content) != 1 or not lib_content[0].startswith("net"):
        raise RuntimeError("The NuGet package doesn't has a valid 'net*.*' folder inside the lib folder.")

    return f"{lib_dir}/{lib_content[0]}/"

def copy_selected_files_from(source_dir: str, dest_dir: str, predicate):
    mkpath(dest_dir)

    contents = os.listdir(source_dir)
    for file in contents:
        full_path = f"{source_dir}/{file}"

        if os.path.isdir(full_path):
            continue
        if predicate(file) is False:
            continue

        copy_file(full_path, dest_dir)

def copy_core_clr(native_dir: str, runtime_dist_dir: str):
    native_content = os.listdir(native_dir)

    # Here we should find the right coreclr dynamic lib.
    # This is only tested and will work on Windows and Linux.
    found_amount = 0
    found_coreclr = ""
    for file in native_content:
        if "coreclr" not in file:
            continue

        found_coreclr = file
        found_amount += 1

    if found_amount == 0:
        raise RuntimeError("The NuGet package doesn't has a valid 'coreclr' native lib.")
    if found_amount != 1:
        raise RuntimeError("The NuGet package seems to have multiple 'coreclr' native libs. This is not expected.")

    # Copy the coreclr shared library
    copy_file(native_dir + found_coreclr, runtime_dist_dir)

def generate_binaries_generic(lib_dir: str, native_dir: str):
    def check_required_natives(file: str) -> bool:
        return "coreclr" not in file and "hostfxr" not in file and "hostpolicy" not in file and "mono-component" not in file
    
    runtime_dist_dir = f"{runtime_base_dir}/dist-{os_arch.get_arch()}/"
    runtime_dist_framework = f"{runtime_dist_dir}/Runtime.Framework/"

    # All the contents from the 'lib' folder should be copied to the framework dir.
    copy_tree(lib_dir, runtime_dist_framework)

    copy_selected_files_from(native_dir, runtime_dist_framework, check_required_natives)
    copy_core_clr(native_dir, runtime_dist_dir)

def generate_binaries_android(lib_dir: str, native_dir: str):
    def check_required_natives(file: str) -> bool:
        return file.endswith(".so")

    runtime_dist_framework = f"{runtime_base_dir}/dist/Runtime.Framework/"
    runtime_jniLibs = f"{runtime_base_dir}/jniLibs/{os_arch.get_arch()}/"
    
    # All the contents from the 'lib' folder should be copied to the framework dir.
    copy_tree(lib_dir, runtime_dist_framework)
    # We need probably all .so shared libs.
    copy_selected_files_from(native_dir, runtime_jniLibs, check_required_natives)
    
    # And the last, the corelib.
    corelib_path = f"{native_dir}/System.Private.CoreLib.dll"
    copy_file(corelib_path, runtime_dist_framework)

def generate_binaries():
    lib_dir = find_libs_dir()
    native_dir = f"{temp_runtime_dir}/{ar_runtime_path}/native/"

    if os_arch.is_os(OSArch.ANDROID):
        generate_binaries_android(lib_dir, native_dir)
    else:
        generate_binaries_generic(lib_dir, native_dir)


package_version = "8.0.11"
package_archid = "win-x64"


if __name__ != "__main__":
    raise RuntimeError("This is only supposed to be runned as script!")

if len(sys.argv) < 2:
    print_usage()
    exit(0)

package_archid = try_get_arg(0)

input_version = try_get_arg(1)
if input_version is not None:
    package_version = input_version

os_arch = osarch_from_archid(package_archid)
if os_arch is None:
    print(f"ERROR: Can't determine what os arch is this from this package arch id: {package_archid}.")
    exit(-1)

nuget_package = f"Microsoft.NETCore.App.Runtime.Mono.{package_archid}"

local_archive = f"{nuget_package}_{package_version}.zip"
ar_runtime_path = f"runtimes/{package_archid}/"
temp_runtime_dir = f"TempRuntime_{package_archid}/"

# Runtime output dirs.
runtime_base_dir = f"thirdparty/Runtime.Mono/gen-{os_arch.get_os()}/"

print(f"-> Preparing the runtime: {nuget_package}")
prepare_runtime()

print(f"-> Processing the local package file: {local_archive}")
extract_runtime()
generate_binaries()

clean_up()

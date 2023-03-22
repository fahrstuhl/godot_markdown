#!/usr/bin/env python
import os
import sys
import subprocess
import shutil


# Local dependency paths, adapt them to your setup
cpp_bindings_path = "godot-cpp/"
godot_extension_path = cpp_bindings_path + "gdextension/"
cpp_library = "libgodot-cpp"

# Try to detect the host platform automatically.
# This is used if no `platform` argument is passed
if sys.platform.startswith("linux"):
    host_platform = "linux"
elif sys.platform.startswith("freebsd"):
    host_platform = "freebsd"
elif sys.platform == "darwin":
    host_platform = "osx"
elif sys.platform == "win32" or sys.platform == "msys":
    host_platform = "windows"
else:
    raise ValueError("Could not detect platform automatically, please specify with " "platform=<platform>")

env = Environment(ENV=os.environ)

opts = Variables([], ARGUMENTS)

# Define our options
opts.Add(EnumVariable("target", "Compilation target", "template_debug", allowed_values=("template_debug", "template_release"), ignorecase=2))
opts.Add(
    EnumVariable(
        "platform",
        "Compilation platform",
        host_platform,
        allowed_values=("linux", "windows", "osx", "android"),
        ignorecase=2,
    )
)
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'linuxbsd', 'linux', 'osx', 'android']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(EnumVariable('arch', "Compilation architecture", 'x86_64', ['x86_64', 'armv7a', 'armv8a']))
opts.Add(EnumVariable("macos_arch", "Target macOS architecture", "universal", ["universal", "x86_64", "arm64"]))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'demo/bin/'))
opts.Add(PathVariable('target_name', 'The library name.', 'libmarkdown', PathVariable.PathAccept))
opts.Add(PathVariable('android_ndk', 'Path to the Android NDK installation', '/opt/android-ndk/', PathVariable.PathAccept))
opts.Add(BoolVariable('build_libcmark_gfm', "Build and deploy libcmark-gfm for current platform.", 'no'))


# only support 64 at this time..
bits = 64

# Updates the environment with the option variables.
opts.Update(env)
# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))

# This makes sure to keep the session environment variables on Windows.
# This way, you can run SCons in a Visual Studio 2017 prompt and it will find
# all the required tools
if host_platform == "windows" and env["platform"] != "android":
    if env["bits"] == "64":
        env = Environment(TARGET_ARCH="amd64")
    elif env["bits"] == "32":
        env = Environment(TARGET_ARCH="x86")

    opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['platform'] == '':
    print("No valid target platform selected.")
    quit()

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

if env["target"] == "debug":
    env.Append(CPPDEFINES=["DEBUG_ENABLED", "DEBUG_METHODS_ENABLED"])

# Check our platform specifics
if env["platform"] == "osx":
    env["target_path"] += "osx/"
    cpp_library += ".osx"

    if env["bits"] == "32":
        raise ValueError("Only 64-bit builds are supported for the macOS target.")

    if env["macos_arch"] == "universal":
        env.Append(LINKFLAGS=["-arch", "x86_64", "-arch", "arm64"])
        env.Append(CCFLAGS=["-arch", "x86_64", "-arch", "arm64"])
    else:
        env.Append(LINKFLAGS=["-arch", env["macos_arch"]])
        env.Append(CCFLAGS=["-arch", env["macos_arch"]])

    env.Append(CFLAGS=["-std=c11"])
    env.Append(CXXFLAGS=["-std=c++17"])
    if env["target"] == "debug":
        env.Append(CCFLAGS=["-g", "-O2"])
    else:
        env.Append(CCFLAGS=["-g", "-O3"])

    arch_suffix = env["macos_arch"]

elif env["platform"] in ("x11", "linux"):
    cpp_library += ".linux"
    env.Append(CCFLAGS=["-fPIC"])
    env.Append(CXXFLAGS=["-std=c++17"])
    env.Append(CFLAGS=["-std=c11"])
    if env["target"] == "debug":
        env.Append(CCFLAGS=["-g3", "-Og"])
    else:
        env.Append(CCFLAGS=["-g", "-O3"])

    arch_suffix = env["arch"]
elif env["platform"] == "windows":
    cpp_library += ".windows"
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV = os.environ)
    env['CC'] = 'x86_64-w64-mingw32-gcc'
    env['CXX'] = 'x86_64-w64-mingw32-gcc'
    env.Append(CPPDEFINES=["WIN32", "_WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS"])
    env.Append(LIBS=['advapi32'])
    env.Append(CCFLAGS=["-W3", "-GR"])
    env.Append(CXXFLAGS=["-std:c++17"])
    env.Append(CFLAGS=["-std=c11"])
    if env["target"] == "debug":
        env.Append(CPPDEFINES=["_DEBUG"])
        env.Append(CCFLAGS=["-EHsc", "-MDd", "-ZI", "-FS"])
        env.Append(LINKFLAGS=["-DEBUG"])
    else:
        env.Append(CPPDEFINES=["NDEBUG"])
        env.Append(CCFLAGS=["-O2", "-EHsc", "-MD"])

    if not(env["use_llvm"]):
        env.Append(CPPDEFINES=["TYPED_METHOD_BIND"])

    arch_suffix = str(bits)

elif env['platform'] == "android":
    if env['arch'] == "armv7a":
        env['CC'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi30-clang"
        env['CXX'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi30-clang++"
    elif env['arch'] == "armv8a":
        env['CC'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang"
        env['CXX'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang++"
    elif env['arch'] == "x86_64":
        env['CC'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android30-clang"
        env['CXX'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android30-clang++"
    env.Append(CXXFLAGS=["-std:c++17"])
    env.Append(CFLAGS=["-std=c11"])
    if env['target'] == 'debug':
        env.Append(CCFLAGS = ['-fPIC', '-g3','-Og'])
    else:
        env.Append(CCFLAGS = ['-fPIC', '-g','-O3'])

# suffix our godot-cpp library
cpp_library += "." + env["target"] + "." + arch_suffix

# Local dependency paths, adapt them to your setup
suffix = "{}/{}/{}".format(env['platform'], env['arch'], env['target'])
target = os.path.join(env['target_path'], suffix, env['target_name'])
libcmark_gfm_lib_path = "demo/bin/{}".format(suffix)
libcmark_gfm_include_paths = [
        "godot_markdown/thirdparty/cmark-gfm/src/", # for cmark-gfm.h
        "godot_markdown/thirdparty/cmark-gfm/extensions/", # for all extension headers
        "godot_markdown/thirdparty/cmark-gfm/build_{}_{}/src/".format(env['platform'], env['arch']), # for export.h and version.h
        "godot_markdown/thirdparty/cmark-gfm/build_{}_{}/extensions/".format(env['platform'], env['arch']), # for extensions_export.h
        ]

env.Append(CPPPATH=[".", godot_extension_path, cpp_bindings_path + "include/", cpp_bindings_path + "gen/include/", 'godot_markdown/src/', *libcmark_gfm_include_paths])
env.Append(LIBPATH=[libcmark_gfm_lib_path, cpp_bindings_path + "bin/"])
env.Append(LIBS=['cmark-gfm-extensions', 'cmark-gfm', 'cmark-gfm-extensions', cpp_library])

# sources = Glob('godot_markdown/src/*.c')
sources = Glob('godot_markdown/src/*.cpp')

library = env.SharedLibrary(target=target, source=sources)

if env['build_libcmark_gfm']:
    cwd = os.getcwd()
    os.chdir("godot_markdown/thirdparty/cmark-gfm")

    lib_target_path = os.path.join(cwd, libcmark_gfm_lib_path)
    build_dir = "build_{}_{}".format(env['platform'], env['arch'])

    os.makedirs(lib_target_path, exist_ok=True)
    os.makedirs(build_dir, exist_ok=True)

    os.chdir(build_dir)
    if env['platform'] == 'android':
        if env['arch'] == "armv7a":
            android_abi = "-DANDROID_ABI=armeabi-v7a"
        elif env['arch'] == "armv8a":
            android_abi = "-DANDROID_ABI=arm64-v8a"
        elif env['arch'] == "x86_64":
            android_abi = "-DANDROID_ABI=x86_64"
        cmake = [
                "cmake",
                "-DCMAKE_TOOLCHAIN_FILE=/opt/android-ndk/build/cmake/android.toolchain.cmake",
                android_abi,
                "-DANDROID_NATIVE_API_LEVEL=",
                "..",
                ]
    elif env['platform'] == 'windows':
        cmake = [
                "cmake",
                "-DCMAKE_TOOLCHAIN_FILE={}/toolchain-mingw32.cmake".format(cwd),
                ".."
                ]

    else:
        cmake = ["cmake", ".."]
    make = ["make"]
    subprocess.run(cmake)
    subprocess.run(make)
    if env['platform'] == 'windows':
        ext = "dll"
    else:
        ext = "a"
    shutil.copy2("extensions/libcmark-gfm-extensions.{}".format(ext), lib_target_path)
    shutil.copy2("src/libcmark-gfm.{}".format(ext), lib_target_path)
    os.chdir(cwd)

env.Tool('compilation_db')
cdb = env.CompilationDatabase('compile_commands.json')
Alias('cdb', cdb)

Default(library)
